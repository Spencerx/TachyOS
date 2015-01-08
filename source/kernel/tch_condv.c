/*
 * tch_condv.c
 *
 *  Created on: 2014. 8. 15.
 *      Author: innocentevil
 */

#include "tch.h"
#include "tch_kernel.h"
#include "tch_condv.h"
#include "tch_mem.h"


#define CONDV_VALID        ((uint8_t) 1)
#define CONDV_WAIT         ((uint8_t) 2)

#define TCH_CONDV_CLASS_KEY           ((uint16_t) 0x2D01)

#define tch_condvSetWait(condv)       ((tch_condvCb*) condv)->state |= CONDV_WAIT
#define tch_condvClrWait(condv)       ((tch_condvCb*) condv)->state &= ~CONDV_WAIT
#define tch_condvIsWait(condv)        ((tch_condvCb*) condv)->state & CONDV_WAIT




static inline void tch_condvValidate(tch_condvId condv);
static inline void tch_condvInvalidate(tch_condvId condv);
static inline BOOL tch_condvIsValid(tch_condvId condv);

static tch_condvId tch_condv_create();
static tchStatus tch_condv_wait(tch_condvId condv,tch_mtxId mtx,uint32_t timeout);
static tchStatus tch_condv_wake(tch_condvId condv);
static tchStatus tch_condv_wakeAll(tch_condvId condv);
static tchStatus tch_condv_destroy(tch_condvId condv);



__attribute__((section(".data"))) static tch_condv_ix CondVar_StaticInstance = {
		tch_condv_create,
		tch_condv_wait,
		tch_condv_wake,
		tch_condv_wakeAll,
		tch_condv_destroy
};



const tch_condv_ix* Condv = &CondVar_StaticInstance;



void tch_condvInit(tch_condvCb* condv){
	uStdLib->string->memset(condv,0,sizeof(tch_condvCb));
	tch_listInit((tch_lnode_t*)&condv->wq);
	condv->wakeMtx = NULL;
	tch_condvValidate(condv);
	condv->__obj.destructor = (tch_uobjDestr) tch_noop_destr;
}


static tch_condvId tch_condv_create(){
	tch_condvCb* condv = (tch_condvCb*)shMem->alloc(sizeof(tch_condvCb));
	uStdLib->string->memset(condv,0,sizeof(tch_condvCb));
	tch_listInit((tch_lnode_t*)&condv->wq);
	condv->wakeMtx = NULL;
	tch_condvValidate(condv);
	condv->__obj.destructor = (tch_uobjDestr) tch_condv_destroy;
	return condv;
}

/*! \brief thread wait until given condition is met
 *  \
 *
 */
static tchStatus tch_condv_wait(tch_condvId id,tch_mtxId lock,uint32_t timeout){
	tch_condvCb* condv = (tch_condvCb*) id;
	if(!tch_condvIsValid(condv))
		return tchErrorResource;
	condv->wakeMtx = lock;
	if(tch_port_isISR()){
		tch_kernel_errorHandler(FALSE,tchErrorISR);
		return tchErrorISR;
	}
	tch_condvSetWait(condv);
	Mtx->unlock(lock);
	tchStatus result = tchOK;
	if((result = tch_port_enterSv(SV_THREAD_SUSPEND,(uint32_t)&condv->wq,timeout)) != tchOK){
		if(!tch_condvIsValid(condv))
			return tchErrorResource;
		switch(result){
		case tchEventTimeout:
			return tchErrorTimeoutResource;
		case tchErrorResource:
			return tchErrorResource;
		}
	}
	if(!tch_condvIsValid(condv))
		return tchErrorResource;
	if((result = Mtx->lock(lock,osWaitForever)) != tchOK)
		return tchErrorResource;
	return tchOK;
}





/*!
 * \brief posix condition variable signal
 */
static tchStatus tch_condv_wake(tch_condvId id){
	tch_condvCb* condv = (tch_condvCb*) id;
	if(!tch_condvIsValid(condv))
		return tchErrorResource;
	if(tch_port_isISR()){                  // if isr mode, no locked mtx is supplied
		if(tch_condvIsWait(condv)){    // check condv is not done
			tch_condvClrWait(condv);   // set condv to done
			tch_schedResumeM((tch_thread_queue*) &condv->wq,1,tchOK,TRUE);
			return tchOK;
		}else{
			return tchErrorParameter;
		}
	}else{
		if(tch_condvIsWait(condv)){
			tch_condvClrWait(condv);
			if(Mtx->unlock(condv->wakeMtx) != tchOK)          // if mtx is not locked by this thread
				return tchErrorResource;
			tch_port_enterSv(SV_THREAD_RESUME,(uint32_t)&condv->wq,tchOK);   // wake single thread from wait queue
			return Mtx->lock(condv->wakeMtx,osWaitForever);                        // lock mtx and return
		}
		return tchErrorParameter;
	}
}

static tchStatus tch_condv_wakeAll(tch_condvId id){
	tch_condvCb* condv = (tch_condvCb*) id;
	if(!tch_condvIsValid(condv))
		return tchErrorResource;
	if(tch_port_isISR()){
		if(tch_condvIsWait(condv)){
			tch_condvClrWait(condv);
			tch_schedResumeM((tch_thread_queue*) &condv->wq,SCHED_THREAD_ALL,tchOK,TRUE);
			return tchOK;
		}else{
			return tchErrorParameter;
		}
	}else {
		if(tch_condvIsWait(condv)){
			tch_condvClrWait(condv);
			if(Mtx->unlock(condv->wakeMtx) != tchOK)
				return tchErrorParameter;
			tch_port_enterSv(SV_THREAD_RESUMEALL,(uword_t)&condv->wq,tchOK);
			return Mtx->lock(condv->wakeMtx,osWaitForever);
		}
		return tchErrorResource;
	}
}

static tchStatus tch_condv_destroy(tch_condvId id){
	tch_condvCb* condv = (tch_condvCb*) id;
	tchStatus result = tchOK;
	if(!tch_condvIsValid(condv))
		return tchErrorResource;
	if(tch_port_isISR()){
		return tchErrorISR;
	}else{
		Mtx->lock(condv->wakeMtx,osWaitForever);
		tch_condvInvalidate(condv);
		result = tch_port_enterSv(SV_THREAD_RESUMEALL,(uword_t)&condv->wq,tchErrorResource);
		Mtx->unlock(condv->wakeMtx);
		shMem->free(condv);
		return result;
	}
}

static inline void tch_condvValidate(tch_condvId condId){
	((tch_condvCb*) condId)->state |= (((((uint32_t) condId) & 0xFFFF) ^ TCH_CONDV_CLASS_KEY) << 2);
}

static inline void tch_condvInvalidate(tch_condvId condId){
	((tch_condvCb*) condId)->state &= ~(0xFFFF << 2);
}

static inline BOOL tch_condvIsValid(tch_condvId condv){
	return ((((tch_condvCb*) condv)->state  >> 2) & 0xFFFF) ==  ((((uint32_t) condv) & 0xFFFF) ^ TCH_CONDV_CLASS_KEY);
}