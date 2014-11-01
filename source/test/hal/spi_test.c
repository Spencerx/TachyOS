/*
 * spi_test.c
 *
 *  Created on: 2014. 11. 1.
 *      Author: innocentevil
 */

#include "tch.h"
#include "spi_test.h"

tchStatus spi_performTest(tch* env){
	tch_spiHandle* spihandle = NULL;
	uaddr_t faddr = 0;
	tch_spiCfg spiCfg;
	uint32_t leakTestcnt = 1000;



	spiCfg.Baudrate = env->Device->spi->Baudrate.Normal;
	spiCfg.ClkMode = env->Device->spi->ClkMode.Mode0;
	spiCfg.FrmFormat = env->Device->spi->FrmFormat.Frame8B;
	spiCfg.FrmOrient = env->Device->spi->FrmOrient.LSBFirst;
	spiCfg.OpMode = env->Device->spi->OpMode.Master;


	do{
		spihandle = env->Device->spi->allocSpi(env,env->Device->spi->spi.spi0,&spiCfg,osWaitForever,ActOnSleep);
		env->Device->spi->freeSpi(spihandle);
	}while(leakTestcnt--);



	spihandle = env->Device->spi->allocSpi(env,env->Device->spi->spi.spi0,&spiCfg,osWaitForever,ActOnSleep);
	leakTestcnt = 0;
	while(leakTestcnt++ < 1000){
		spihandle->write(spihandle,"HelloWorld",5);
	}
	return osOK;

}
