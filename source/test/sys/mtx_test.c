/*
 * mtx_test.c
 *
 *  Created on: 2014. 8. 17.
 *      Author: manics99
 */


#include "tch.h"
#include "mtx_test.h"


static DECLARE_THREADROUTINE(child1Routine);
static DECLARE_THREADROUTINE(child2Routine);

static tch_threadId child1;
static tch_threadId child2;

static void race(tch* api);
static tch_mtxId mmtx;


/*!
 * \brief Mtx Unit Test
 */
tchStatus mtx_performTest(tch* api){



	const tch_thread_ix* Thread = api->Thread;
	tch_threadCfg thCfg;
	thCfg._t_name = "child1_mtx";
	thCfg._t_routine = child1Routine;
	thCfg.t_proior = Normal;
	thCfg.t_stackSize = 512;
	child1 = Thread->create(&thCfg,api);

	thCfg._t_name = "child2_mtx";
	thCfg._t_routine = child2Routine;
	thCfg.t_stackSize = 512;
	thCfg.t_proior = Normal;
	child2 = Thread->create(&thCfg,api);

	mmtx = api->Mtx->create();
	if(api->Mtx->lock(mmtx,tchWaitForever) != tchOK)
		return tchErrorOS;
	api->Thread->start(child1);
	api->Thread->yield(200);
	api->Mtx->unlock(mmtx);


	tchStatus result = api->Thread->join(child1,tchWaitForever);

	return result;

}


static void race(tch* api){
	uint32_t idx = 0;
	const tch_mtx_ix* Mtx = api->Mtx;
	for(idx = 0;idx < 100;idx++){
		if(Mtx->lock(mmtx,tchWaitForever) == tchOK){
			api->Thread->yield(0);
			Mtx->unlock(mmtx);
		}
	}
}

static DECLARE_THREADROUTINE(child1Routine){
	tchStatus result = tchOK;
	if((result = env->Mtx->lock(mmtx,10)) != tchErrorTimeoutResource)    // Timeout expected
		return tchErrorOS;
	if((result = env->Mtx->unlock(mmtx)) != tchErrorResource)
		return tchErrorOS;
	if((result = env->Mtx->lock(mmtx,tchWaitForever)) != tchOK)
		return tchErrorOS;
	env->Thread->start(child2);
	race(env);
	env->Mtx->destroy(mmtx);
	return env->Thread->join(child2,tchWaitForever);
}

static DECLARE_THREADROUTINE(child2Routine){
	race(env);
	tchStatus result = tchOK;
	env->Thread->yield(50);
	if((result = env->Mtx->lock(mmtx,tchWaitForever)) == tchErrorResource)
		return tchOK;
	return tchErrorOS;
}
