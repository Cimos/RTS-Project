/*
 * i2c_HAL.c
 *
 *  Created on: 20Aug.,2018
 *  	Author: MadMan
 *
 *  BBB P9 connections:
 *    - P9_Pin17 - SCL - I2C1	 GPIO3_2
 *    - P9_Pin18 - SDA - I2C1    GPIO3_1
 *
 *
 */

#include "i2c_HAL.h"
#include <strings.h>
#include <devctl.h>
#include <hw/i2c.h>
#include <fcntl.h>
#include "../FT800/FT800.h"
#include <stdlib.h>
#include <stdio.h>

extern int errno;


//#define DEBUGF printf("DEBUG->"); print
#define DEBUGF //

#define ACTIVE  0x00
#define STANDBY 0x41
#define SLEEP   0x42
#define PWRDOWN 0x50
#define CLKEXT  0x44
#define CLK48M  0x62
#define CLK36M  0x61
#define CORERST 0x68

#define I2C_PORT            0
#define I2C_ADDRESS         0x22
#define I2C_FREQUENCY       400000    //100k hz //400k needed?


extern unsigned short dli;
extern unsigned short cmd_offset;

/*-----------------------------------------------------------------------------
 * Global Variables Declarations
 *---------------------------------------------------------------------------*/
typedef struct
{
	int pin;
}GPIO_HANDLE;


static struct {
  GPIO_HANDLE reset;
  I2C_HANDLE i2c;
  ft_uint8_t reg_id;
  UINT8 addr;
} _ft800;



/*-----------------------------------------------------------------------------
 * Global Function Declarations
 *---------------------------------------------------------------------------*/

int I2C_Open(I2C_HANDLE *handle, int port, UINT32 i2cFrequency, UINT8 notUsed1, UINT8 notUsed2);
int I2C_Close(I2C_HANDLE *handle);
int I2C_Write(I2C_HANDLE *handle, UINT8 addr, int size, UINT8 *command);
int I2C_Transaction(I2C_HANDLE *handle, UINT8 addr, UINT8 *sndBuf, int size, UINT8 *retBuf, int size2);



/*-----------------------------------------------------------------------------
 * Global Function Implementation
 *---------------------------------------------------------------------------*/




// --------------------------------------- READ/WRITE ---------------------------------------

void host_command(UINT8 command)
{
    UINT8 data[3] = {};

    data[0] = 0x40 | (command & 0x3F);      // Command[5:0]
    data[1] = 0x00;                         // [15:8]
    data[2] = 0x00;                         // [7:0]

    I2C_Write(&_ft800.i2c, _ft800.addr, 3, &command);
}

void wr8(UINT32 addr, ft_uint8_t value)
{
   // int ret;
    UINT8 data[4] = {};

    data[0] = 0x80 | ((addr>>16) & 0x3F);     // Upper two bits forces to '80'
    data[1] = addr>>8 & 0xFF;
    data[2] = addr & 0xFF;
    data[3] = value;

    I2C_Write(&_ft800.i2c, _ft800.addr, sizeof(data), data);
}

void wr16(UINT32 addr, ft_uint16_t value)
{
    //int ret;
    UINT8 data[5] = {};

    data[0] = 0x80 | ((addr>>16) & 0x3F);
    data[1] = addr>>8 & 0xFF;
    data[2] = addr & 0xFF;
    data[3] = value & 0xFF;
    data[4] = value>>8 & 0xFF;

    I2C_Write(&_ft800.i2c, _ft800.addr, 5, data);
}

void wr32(UINT32 addr, ft_uint32_t value)
{
    //int ret;
    UINT8 data[7] = {};

    data[0] = 0x80 | ((addr>>16) & 0x3F);
    data[1] = addr>>8 & 0xFF;
    data[2] = addr & 0xFF;
    data[3] = value & 0xFF;
    data[4] = value>>8 & 0xFF;
    data[5] = value>>16 & 0xFF;
    data[6] = value>>24 & 0xFF;

    I2C_Write(&_ft800.i2c, _ft800.addr, 7, data);
}

