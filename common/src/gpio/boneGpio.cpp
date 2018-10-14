
/******************************************************************************
*
* RTS Traffic Light Project 2018
*
*****************************************************************************/
/**
* @file     boneGpio.cpp
* @brief    RTS Traffic Light Project
* @author   Michael Stekla	s3545953
* 			Simon Maddison	s3493550
* 			Shawn Buschmann	s3478646
*/


/*-----------------------------------------------------------------------------
* Included Files
*---------------------------------------------------------------------------*/
#include "boneGpio.h"
#include <hw/inout.h>// for in32() and out32();
#include <sys/mman.h>// for mmap_device_io();
#include <stdint.h>// for unit32 types
#include <sys/neutrino.h>// for ThreadCtl( _NTO_TCTL_IO_PRIV, NULL)
#include <sched.h>
#include <sys/procmgr.h>

/*-----------------------------------------------------------------------------
* Definitions
*---------------------------------------------------------------------------*/

#define PU_ENABLE0x00
#define PU_DISABLE0x01
#define PU_PULL_UP0x01
#define PU_PULL_DOWN0x00
#define RECV_ENABLE0x01
#define RECV_DISABLE0x00
#define SLEW_FAST0x00
#define SLEW_SLOW0x01

// GPMC_A1_Configuration
#define PIN_MODE_0 0x00
#define PIN_MODE_1 0x01
#define PIN_MODE_2 0x02
#define PIN_MODE_3 0x03
#define PIN_MODE_4 0x04
#define PIN_MODE_5 0x05
#define PIN_MODE_6 0x06
#define PIN_MODE_7 0x07

#define AM335X_CONTROL_MODULE_BASE   (uint64_t) 0x44E10000
#define AM335X_CONTROL_MODULE_SIZE   (size_t)   0x00001448
#define AM335X_GPIO_SIZE             (uint64_t) 0x00001000
#define AM335X_GPIO1_BASE            (size_t)   0x4804C000
#define AM335X_GPIO2_BASE            (size_t)   0x481AC000
#define AM335X_GPIO2_BASE            (size_t)   0x481AE000
#define GPIO_OE        0x134
#define GPIO_DATAIN    0x138
#define GPIO_DATAOUT   0x13C


/*-----------------------------------------------------------------------------
* Global Variables and Buffers
*---------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
* Local Function Declarations
*---------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------
* Global Function Implementation
*---------------------------------------------------------------------------*/

/* ----------------------------------------------------	*
 *	@ writeBoneLeds:									*
 *	@breif:  											*
 *	@return:returns true or false for file				*
 * ---------------------------------------------------	*/
bool writeBoneLeds(uint32_t pin, bool setTo)
{
	uintptr_t gpio1_base = 0;

	volatile uint32_t	val = 0;
	volatile uint32_t	tmp = 0;

	gpio1_base = mmap_device_io(AM335X_GPIO_SIZE, AM335X_GPIO1_BASE);

	if( !gpio1_base )
	{
		DEBUGF("Can't map Control Base Module / GPIO Base");
		return false;
	}
	DEBUGF("I/O Port Mapping is Successful!\n");

	// Read GPIO output enable register (data direction in/out)
	//  0 The corresponding GPIO port is configured as an output.
	//  1 The corresponding GPIO port is configured as an input.

	// READ the value of the output enable register for the
	// I/O pins that are connected to the LEDS
	val  = in32(gpio1_base + GPIO_OE);
	DEBUGF("Original value of GPIO_1 output enable register: %#010x\n", val);

	// write value to output enable
	if (setTo)
		val |= pin;
	else
		val &= ~pin;

	out32(gpio1_base + GPIO_OE, val);

	// confirm that GPIO_OE is set to output
	tmp  = in32(gpio1_base + GPIO_OE);
	DEBUGF("New value of GPIO_1 output enable register: %#010x\n", tmp);

	if (tmp != val)
		return false;


	// TODO: is this needed?
	munmap_device_io(gpio1_base, AM335X_GPIO_SIZE);

	DEBUGF("Main Terminated...!\n");
	return true;
}



