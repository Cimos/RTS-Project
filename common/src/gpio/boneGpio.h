
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

//#include "../../public/debug.h"

#include <stdlib.h>
#include <stdio.h>
#include <hw/inout.h>      // for in32() and out32();
#include <sys/mman.h>      // for mmap_device_io();
#include <sys/neutrino.h>  // for ThreadCtl( _NTO_TCTL_IO_PRIV , NULL);
#include <stdint.h>		   // for unit32 types



/*-----------------------------------------------------------------------------
* Definitions
*---------------------------------------------------------------------------*/



// P8
#define gpio1_6 (1<<6)			// P8_6
#define gpio1_6_config 0x818	// Find this in a headerPin table/ Also does not always map to pin 6

#define gpio1_12 (1<<12)		// P8_12
#define gpio1_12_config 0x830

#define gpio1_13 (1<<13)		// P8_11
#define gpio1_13_config 0x834

// P9
#define LCD_RST (1<<16)  // LCD_RST is connected to rst on the click breakout board		pin 16
#define GPMC_A0_GPIO1_16 0x840





#define LED0	(1<<21)   // GPIO1_21
#define LED1	(1<<22)   // GPIO1_22
#define LED2	(1<<23)   // GPIO1_23
#define LED3	(1<<24)   // GPIO1_24


// GPMC_A1_Configuration
#define PIN_MODE_0   0x00
#define PIN_MODE_1   0x01
#define PIN_MODE_2   0x02
#define PIN_MODE_3   0x03
#define PIN_MODE_4   0x04
#define PIN_MODE_5   0x05
#define PIN_MODE_6   0x06
#define PIN_MODE_7   0x07

// PIN MUX Configuration strut values  (page 1420 from TRM)
#define PU_ENABLE    0x00
#define PU_DISABLE   0x01
#define PU_PULL_UP   0x01
#define PU_PULL_DOWN 0x00
#define RECV_ENABLE  0x01
#define RECV_DISABLE 0x00
#define SLEW_FAST    0x00
#define SLEW_SLOW    0x01


typedef union _CONF_MODULE_PIN_STRUCT   // See TRM Page 1420
{
  unsigned int d32;
  struct {    // name: field size
           unsigned int conf_mmode : 3;       // LSB
           unsigned int conf_puden : 1;
           unsigned int conf_putypesel : 1;
           unsigned int conf_rxactive : 1;
           unsigned int conf_slewctrl : 1;
           unsigned int conf_res_1 : 13;      // reserved
           unsigned int conf_res_2 : 12;      // reserved MSB
         } b;
} _CONF_MODULE_PIN;


/*-----------------------------------------------------------------------------
* Global Variables and Buffers
*---------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
* Global Function Declarations
*---------------------------------------------------------------------------*/


/* ----------------------------------------------------	*
 *	@writeBoneLeds:									*
 *	@breif: 											*
 *	@return:returns true or false for file				*
 * ---------------------------------------------------	*/
bool writeBoneLeds(uint32_t pin, bool setTo);

/* ----------------------------------------------------	*
 *	@setPinIOStatus:									*
 *	@breif: also enables pinmux for pin					*
 *	@return:returns true or false for file				*
 * ---------------------------------------------------	*/
bool setPinIOStatus(uint32_t pin, uint32_t pinConfig, bool inOut);	//(inOut) input = 1, output = 0


/* ----------------------------------------------------	*
 *	@writepin_gpio1:									*
 *	@breif: 											*
 *	@return:returns true or false for file				*
 * ---------------------------------------------------	*/
bool writepin_gpio1(uint32_t pin, bool setTo);

#endif /* SRC_GPIO_BONEGPIO_H_ */
