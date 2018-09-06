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

#ifndef SRC_HARDWARE_SUPPORT_FT800_LCDTHREAD_H_
#define SRC_HARDWARE_SUPPORT_FT800_LCDTHREAD_H_


typedef struct {

}_self;

// ---------------------------------------------------------------------------
//							Global Function Definitions
// ---------------------------------------------------------------------------

// init i2c
int i2cInit(_self *self);

// init lcd graphics ft800 engine
int lcdInit(_self *self);













#endif /* SRC_HARDWARE_SUPPORT_FT800_LCDTHREAD_H_ */
