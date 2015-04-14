/*
 * bar_test.c
 *
 *  Created on: 2014. 8. 30.
 *      Author: innocentevil
 */



/*! unit test for barrier
 *  -> 3 Thread(s) is initiated and wait in barrier
 *  -> one of the thread will signal main thread before wait in barrier
 *  -> main thread is woke up and yield 50 ms to guarantee all child thread in waiting
 *  -> and unlock barrier ==> all child thread should get 'osOk'
 *
 *
 *
 *
 */

#include "tch.h"
#include "bar_test.h"

static DECLARE_THREADROUTINE(child1Routine);
static DECLARE_THREADROUTINE(child2Routine);
static DECLARE_THREADROUTINE(child3Routine);

static tch_threadId child1Id;
static tch_threadId child2Id;
static tch_threadId child3Id;
static tch_threadId mainId;

static tch_barId bid;

tchStatus barrier_performTest(tch* api){
	mainId = api->Thread->self();


	tch_threadCfg tcfg;
	tcfg.t_name = "child1";
	tcfg.t_routine = child1Routine;
	tcfg.t_priority = Normal;
	tcfg.t_memDef.stk_sz = 512;

	child1Id = api->Thread->create(&tcfg,api);

	tcfg.t_name = "child2";
	tcfg.t_routine = child2Routine;
	tcfg.t_priority = Normal;
	tcfg.t_memDef.stk_sz = 512;
	child2Id = api->Thread->create(&tcfg,api);


	tcfg.t_name = "child3";
	tcfg.t_routine = child3Routine;
	tcfg.t_priority = Normal;
	tcfg.t_memDef.stk_sz = 512;
	child3Id = api->Thread->create(&tcfg,api);

	tch_assert(api,child1Id && child2Id && child3Id,tchErrorOS);

	bid = api->Barrier->create();

	api->Thread->start(child1Id);
	api->Thread->start(child2Id);
	api->Thread->start(child3Id);


	api->Thread->yield(10);
	api->Barrier->signal(bid,tchOK);
	if(api->Barrier->wait(bid,tchWaitForever) != tchErrorResource)
		return tchErrorOS;

	if(api->Thread->join(child1Id,tchWaitForever) != tchOK)
		return tchErrorOS;
	if(api->Thread->join(child2Id,tchWaitForever) != tchOK)
		return tchErrorOS;
	if(api->Thread->join(child3Id,tchWaitForever) != tchOK)
		return tchErrorOS;



	return tchOK;
}


static DECLARE_THREADROUTINE(child1Routine){
	if(env->Barrier->wait(bid,tchWaitForever) != tchOK)
		return tchErrorOS;
	if(env->Barrier->wait(bid,tchWaitForever) != tchErrorResource)
		return tchErrorOS;
	return tchOK;
}

static DECLARE_THREADROUTINE(child2Routine){
	if(env->Barrier->wait(bid,tchWaitForever) != tchOK)
		return tchErrorOS;
	if(env->Barrier->wait(bid,tchWaitForever) != tchErrorResource)
		return tchErrorOS;
	return tchOK;
}

static DECLARE_THREADROUTINE(child3Routine){
	if(env->Barrier->wait(bid,tchWaitForever) != tchOK)
		return tchErrorOS;
	env->Thread->yield(20);
	env->Barrier->destroy(bid);
	return tchOK;
}