/* ----------------------------------------------------	*
 *	@setPinIOStatus:									*
 *	@breif:  											*
 *	@return:returns true or false for file				*
 * ---------------------------------------------------	*/
bool setPinIOStatus(uint32_t pin, uint32_t pinConfig, bool inOut)	// input = 1, output = 0
{
	uintptr_t gpio1_base = 0;
	uintptr_t control_module = NULL;
	volatile uint32_t val;

	ThreadCtl(_NTO_TCTL_IO_PRIV , (void*)0);

	gpio1_base = mmap_device_io(AM335X_GPIO_SIZE, AM335X_GPIO1_BASE);
	control_module = mmap_device_io(AM335X_CONTROL_MODULE_SIZE, AM335X_CONTROL_MODULE_BASE);

	if(gpio1_base && control_module)

	{

	val = in32(gpio1_base + GPIO_OE);
	DEBUGF("Original gpio i/o register val = %#010x\n", val);


	if (inOut)
	{
		val |= pin;
		DEBUGF("pin set high\n");
	}
	else
	{
		val &= ~pin;
		DEBUGF("pin set low\n");
	}

	out32(gpio1_base + GPIO_OE, val); // write value to output enable for data pins
	DEBUGF("New gpio i/o register val = %#010x\n", val);


	in32s((void*)&val, 1, control_module + pinConfig);
	DEBUGF("Original pinmuxconfiguration for GPIO1_ = %#010x\n",val);

	// set up pin mux for the pins we are going to use (see page 1354 of TRM)
	volatile _CONF_MODULE_PIN pinConfigGPMC; // Pin configuration strut
	pinConfigGPMC.d32= 0;

	// Pin MUX register default setup for input (GPIOinput, disable pull up/down -Mode 7)
	pinConfigGPMC.b.conf_slewctrl= SLEW_SLOW; // Select between faster or slower slew rate
	pinConfigGPMC.b.conf_rxactive= RECV_ENABLE; // Input enable value for the PAD
	pinConfigGPMC.b.conf_putypesel= PU_PULL_UP; // Pad pullup/pulldowntype selection
	pinConfigGPMC.b.conf_puden= PU_ENABLE; // Pad pullup/pulldownenable
	pinConfigGPMC.b.conf_mmode= PIN_MODE_7; // Pad functional signal mux select 0 -7

	// Write to PinMuxregisters for the GPIO1_28
	out32(control_module + pinConfig, pinConfigGPMC.d32);
	in32s((void*)&val, 1, control_module + pinConfig); // Read it back

	DEBUGF("New configuration register for GPIO1_ = %#010x\n", val);

	munmap_device_io(control_module, AM335X_CONTROL_MODULE_SIZE);
	munmap_device_io(gpio1_base, AM335X_GPIO_SIZE);

	return true;
	}

	return false;
}



/* ----------------------------------------------------	*
 *	@writepin_gpio1:									*
 *	@breif:  											*
 *	@return:returns true or false for file				*
 * ---------------------------------------------------	*/
bool writepin_gpio1(uint32_t pin,  bool setTo)
{
	volatile uint32_t	tmp = 0;
	volatile uint32_t val;
	uintptr_t gpio1_base = 0;

	gpio1_base = mmap_device_io(AM335X_GPIO_SIZE, AM335X_GPIO1_BASE);

	if(gpio1_base)
	{

	val = in32(gpio1_base + GPIO_DATAOUT);

	if (setTo)
		val |= pin;
	else
		val &= ~pin;
	DEBUGF("New configuration register for GPIO1_ = %#010x\n", val);

	out32(gpio1_base + GPIO_DATAOUT, val); //

	munmap_device_io(gpio1_base, AM335X_GPIO_SIZE);
	}

	if (tmp != val)
		return false;

	return true;
}


/*-----------------------------------------------------------------------------
* Local Function Implementation
*---------------------------------------------------------------------------*/































/// EOF
