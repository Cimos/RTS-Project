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
typedef struct {


	int fd;

}I2C_HANDLE;


/*-----------------------------------------------------------------------------
* Local Variables and Buffers
*---------------------------------------------------------------------------*/


struct _self
{
	char *devName;
	I2C_HANDLE handle;

	UINT32 bus_speed;
	UINT8 channel;
	UINT8 slave_addr;
};

const char *devname[2] = { "/dev/i2c1"};
unsigned bus_speed[2] = {400000, 400000};


static float result_press, result_temp;

/*-----------------------------------------------------------------------------
* Threads Declarations
*---------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
* Local Function Declarations
*---------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
* Main Function
*---------------------------------------------------------------------------*/
int main(void)
{
	static _self self;
	puts("RTS - Traffic Light Project");
	return EXIT_SUCCESS;
}

/*-----------------------------------------------------------------------------
* Thread Definitions
*---------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------
* Local Function Definitions
*---------------------------------------------------------------------------*/
//     I2C_Open(&_ft800.i2c, I2C_PORT, I2C_FREQUENCY, 30, 1);

void I2C_Open(I2C_HANDLE *p, int I2C_PORT, int I2C_FREQUENCY, int k, int l)
{
	// Open I2C resource and set it up
	if ((file = open("/dev/i2c1",O_RDWR)) < 0)	  // OPEN I2C1
	{
		printf("Error while opening Device File.!!\n");
		exit(EXIT_FAILURE);
	}
	else
		//printf("I2C1 Opened Successfully\n");

	error = devctl(file,DCMD_I2C_SET_BUS_SPEED,&(speed),sizeof(speed),NULL);  // Set Bus speed
	if (error)
	{
		fprintf(stderr, "Error setting the bus speed: %d\n",strerror ( error ));
		exit(EXIT_FAILURE);
	}
	else
}
void I2C_Close(I2C_HANDLE *p);



int I2C_Write(I2C_HANDLE handle, UINT8 addr, UINT8* data, int size)
{
	i2c_send_t hdr;
	iov_t siov[2];

	hdr.slave.addr = addr;
	hdr.slave.fmt = I2C_ADDRFMT_7BIT;
	hdr.len = size;
	hdr.stop = 1;

	SETIOV(&siov[0], &hdr, sizeof(hdr));
	SETIOV(&siov[1], &data[0], size);

	return devctlv(handle.fd, DCMD_I2C_SEND, 2, 0, siov, NULL, NULL);
}





int I2C_Transaction(I2C_HANDLE handle, UINT8 addr, UINT8 *command, int size, UINT8 *retBuf, int size2)
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
    SETIOV(&siov[1], &command[0], size);

    SETIOV(&riov[0], &hdr, sizeof(hdr));	// setup riov
    SETIOV(&riov[1], retBuf, size2);

    // return success??
	return devctlv(handle.fd, DCMD_I2C_SENDRECV, 2, 2, siov, riov, NULL);
}


