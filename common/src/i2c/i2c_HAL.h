/*
 * i2c_HAL.h
 *
 *  Created on: 20Aug.,2018
 *      Author: MadMan
 */

#ifndef SRC_I2C_I2C_HAL_H_
#define SRC_I2C_I2C_HAL_H_

#include "../FT800/FT800.h"

//#define NULL 0

//void host_command(UINT8 command);
//void wr8(UINT32 addr, ft_uint8_t value);
//void wr16(UINT32 addr, ft_uint16_t value);
//void wr32(UINT32 addr, ft_uint32_t value);
//ft_uint8_t rd8(UINT32 addr);
//ft_uint16_t rd16(UINT32 addr);/
//ft_uint32_t rd32(UINT32 addr);
//ft_uint8_t wr8s(UINT32 addr, ft_char8_t *s);   // max 20 bytes of data excluding addr
//unsigned int incCMDOffset(unsigned int currentOffset, unsigned char commandSize);
//void cmd_incrementn(unsigned char n);
//void dl_cmd(unsigned long cmd);
//void cmd(ft_uint32_t command);



enum bus_speed
{
	BAUD_100K = 100000,
	BAUD_400K = 400000,
	BAUD_3_2M = 3200000
};

typedef struct {
	char devName[10];
	int fd;
	UINT32 bus_speed;
}I2C_HANDLE;



void i2c_LCD_port_OPEN();
void i2c_LCD_port_CLOSE();







#endif /* SRC_I2C_I2C_HAL_H_ */
