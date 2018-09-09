/******************************************************************************
*
* RTS Traffic Light Project 2018
*
*****************************************************************************/
/**
* @file     projectTrafficLight.c
* @brief    RTS Traffic Light Project
* @author   Michael Stekla	s3545953
* 			Simon Maddison	s3493550
* 			Shawn Buschmann	s3478646
*/

/*-----------------------------------------------------------------------------
* Included Files
*---------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>

#include "FT800.h"

#include <string.h>

//#include "lcdThread.h"
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
/*-----------------------------------------------------------------------------
* Definitions
*---------------------------------------------------------------------------*/

#define MAX_RUNTIME_BUFFER 21

/*-----------------------------------------------------------------------------
* Local Variables and Buffers
*---------------------------------------------------------------------------*/
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


struct _self
{
	I2C_HANDLE I2C_handle = {"/dev/i2c", 0, bus_speed::BAUD_100K};
	UINT8 slave_addr;
};




/*-----------------------------------------------------------------------------
* Threads Declarations
*---------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
* Local Function Declarations
*---------------------------------------------------------------------------*/

int I2C_Open(I2C_HANDLE *handle, int port, UINT32 i2cFrequency, UINT8 notUsed1, UINT8 notUsed2);
int I2C_Close(I2C_HANDLE *handle);
int I2C_Write(I2C_HANDLE *handle, UINT8 addr, UINT8* data, int size);
int I2C_Transaction(I2C_HANDLE *handle, UINT8 addr, UINT8 *sndBuf, int size, UINT8 *retBuf, int size2);

/*-----------------------------------------------------------------------------
* Main Function
*---------------------------------------------------------------------------*/
int main(void)
{
	static _self self;

	int error = 0;

	UINT8 *tmp = (UINT8*)"Hello World";
	error = I2C_Open(&self.I2C_handle, 0, bus_speed::BAUD_100K, 0, 0);

	error = I2C_Write(&self.I2C_handle, 0x12, tmp, sizeof("Hello World"));

	error = I2C_Close(&self.I2C_handle);





	puts("RTS - Traffic Light Project");
	return EXIT_SUCCESS;
}

/*-----------------------------------------------------------------------------
* Thread Definitions
*---------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------
* Local Function Definitions
*---------------------------------------------------------------------------*/


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

	handle->devName[8] = ('0' + port);
	handle->bus_speed = i2cFrequency;

	if ((handle->fd = open("/dev/i2c1", O_RDWR)) < 0)
	{ return -1; }

	return devctl(handle->fd, DCMD_I2C_SET_BUS_SPEED, &(i2cFrequency), sizeof(i2cFrequency), NULL);
	//fprintf(stderr, "Error setting the I2C bus speed: %s\n",strerror ( error ));


	//error = devctl(handle->fd,DCMD_I2C_SET_SLAVE_ADDR,&address,sizeof(address),NULL);

	//fprintf(stderr, "Error setting the slave address: %s\n",strerror ( error ));

	//return error;
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
int I2C_Write(I2C_HANDLE *handle, UINT8 addr, UINT8* data, int size)
{
	i2c_send_t hdr;
	iov_t siov[2];

	hdr.slave.addr = addr;
	hdr.slave.fmt = I2C_ADDRFMT_7BIT;
	hdr.len = size;
	hdr.stop = 1;

	SETIOV(&siov[0], &hdr, sizeof(hdr));
	SETIOV(&siov[1], &data[0], size);


	int error = devctlv(handle->fd, DCMD_I2C_SEND, 2, 0, siov, NULL, NULL);

	fprintf(stderr, "Error sendding i2c msg: %s\n",strerror ( error ));

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
	i2c_sendrecv_t  hdr;
	iov_t siov[2] = {};
	iov_t riov[2] = {};

    hdr.slave.addr = addr;
    hdr.slave.fmt = I2C_ADDRFMT_7BIT;
    hdr.send_len = size;
    hdr.recv_len = size2;
    hdr.stop = 1;

    SETIOV(&siov[0], &hdr, sizeof(hdr));	// setup siov
    SETIOV(&siov[1], &sndBuf[0], size);

    SETIOV(&riov[0], &hdr, sizeof(hdr));	// setup riov
    SETIOV(&riov[1], retBuf, size2);

    // return success??
	return devctlv(handle->fd, DCMD_I2C_SENDRECV, 2, 2, siov, riov, NULL);
}


