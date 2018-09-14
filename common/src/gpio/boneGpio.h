
/******************************************************************************
*
* RTS Traffic Light Project 2018
*
*****************************************************************************/
/**
* @file     boneGpio.h
* @brief    RTS Traffic Light Project
* @author   Michael Stekla	s3545953
* 			Simon Maddison	s3493550
* 			Shawn Buschmann	s3478646
*/


#ifndef SRC_GPIO_BONEGPIO_H_
#define SRC_GPIO_BONEGPIO_H_


/*-----------------------------------------------------------------------------
* Included Files
*---------------------------------------------------------------------------*/

#include "../../public/debug.h"

#include <stdlib.h>
#include <stdio.h>
#include <hw/inout.h>      // for in32() and out32();
#include <sys/mman.h>      // for mmap_device_io();
#include <sys/neutrino.h>  // for ThreadCtl( _NTO_TCTL_IO_PRIV , NULL);
#include <stdint.h>		   // for unit32 types



/*-----------------------------------------------------------------------------
* Definitions
*---------------------------------------------------------------------------*/


#define LED0	(1<<21)   // GPIO1_21
#define LED1	(1<<22)   // GPIO1_22
#define LED2	(1<<23)   // GPIO1_23
#define LED3	(1<<24)   // GPIO1_24




/*-----------------------------------------------------------------------------
* Global Variables and Buffers
*---------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
* Global Function Declarations
*---------------------------------------------------------------------------*/




/* ----------------------------------------------------	*
 *	@getMsgQueId writeIObeagleLeds:						*
 *	@breif: 											*
 *	@return:returns true or false for file				*
 * ---------------------------------------------------	*/
bool writeBoneLeds(volatile uint32_t val = LED0|LED1|LED2|LED3);


#endif /* SRC_GPIO_BONEGPIO_H_ */
