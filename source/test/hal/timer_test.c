/*
 * timer_test.c
 *
 *  Created on: 2014. 10. 13.
 *      Author: innocentevil
 */


#include "tch.h"
#include "timer_test.h"


static DECLARE_THREADROUTINE(waiter1Run);
static DECLARE_THREADROUTINE(waiter2Run);

static DECLARE_THREADROUTINE(pulsDrv1Run);
static DECLARE_THREADROUTINE(pulsDrv2Run);

tchStatus timer_performTest(tch* env){

	tchStatus result = osOK;

	tch_gptimerDef gptDef;
	gptDef.UnitTime = env->Device->timer->UnitTime.mSec;
	gptDef.pwrOpt = ActOnSleep;

	tch_gptimerHandle* gptimer = env->Device->timer->openGpTimer(env,env->Device->timer->timer.timer0,&gptDef,osWaitForever);

	tch_threadId waiterThread1;
	tch_threadId waiterThread2;

	uint8_t* waiterThread1Stk = (uint8_t*) env->Mem->alloc(1 << 9);
	uint8_t* waiterThread2Stk = (uint8_t*) env->Mem->alloc(1 << 9);

	tch_threadCfg thcfg;
	thcfg._t_name = "Waiter1";
	thcfg._t_routine = waiter1Run;
	thcfg._t_stack = waiterThread1Stk;
	thcfg.t_proior = Normal;
	thcfg.t_stackSize = 1 << 9;


	waiterThread1 = env->Thread->create(&thcfg,gptimer);


	thcfg._t_name = "Waiter2";
	thcfg._t_routine = waiter2Run;
	thcfg._t_stack = waiterThread2Stk;
	thcfg.t_proior = Normal;
	thcfg.t_stackSize = 1 << 9;

	waiterThread2 = env->Thread->create(&thcfg,gptimer);


	env->Thread->start(waiterThread1);
	env->Thread->start(waiterThread2);

	result = env->Thread->join(waiterThread1,osWaitForever);
	result = env->Thread->join(waiterThread2,osWaitForever);

	gptimer->close(gptimer);   // gptimer test complete

	if(result != osOK)
		return osErrorOS;


	tch_pwmDef pwmDef;
	pwmDef.PeriodInUnitTime = 1000;
	pwmDef.UnitTime = env->Device->timer->UnitTime.uSec;
	pwmDef.pwrOpt = ActOnSleep;
	tch_pwmHandle* pwmDrv = env->Device->timer->openPWM(env,env->Device->timer->timer.timer0,&pwmDef,osWaitForever);


	thcfg._t_name = "PulseDrv1";
	thcfg._t_routine = pulsDrv1Run;
	thcfg._t_stack = waiterThread1Stk;
	thcfg.t_proior = Normal;
	thcfg.t_stackSize = 1 << 9;

	waiterThread1 = env->Thread->create(&thcfg,pwmDrv);



	thcfg._t_name = "pulseDrv2";
	thcfg._t_routine = pulsDrv2Run;
	thcfg._t_stack = waiterThread2Stk;
	thcfg.t_proior = Normal;
	thcfg.t_stackSize = 1 << 9;
	waiterThread2 = env->Thread->create(&thcfg,pwmDrv);

	env->Thread->start(waiterThread1);
	env->Thread->start(waiterThread2);

	result = env->Thread->join(waiterThread1,osWaitForever);
	result = env->Thread->join(waiterThread2,osWaitForever);

	pwmDrv->close(pwmDrv);



	env->Mem->free(waiterThread1Stk);
	env->Mem->free(waiterThread2Stk);


	return result;
}


static DECLARE_THREADROUTINE(waiter1Run){
	tch_gptimerHandle* gptimer = (tch_gptimerHandle*)sys->Thread->getArg();
	int cnt = 1000;
	while(cnt--)
		gptimer->wait(gptimer,10);
	return osOK;

}

static DECLARE_THREADROUTINE(waiter2Run){
	tch_gptimerHandle* gptimer = (tch_gptimerHandle*)sys->Thread->getArg();
	int cnt = 1000;
	while(cnt--)
		gptimer->wait(gptimer,10);
	return osOK;
}


static DECLARE_THREADROUTINE(pulsDrv1Run){
	tch_pwmHandle* pwmDrv = (tch_pwmHandle*) sys->Thread->getArg();
	int cnt = 1000;
	float a = 0.f;
	while(cnt--){
		pwmDrv->setDuty(pwmDrv,1,cnt / 1000.f);
		sys->Thread->sleep(1);
	}
	return osOK;
}

static DECLARE_THREADROUTINE(pulsDrv2Run){
	tch_pwmHandle* pwmDrv = (tch_pwmHandle*) sys->Thread->getArg();
	int cnt = 1000;
	while(cnt--){
		pwmDrv->setDuty(pwmDrv,1,cnt / 1000.f);
		sys->Thread->sleep(1);
	}
	return osOK;
}
