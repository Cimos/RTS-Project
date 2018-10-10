
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


#define AM335X_GPIO_SIZE	0x00001000
#define AM335X_GPIO1_BASE	0x4804C000
#define GPIO_OE        0x134 	// GPIO Output Enable Register
#define GPIO_DATAIN    0x138	// GPIO Data In Register
#define GPIO_DATAOUT   0x13C	// GPIO Data Out Register




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
 *	@getMsgQueId writeBoneLeds:						*
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


/*-----------------------------------------------------------------------------
* Local Function Implementation
*---------------------------------------------------------------------------*/































/// EOF
