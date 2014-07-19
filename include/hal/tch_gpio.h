/*
 * tch_gpio.h
 *
 * Copyright (C) 2014 doowoong,lee
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the LGPL v3 license.  See the LICENSE file for details.
 *
 *
 *  Created on: 2014. 6. 15.
 *      Author: innocentevil
 */

#ifndef TCH_GPIO_H_
#define TCH_GPIO_H_

#include "tch.h"
#include "tch_gpio.h"


typedef enum {GPIO_1,GPIO_2,GPIO_3,GPIO_4,GPIO_5,GPIO_6,GPIO_7,GPIO_8,GPIO_9,GPIO_10,GPIO_11} ioport_Num;
typedef enum {GPIO_A,GPIO_B} ioport_Alph;
typedef union {
	ioport_Num;
	ioport_Alph;
} ioport_t;
typedef struct tch_gpio_handle tch_gpio_handle;

/**
 * gpio handle interface
 */
typedef struct tch_gpio_evCfg {
	enum {Rising,Falling,Both}Edge;
	enum {Intrrupt,Event}Type;
	uint8_t GPIO_Evt_Type;
}tch_gpio_evCfg;

typedef struct tch_gpio_cfg {
	enum {Out,In,Analog,Function}Mode;
	enum {PushPull,OpenDrain}Otype;
	enum {IoSpeedLow,IoSpeedMid,IoSpeedHigh,IoSpeedVeryHigh}OSpeed;
	enum {Pullup,Pulldown,Float}PuPd;
	uint8_t Af;
}tch_gpio_cfg;

typedef struct tch_gpio_handle {
	/**
	 * write gpio port's output value
	 * @param self handle object
	 * @param val bit flags value to be written into gpio(s)
	 */
	void (*out)(tch_gpio_handle* self,uint16_t pin,tch_bState nstate);
	tch_bState (*in)(tch_gpio_handle* self);
	void (*writePort)(tch_gpio_handle* self,uint16_t val);
	uint16_t (*readPort)(tch_gpio_handle* self);
	void (*registerIoEvent)(tch_gpio_handle* self,uint16_t pin);
}tch_gpio_handle;


typedef struct tch_lld_gpio {
	tch_gpio_handle* (*allocIo)(const ioport_t port,uint16_t pin,const tch_gpio_cfg* cfg,tch_pwr_def pcfg);
	void (*freeIo)(tch_gpio_handle* IoHandle);
}tch_lld_gpio;

extern const tch_lld_gpio* tch_gpio_instance;

#endif /* TCH_GPIO_H_ */
