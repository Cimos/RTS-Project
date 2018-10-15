/*
 * lcdThread.h
 *
 *  Created on: 16Aug.,2018
 *      Author: MadMan
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <fcntl.h>
#include <devctl.h>
#include <hw/i2c.h>
#include <errno.h>
#include <unistd.h>
#include <sys/neutrino.h>
#include <strings.h>

#include "../i2c/i2c_HAL.h"


#ifndef SRC_HARDWARE_SUPPORT_FT800_LCDTHREAD_H_
#define SRC_HARDWARE_SUPPORT_FT800_LCDTHREAD_H_


typedef struct {
	// ********* I2C
		I2C_HANDLE I2C_handle = {"/dev/i2c", 1, bus_speed::BAUD_100K};
		UINT8 slave_addr = 0x23;
}_self_i2c;

// ---------------------------------------------------------------------------
//							Global Function Definitions
// ---------------------------------------------------------------------------

// init i2c
int i2cInit(_self_i2c *self);

// init lcd graphics ft800 engine
int lcdInit(_self_i2c *self);


void Screen_animations(int i);

void lcd_2();


void splash_screen2();
void splash_screen();







#endif /* SRC_HARDWARE_SUPPORT_FT800_LCDTHREAD_H_ */
