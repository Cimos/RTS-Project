
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


/*-----------------------------------------------------------------------------
* Definitions
*---------------------------------------------------------------------------*/



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
bool writeBoneLeds(volatile uint32_t val)
{
	uintptr_t gpio1_base = 0;

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
	val &= ~(LED0|LED1|LED2|LED3);
	out32(gpio1_base + GPIO_OE, val);

	// confirm that GPIO_OE is set to output
	tmp  = in32(gpio1_base + GPIO_OE);
	DEBUGF("New value of GPIO_1 output enable register: %#010x\n", val);

	if (tmp != val)
		return false;

	// TODO: is this needed?
	munmap_device_io(gpio1_base, AM335X_GPIO_SIZE);

	DEBUGF("Main Terminated...!\n");
	return true;
}




/* ----------------------------------------------------	*
 *	@writepin:											*
 *	@breif:  											*
 *	@return:returns true or false for file				*
 * ---------------------------------------------------	*/
bool writepin(uint32_t pin, uint32_t pinConfig, bool setTo)
{
	volatile uint32_t	tmp = 0;
	volatile uint32_t val;
	uintptr_t gpio1_base = 0;
	uintptr_t control_module = NULL;

	gpio1_base = mmap_device_io(AM335X_GPIO_SIZE, AM335X_GPIO1_BASE);
	control_module = mmap_device_io(AM335X_CONTROL_MODULE_SIZE, AM335X_CONTROL_MODULE_BASE);

	if( (control_module)&&(gpio1_base) )
	{
	volatile uint32_t val = 0;
	in32s((void*)&val, 1, control_module + pinConfig );
	DEBUGF("Current pinmux configuration for GPIO1_28 = %#010x\n", val);
	munmap_device_io(control_module, AM335X_CONTROL_MODULE_SIZE);

	DEBUGF("I/O Port Mapping is Successful!\n");

	val = in32(gpio1_base + GPIO_OE);
	val &= ~pin;                      // 0 for output
	out32(gpio1_base + GPIO_OE, val); // write value to output enable for data pins

	val = in32(gpio1_base + GPIO_DATAOUT);

	if (setTo)
		val |= pin;
	else
		val &= ~pin;

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