ft_uint8_t rd8(UINT32 addr)
{
    //int ret;
    UINT8 data[3] = {};

    data[0] = 0x00 | ((addr>>16) & 0x3F);   // Upper two bits forces to '00'
    data[1] = addr>>8 & 0xFF;
    data[2] = addr & 0xFF;

    // ret is error code and should be check
    // step 1 -- 10
    I2C_Transaction(&_ft800.i2c, _ft800.addr,
                            data,3,     //tx
                            data,1);   //rx

    return (ft_uint8_t)data[0];
}

ft_uint16_t rd16(UINT32 addr)
{
    //int ret;
    UINT8 data[3] = {};

    data[0] = 0x00 | ((addr>>16) & 0x3F);
    data[1] = addr>>8 & 0xFF;
    data[2] = addr & 0xFF;

    I2C_Transaction(&_ft800.i2c, _ft800.addr,
                            data,3,
                            data,2);

    return (ft_uint16_t)((data[0] << 8) | (data[1]));
}

ft_uint32_t rd32(UINT32 addr)
{
    //int ret;
    UINT8 data[4] = {};

    data[0] = 0x00 | ((addr>>16) & 0x3F);
    data[1] = addr>>8 & 0xFF;
    data[2] = addr & 0xFF;

    I2C_Transaction(&_ft800.i2c, _ft800.addr,
                            data,3,
                            data,4);

    return (ft_uint32_t)((data[0] << 24) | (data[1] << 16)
                       | (data[2] << 8)  | (data[3]));
}

ft_uint8_t wr8s(UINT32 addr, ft_char8_t *s)   // max 20 bytes of data excluding addr
{
    int length = strlen(s);
    int allignment = length%4;
    int i = 0;
    UINT8 *data = NULL;


	if ((data = (UINT8 *)malloc((length+3) + allignment)) == NULL)
	{
		return i;
	}

    data[0] = 0x80 + ((addr>>16) & 0x3F);
    data[1] = addr>>8 & 0xFF;
    data[2] = addr & 0xFF;

    for (i = 0; i<length ; i++)
    {
      data[i+3] = s[i];
    }
    for (; i%4;i++)
    {
      data[i+3] = 0x00;
    }

    I2C_Write(&_ft800.i2c, _ft800.addr, (length + 3 + allignment), data);

    free(data);
    return i;
}

unsigned int incCMDOffset(unsigned int currentOffset, unsigned char commandSize)
{
    unsigned int newOffset;			// used to hold new offset
    newOffset = currentOffset + commandSize;	// Calculate new offset
    if(newOffset > 4095)			// If new offset past boundary...
    {
        newOffset = (newOffset - 4096);		// ... roll over pointer
    }
    return newOffset;				// Return new offset
}


void cmd_incrementn(unsigned char n)
{
    cmd_offset=(cmd_offset+n)%4096;
}

void dl_cmd(unsigned long cmd)
{
  wr32(RAM_DL + dli, cmd);
  dli += 4;
}


void cmd(ft_uint32_t command){
    wr32(RAM_CMD + cmd_offset, command);
    cmd_increment4();
}

void i2c_LCD_port_OPEN()
{
    I2C_Open(&_ft800.i2c, I2C_PORT, I2C_FREQUENCY, 30, 1);
	_ft800.addr = I2C_ADDRESS;
}

void i2c_LCD_port_CLOSE()
{
    I2C_Close(&_ft800.i2c);
}








/*
 * @breif: use to open and init i2c port
 * @ret: returns error code
 *
 */
