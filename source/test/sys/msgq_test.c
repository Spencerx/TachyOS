/*
 * msgq_test.c
 *
 *  Copyright (C) 2014 doowoong,lee
 *  All rights reserved.
 *
 *  This software may be modified and distributed under the terms
 *  of the LGPL v3 license.  See the LICENSE file for details.
 *
 *
 *  Created on: 2014. 7. 6.
 *      Author: innocentevil
 */



#include "msgq_test.h"

static DECLARE_THREADROUTINE(sender);
static tch_threadId sender_id;

static DECLARE_THREADROUTINE(receiver);
static tch_threadId receiver_id;

static DECLARE_IO_CALLBACK(ioEventListener);

static tch_msgqId mid;
static tch_barId mBar;

static tch_GpioHandle* out;
static tch_GpioHandle* in;

static volatile tch* Api;
static int irqcnt;
static int usrcnt;
static int miscnt;

tchStatus msgq_performTest(tch* api){
	Api = api;
	mBar = api->Barrier->create();
	irqcnt = 0;
	miscnt = 0;
	usrcnt = 0;


	const tch_msgq_ix* MsgQ = api->MsgQ;
	mid = MsgQ->create(10);

	tch_GpioCfg iocfg;
	api->Device->gpio->initCfg(&iocfg);
	iocfg.Mode = GPIO_Mode_OUT;
	iocfg.Otype = GPIO_Otype_OD;
	iocfg.PuPd = GPIO_PuPd_PU;
	iocfg.Speed = GPIO_OSpeed_50M;
	out = api->Device->gpio->allocIo(api,tch_gpio0,1 << 2,&iocfg,tchWaitForever);
	out->out(out,1 << 2,bSet);

	iocfg.Mode = GPIO_Mode_IN;
	iocfg.PuPd = GPIO_PuPd_PU;;
	iocfg.Speed = GPIO_OSpeed_50M;
	in = api->Device->gpio->allocIo(api,tch_gpio0,1 << 0,&iocfg,tchWaitForever);

	tch_GpioEvCfg evcfg;
	api->Device->gpio->initEvCfg(&evcfg);
	evcfg.EvEdge = GPIO_EvEdge_Fall;
	evcfg.EvType = GPIO_EvType_Interrupt;
	evcfg.EvCallback = ioEventListener;
	uint32_t pmsk = 1 << 0;
	in->registerIoEvent(in,&evcfg,&pmsk);

	const tch_thread_ix* Thread = api->Thread;
	tch_threadCfg tcfg;
	api->Thread->initCfg(&tcfg);
	tcfg.t_name = "sender";
	tcfg.t_routine = sender;
	tcfg.t_priority= Normal;
	tcfg.t_memDef.stk_sz= 1 << 9;
	sender_id = Thread->create(&tcfg,api);

	tcfg.t_name = "receiver";
	tcfg.t_routine = receiver;
	receiver_id = Thread->create(&tcfg,api);

	Thread->start(receiver_id);
	Thread->start(sender_id);


	tch_assert(api,Thread->join(receiver_id,tchWaitForever) == tchOK,tchErrorOS);
	tch_assert(api,Thread->join(sender_id,tchWaitForever) == tchOK,tchErrorOS);
	api->Barrier->destroy(mBar);


	in->unregisterIoEvent(in,1 << 0);

	out->close(out);
	in->close(in);

	return tchOK;

}


static DECLARE_THREADROUTINE(sender){
	uint32_t cnt = 0;
	while(cnt < 50){
		env->MsgQ->put(mid,0xFF,tchWaitForever);
		out->out(out,1 << 2,bClear);
		env->Thread->yield(1);
		out->out(out,1 << 2,bSet);
		env->Thread->yield(1);
		cnt++;
	}
	env->Barrier->wait(mBar,tchWaitForever);
	env->Thread->yield(10);
	env->MsgQ->destroy(mid);
	return tchOK;
}

static DECLARE_THREADROUTINE(receiver){
	uint32_t cnt = 0;
	tchEvent evt;
	uint32_t mval = 0;
	uint32_t totalMsgcnt = 0;
	while(cnt < 100){
		evt = env->MsgQ->get(mid,tchWaitForever);
		totalMsgcnt++;
		if(evt.status == tchEventMessage){
			cnt++;
			if(evt.value.v == 0xFF){
				usrcnt++;
			}else if(evt.value.v == 0xF0){
				irqcnt++;
			}else{
				mval = evt.value.v;
				miscnt++;
			}
		}
	}
	evt = env->MsgQ->get(mid,10);
	if(evt.status != tchErrorTimeoutResource)
		return tchErrorOS;
	env->Barrier->signal(mBar,tchOK);
	evt = env->MsgQ->get(mid,tchWaitForever);
	if(evt.status != tchErrorResource)
		return tchErrorResource;
	return tchOK;
}

static DECLARE_IO_CALLBACK(ioEventListener){
	Api->MsgQ->put(mid,0xF0,0);
	return TRUE;
}

