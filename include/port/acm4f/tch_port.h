/*
 * tch_port.h
 *
 *  Created on: 2014. 6. 13.
 *      Author: innocentevil
 */

#ifndef TCH_PORT_H_
#define TCH_PORT_H_

#include "tch_portcfg.h"

/****
 *  define exception entry / exit stack data structure
 */
typedef struct _tch_exc_stack_t tch_exc_stack;
typedef struct _tch_thread_context_t tch_thread_context;
typedef struct _tch_port_ix tch_port_ix;

typedef struct _arm_sbrt_ctx arm_sbrt_ctx;
typedef struct _arm_exc_stack arm_exc_stack;





/***
 *  port interface
 */
struct _tch_port_ix {

	void (*_enableISR)(void);
	void (*_disableISR)(void);
	/***
	 *  Kernal lock action
	 *   - to guarantee kernel operation not interrupted or preempted
	 */
	void (*_kernel_lock)(void);
	/***
	 *  Kernel Unlock action
	 *   - to allow interrupt or thread preemption when it's needed
	 */
	void (*_kernel_unlock)(void);

	void (*_switchContext)(void* nth,void* cth);
	void (*_saveContext)(void* cth);
	void (*_restoreContext)(void* cth);
	void (*_returnToKernelModeThread)(void* routine,void* arg1,void* arg2);
	int (*_enterSvFromUsr)(int sv_id,void* arg1,void* arg2);
	int (*_enterSvFromIsr)(int sv_id,void* arg1,void* arg2);
};

const tch_port_ix* tch_port_init();


typedef struct _arm_sbrtn_ctx arm_sbrtn_ctx;


struct _arm_sbrt_ctx{
	uint32_t R4;
	uint32_t R5;
	uint32_t R6;
	uint32_t R7;
	uint32_t R8;
	uint32_t R9;
	uint32_t R10;
	uint32_t R11;
	uint32_t LR;
};


/**
 * follows arm v7m arch. ref.
 * exception push & pop registers below in the stack at the entry and exit
 */
struct _arm_exc_stack{
	uint32_t R0;
	uint32_t R1;
	uint32_t R2;
	uint32_t R3;
	uint32_t R12;
	uint32_t LR14;
	uint32_t Return;
	uint32_t xPSR;
#ifdef FEATURE_HFLOAT
	uint32_t S0;
	uint32_t S1;
	uint32_t S2;
	uint32_t S3;
	uint32_t S4;
	uint32_t S5;
	uint32_t S6;
	uint32_t S7;
	uint32_t S8;
	uint32_t S9;
	uint32_t S10;
	uint32_t S11;
	uint32_t S12;
	uint32_t S13;
	uint32_t S14;
	uint32_t S15;
	uint32_t FPSCR;
	uint32_t RESV;
#endif
} __attribute__((aligned(8)));

struct _tch_exc_stack_t {
	arm_exc_stack* exc_sp;
};

struct _tch_thread_context_t {
	arm_sbrt_ctx* sp;
};




#endif /* TCH_PORT_H_ */
