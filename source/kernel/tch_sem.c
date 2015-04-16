/*
 * tch_sem.c
 *
 *  Created on: 2014. 8. 19.
 *      Author: innocentevil
 */


#include "tch_ktypes.h"
#include "tch_kernel.h"
#include "tch_sem.h"
#include "tch_list.h"


#define TCH_SEMAPHORE_CLASS_KEY                      ((uint16_t) 0x1A0A)


typedef struct _tch_sem_t {
	tch_uobj          __obj;
	uint32_t          state;
	uint32_t          count;
	tch_lnode       wq;
} tch_semaphore_cb;

static tch_semId tch_semaphore_create(uint32_t count);
static tchStatus tch_semaphore_wait(tch_semId sid,uint32_t timeout);
static tchStatus tch_semaphore_unlock(tch_semId sid);
static tchStatus tch_semaphore_destroy(tch_semId sid);

static void tch_semaphoreValidate(tch_semId sid);
static void tch_semaphoreInvalidate(tch_semId sid);
static BOOL tch_semaphoreIsValid(tch_semId sid);



__attribute__((section(".data"))) static tch_semaph_ix Semaphore_StaticInstance = {
		tch_semaphore_create,
		tch_semaphore_wait,
		tch_semaphore_unlock,
		tch_semaphore_destroy
};

const tch_semaph_ix* Sem = (const tch_semaph_ix*) &Semaphore_StaticInstance;



static tch_semId tch_semaphore_create(uint32_t count){
	tch_semaphore_cb* sem = (tch_semaphore_cb*) tch_shMemAlloc(sizeof(tch_semaphore_cb),FALSE);
	if(!sem)
		return NULL;
	sem->count = count;
	sem->__obj.destructor = (tch_uobjDestr) tch_semaphore_destroy;
	tch_listInit(&sem->wq);
	tch_semaphoreValidate(sem);
	return (tch_semId) sem;
}

static tchStatus tch_semaphore_wait(tch_semId id,uint32_t timeout){
	if(tch_port_isISR())
		return tchErrorISR;
	if(!tch_semaphoreIsValid(id))
		return tchErrorResource;
	tch_semaphore_cb* sem = (tch_semaphore_cb*) id;
	tchStatus result = tchOK;
	while(!tch_port_exclusiveCompareDecrement((int*)&sem->count,0)){                // try to exclusively decrement count value
		result = tch_port_enterSv(SV_THREAD_SUSPEND,(uint32_t)&sem->wq,timeout);
		if(!tch_semaphoreIsValid(id))                                                            // validity of semaphore is double-checked because waiting thread is siganled and in ready state before semaphore destroyed
			return tchErrorResource;
		switch(result){
		case tchEventTimeout:                                   // if timeout expires, sv call will return tchEventTimeout
			return tchErrorTimeoutResource;
		case tchErrorResource:                                  // if semaphore destroyed sv call will return tchErrorResource
			return tchErrorResource;
		}
	}
	if(result == tchOK)
		return tchOK;
	return tchErrorOS;                      // unreachable code
}

static tchStatus tch_semaphore_unlock(tch_semId id){
	if(!tch_semaphoreIsValid(id))
		return tchErrorResource;
	tch_semaphore_cb* sem = (tch_semaphore_cb*) id;
	sem->count++;
	if(!tch_listIsEmpty(&sem->wq)){
		if(tch_port_isISR())
			tchk_schedThreadResumeM((tch_thread_queue*) &sem->wq,SCHED_THREAD_ALL,tchOK,TRUE);
		else
			tch_port_enterSv(SV_THREAD_RESUMEALL,(uint32_t)&sem->wq,tchOK);
	}
	return tchOK;
}

static tchStatus tch_semaphore_destroy(tch_semId id){
	if(!tch_semaphoreIsValid(id))
		return tchErrorParameter;
	tch_semaphore_cb* sem = (tch_semaphore_cb*) id;
	sem->state = 0;
	sem->count = 0;
	tch_semaphoreInvalidate(id);
	if(!tch_listIsEmpty(&sem->wq)){
		if(tch_port_isISR())
			tchk_schedThreadResumeM((tch_thread_queue*) &sem->wq,SCHED_THREAD_ALL,tchOK,TRUE);
		else
			tch_port_enterSv(SV_THREAD_RESUMEALL,(uint32_t)&sem->wq,tchErrorResource);
	}
	tch_shMemFree(sem);
	return  tchOK;
}


static void tch_semaphoreValidate(tch_semId sid){
	((tch_semaphore_cb*) sid)->state |= (((uint32_t) sid & 0xFFFF) ^ TCH_SEMAPHORE_CLASS_KEY);
}

static void tch_semaphoreInvalidate(tch_semId sid){
	((tch_semaphore_cb*) sid)->state &= ~(0xFFFF);
}

static BOOL tch_semaphoreIsValid(tch_semId sid){
	return (((tch_semaphore_cb*) sid)->state & 0xFFFF) == (((uint32_t) sid & 0xFFFF) ^ TCH_SEMAPHORE_CLASS_KEY);
}