int I2C_Open(I2C_HANDLE *handle, int port, UINT32 i2cFrequency, UINT8 notUsed1, UINT8 notUsed2)
{
	int error = 0;
	//_Uint32t speed = 10000; // nice and slow (will work with 200000)
	//i2c_addr_t address;
	//address.fmt = I2C_ADDRFMT_7BIT;
	//address.addr = 0x90;

	if (port > 1 || port < 0)
	{ return -1; }

	//TODO: get this in a char array
	handle->devName[8] = ('0' + port);
	handle->bus_speed = i2cFrequency;

	if ((handle->fd = open("/dev/i2c1", O_RDWR)) < 0)
	{
		error = errno;
		DEBUGF("i2c_HAL->The error generated was %d\n", error );
		DEBUGF("i2c_HAL->That means: %s\n", strerror( error ) );
	    fflush(stdout);
		return -1;
	}

	error = devctl(handle->fd, DCMD_I2C_SET_BUS_SPEED, &(handle->bus_speed), sizeof(handle->bus_speed), NULL);
	DEBUGF("i2c_HAL->Error setting the I2C bus speed: %s\n",strerror ( error ));



	return error;
}

/*
 * @breif: use to close i2c port
 * @ret: returns error code
 *
 */
int I2C_Close(I2C_HANDLE *handle)
{
	return close(handle->fd);
}

/*
 * @breif: use to write data to a i2c salve
 * @ret: returns error code
 *
 * @note: addr is not address of slave but address of register at slave
 *
 */
int I2C_Write(I2C_HANDLE *handle, UINT8 addr, int size, UINT8 *command)
{
	i2c_send_t hdr;
	iov_t siov[2];

	hdr.slave.addr = addr;
	hdr.slave.fmt = I2C_ADDRFMT_7BIT;
	hdr.len = size;
	hdr.stop = 1;

	SETIOV(&siov[0], &hdr, sizeof(hdr));
	SETIOV(&siov[1], &command[0], size);


	int error = devctlv(handle->fd, DCMD_I2C_SEND, 2, 0, siov, NULL, NULL);

	DEBUGF("i2c_HAL->Error sending i2c msg: %s\n",strerror ( error ));

	return error;
}

/*
 * @breif: use to write then read a i2c salve.. i.e. reading a register from the slave
 * @ret: returns error code
 *
 * @note: addr is not address of slave but address of register at slave
 *
 */

int I2C_Transaction(I2C_HANDLE *handle, UINT8 addr, UINT8 *sndBuf, int size, UINT8 *retBuf, int size2)
{
	int error = 0;
//	i2c_sendrecv_t  hdr;
	i2c_send_t  hdr_send;
	i2c_recv_t  hdr_recv;
	iov_t siov[2] = {};
	iov_t riov[2] = {};

//	hdr.slave.addr = addr;
//	hdr.slave.fmt = I2C_ADDRFMT_7BIT;
//	hdr.send_len = size;
//	hdr.recv_len = size2;
//	hdr.stop = 1;
//
//    SETIOV(&siov[0], &hdr, sizeof(hdr));	// setup siov
//    SETIOV(&siov[1], &sndBuf[0], size);
//
//    SETIOV(&riov[0], &hdr, sizeof(hdr));	// setup riov
//    SETIOV(&riov[1], retBuf, size2);



	hdr_send.slave.addr = addr;
	hdr_send.slave.fmt = I2C_ADDRFMT_7BIT;
	hdr_send.len = size;
	hdr_send.stop = 1;
	hdr_recv.slave.addr = addr;
	hdr_recv.slave.fmt = I2C_ADDRFMT_7BIT;
	hdr_recv.len = size2;
	hdr_recv.stop = 1;

    SETIOV(&siov[0], &hdr_send, sizeof(hdr_send));	// setup siov
    SETIOV(&siov[1], &sndBuf[0], size);
    SETIOV(&riov[0], &hdr_recv, sizeof(hdr_recv));	// setup riov
    SETIOV(&riov[1], retBuf, size2);


	//devctlv(handle->fd, DCMD_I2C_SENDRECV, 2, 2, siov, riov, NULL);
	devctlv(handle->fd, DCMD_I2C_SEND, 2, 0, siov, NULL, NULL);

	error = errno;
	DEBUGF("i2c_HAL->Error with i2c: %s\n", strerror ( error ));


	devctlv(handle->fd, DCMD_I2C_RECV, 2, 0, riov, NULL, NULL);
	error = errno;
	DEBUGF("i2c_HAL->Error with i2c: %s\n", strerror ( error ));

	return 0;
}



