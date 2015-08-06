/*
 * tch_condv.c
 *
 *  Created on: 2014. 8. 15.
 *      Author: innocentevil
 */

#include "tch.h"
#include "kernel/tch_kernel.h"
#include "kernel/tch_condv.h"


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
		.create = tch_condv_create,
		.wait = tch_condv_wait,
		.wake = tch_condv_wake,
		.wakeAll = tch_condv_wakeAll,
		.destroy = tch_condv_destroy
};


const tch_condv_ix* Condv = &CondVar_StaticInstance;



tch_condvId tchk_condvInit(tch_condvCb* condv,BOOL is_static){
	memset(condv,0,sizeof(tch_condvCb));
	cdsl_dlistInit((cdsl_dlistNode_t*)&condv->wq);
	condv->waitMtx = NULL;
	tch_condvValidate(condv);
	condv->__obj.__destr_fn =  is_static? (tch_kobjDestr)__tch_noop_destr : (tch_kobjDestr)tch_condv_destroy;
	return (tch_condvId) condv;
}


static tch_condvId tch_condv_create(){
	tch_condvCb* condv = (tch_condvCb*) kmalloc(sizeof(tch_condvCb),FALSE);
	if(!condv)
		return NULL;
	if(tch_port_isISR()){
		tchk_condvInit(condv,FALSE);
		return (tch_condvId) condv;
	}
	return (tch_condvId) tch_port_enterSv(SV_CONDV_INIT,condv,FALSE,0);
}

/*! \brief thread wait until given condition is met
 *  \
 *
 */
static tchStatus tch_condv_wait(tch_condvId id,tch_mtxId lock,uint32_t timeout){
	tch_condvCb* condv = (tch_condvCb*) id;
	if(!tch_condvIsValid(condv))
		return tchErrorResource;
	condv->waitMtx = lock;
	if(tch_port_isISR()){
		return tchErrorISR;
	}
	tch_condvSetWait(condv);
	Mtx->unlock(lock);
	tchStatus result = tchOK;
	if((result = tch_port_enterSv(SV_THREAD_SUSPEND,(uint32_t)&condv->wq,timeout,0)) != tchOK){
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
	if((result = Mtx->lock(lock,tchWaitForever)) != tchOK)
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
			tchk_schedThreadResumeM((tch_thread_queue*) &condv->wq,1,tchOK,TRUE);
			return tchOK;
		}else{
			return tchErrorParameter;
		}
	}else{
		if(tch_condvIsWait(condv)){
			tch_condvClrWait(condv);
			if(Mtx->unlock(condv->waitMtx) != tchOK)          // if mtx is not locked by this thread
				return tchErrorResource;
			tch_port_enterSv(SV_THREAD_RESUME,(uint32_t)&condv->wq,tchOK,0);   // wake single thread from wait queue
			return Mtx->lock(condv->waitMtx,tchWaitForever);                        // lock mtx and return
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
			tchk_schedThreadResumeM((tch_thread_queue*) &condv->wq,SCHED_THREAD_ALL,tchOK,TRUE);
			return tchOK;
		}else{
			return tchErrorParameter;
		}
	}else {
		if(tch_condvIsWait(condv)){
			tch_condvClrWait(condv);
			if(Mtx->unlock(condv->waitMtx) != tchOK)
				return tchErrorParameter;
			tch_port_enterSv(SV_THREAD_RESUMEALL,(uword_t)&condv->wq,tchOK,0);
			return Mtx->lock(condv->waitMtx,tchWaitForever);
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
		Mtx->lock(condv->waitMtx,tchWaitForever);
		tch_condvInvalidate(condv);
		result = tch_port_enterSv(SV_THREAD_RESUMEALL,(uword_t)&condv->wq,tchErrorResource,0);
		Mtx->unlock(condv->waitMtx);
		kfree(condv);
		return result;
	}
}



tchStatus tchk_condvWait(tch_condvId condv,tch_mtxId mtx,uint32_t timeout){

}

tchStatus tchk_condvWake(tch_condvId condv){

}

tchStatus tchk_condvWakeAll(tch_condvId condv){

}

tchStatus tchk_condvDestroy(tch_condvId condv){

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

