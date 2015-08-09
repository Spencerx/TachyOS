/*
 * tch_kernel.h
 *
 *
 * Copyright (C) 2014 doowoong,lee
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the LGPL v3 license.  See the LICENSE file for details.
 *
 *  Created on: 2014. 6. 22.
 *      Author: innocentevil
 */

#ifndef TCH_KERNEL_H_
#define TCH_KERNEL_H_

/***
 *   Kernel Interface
 *   - All kernel functions(like scheduling,synchronization) are based on this module
 *   - Initialize kernel enviroment (init kernel internal objects,
 */
#include "tch.h"
#include "tch_board.h"
#include "tch_ktypes.h"
#include "tch_port.h"
#include "tch_sched.h"
#include "tch_hal.h"
#include "tch_mm.h"



#if !defined(__BUILD_TIME_EPOCH)
#define __BUILD_TIME_EPOCH 0UL
#endif

#ifndef offsetof
#define offsetof(type,member)					(size_t)(&((type*) 0)->member)
#endif


#ifndef container_of
#define container_of(ptr,type,member) 		(type*) (((size_t) ptr - (size_t) offsetof(type,member)))
#endif


/**
 *  syscall definition
 */
#define __SYSCALL__	__attribute__((section(".sysc.table")))

typedef uword_t (*tch_syscall)(uint32_t arg1,uint32_t arg2,uint32_t arg3);
extern uint32_t __syscall_entry;

#define DECLARE_SYSCALL_3(fn,t1,t2,t3,rt) \
	static rt __##fn(t1,t2,t3)

#define DEFINE_SYSCALL_3(fn,t1,p1,t2,p2,t3,p3,rt)	\
	const __SYSCALL__ void* __entry__##fn = __##fn;\
	static rt __##fn(t1 p1,t2 p2, t3 p3)

#define __SYSCALL_3(fn,arg1,arg2,arg3) \
	tch_port_enterSv((uint32_t) &__entry__##fn - (uint32_t) &__syscall_entry,(uword_t) arg1,(uword_t) arg2,(uword_t) arg3)


#define DECLARE_SYSCALL_2(fn,t1,t2,rt) \
	static rt __##fn(t1, t2)

#define DEFINE_SYSCALL_2(fn,t1,p1,t2,p2,rt)  \
	const __SYSCALL__ void* __entry__##fn = __##fn;\
	static rt __##fn(t1 p1,t2 p2)

#define __SYSCALL_2(fn,arg1,arg2) \
	tch_port_enterSv((uint32_t) &__entry__##fn - (uint32_t) &__syscall_entry,(uword_t) arg1, (uword_t) arg2,0)


#define DECLARE_SYSCALL_1(fn,t1,rt) \
	static rt __##fn(t1)

#define DEFINE_SYSCALL_1(fn,t1,p1,rt) \
	const __SYSCALL__ void* __entry__##fn = __##fn;\
	static rt __##fn(t1 p1)

#define __SYSCALL_1(fn,arg1)\
	tch_port_enterSv((uint32_t) &__entry__##fn - (uint32_t) &__syscall_entry,(uword_t) arg1,0,0)


#define DECLARE_SYSCALL_0(fn,rt) \
	static rt __##fn(void)

#define DEFINE_SYSCALL_0(fn,rt) \
	const __SYSCALL__ void* __entry__##fn = __##fn;\
	static rt __##fn(void)

#define __SYSCALL_0(fn)\
	tch_port_enterSv((uint32_t) &__entry__##fn - (uint32_t) &__syscall_entry,0,0,0)





#define TCH_SYS_TASKQ_SZ                    (16)
#define tchk_kernelSetResult(th,result)		((tch_thread_uheader*) th)->kRet = (uint32_t) result
#define getThreadHeader(th_id)  			((tch_thread_uheader*) th_id)
#define getThreadKHeader(th_id) 			(((tch_thread_uheader*) th_id)->kthread)

/*!
 * \brief
 */
extern __attribute__((naked)) void __init(void* arg);
extern void tch_kernelInit(void* arg);
extern void tch_kernelOnSvCall(uint32_t sv_id,uint32_t arg1, uint32_t arg2,uint32_t arg3);
extern void tch_KernelOnSystick();
extern void tch_kernelOnWakeup();


extern void tch_kernelOnMemFault(paddr_t pa, int fault);
extern void tch_kernelOnHardFault(int fault);


extern tchStatus tch_kernelPostSysTask(int id,tch_sysTaskFn fn,void* arg);
extern tchStatus tch_kernelEnableInterrupt(IRQn_Type irq,uint32_t priority);
extern tchStatus tch_kernelDisableInterrupt(IRQn_Type irq);




extern tchStatus __tch_noop_destr(tch_kobj* obj);
extern void __tchk_thread_atexit(tch_threadId thread,int res) __attribute__((naked));


/**\!brief Notify kernel that system is busy, so system should be prevented from going into sleep mode
 *
 */
extern void tch_kernelSetBusyMark();

/**\!brief Notify kernel that busy task is finished, so system can be go into sleep mode
 *
 */
extern void tch_kernelClrBusyMark();

/**\!brief check whether system is busy
 *
 */
extern BOOL tch_kernelIsBusy();


/**
 * Kernel API Struct* List
 * - are bound statically in compile time
 */
extern const tch_thread_ix* Thread;
extern const tch_condv_ix* Condv;
extern const tch_mtx_ix* Mtx;
extern const tch_semaph_ix* Sem;
extern const tch_bar_ix* Barrier;
extern const tch_msgq_ix* MsgQ;
extern const tch_event_ix* Event;
extern const tch_mailq_ix* MailQ;
extern const tch_mpool_ix* Mempool;
extern const tch_mem_ix* uMem;
extern const tch_ustdlib_ix* uStdLib;
extern const tch_event_ix* Event;
extern const tch_signal_ix* Sig;

extern const tch_hal* Hal;


extern const tch_kernel_descriptor kernel_descriptor;
extern tch_thread_uheader* tch_currentThread;
extern volatile uint64_t tch_systimeTick;
extern tch_thread_queue procList;
extern const tch* tch_rti;
extern tch_boardParam boardHandle;
extern tch_memId sharedMem;
extern BOOL __VALID_SYSCALL;



#endif /* TCH_KERNEL_H_ */
