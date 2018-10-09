/******************************************************************************
*
* RTS Traffic Light Project 2018
*
*****************************************************************************/
/**
* @file     projectTrafficLight.cpp
* @brief    RTS Traffic Light Project
* @author   Michael Stekla	s3545953
* 			Simon Maddison	s3493550
* 			Shawn Buschmann	s3478646
*/

/*-----------------------------------------------------------------------------
* Included Files
*---------------------------------------------------------------------------*/

#include "control_hub.h"

#include <stdio.h>
#include <stdlib.h>

#ifdef lcdDebug
//#include "lcdThread.h"
#include "FT800.h"
#endif

#include "file_io.h"
#include "keyPad.h"
#include "debug.h"
//#include "boneGpio.h"



#include <string.h>
#include <stdint.h>
#include <pthread.h>
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
#define SERVERINFO "TODO_CHANGE_ME"


/*-----------------------------------------------------------------------------
* Local Variables and Buffers
*---------------------------------------------------------------------------*/

typedef struct
{
	pthread_t thread;
	pthread_attr_t attr;
	struct sched_param sch_prm;
	int priority;
}_thread;



static struct
{
	int serverPID = 0;
	int serverCHID = 0;
	
	FILE *fp = NULL;
	char line[255] = {};
	char buf[MESSAGESIZE] = {};
	int chid = 0;
	int errorVal = 0;
	
	//mqd_t qd;

	_thread serverThread;

}self;




#ifdef lcdDebug
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
#endif

/*-----------------------------------------------------------------------------
* Threads Declarations
*---------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
* Local Function Declarations
*---------------------------------------------------------------------------*/
// Server Host
void *server(void *chid);
void tmp(void);


#ifdef lcdDebug

int I2C_Open(I2C_HANDLE *handle, int port, UINT32 i2cFrequency, UINT8 notUsed1, UINT8 notUsed2);
int I2C_Close(I2C_HANDLE *handle);
int I2C_Write(I2C_HANDLE *handle, UINT8 addr, UINT8* data, int size);
int I2C_Transaction(I2C_HANDLE *handle, UINT8 addr, UINT8 *sndBuf, int size, UINT8 *retBuf, int size2);
#endif



void keypad_cb(char keypress);
/*-----------------------------------------------------------------------------
* Main Function
*---------------------------------------------------------------------------*/
int main(void)
{
	//static _self self;

	int error = 0;

	keyPad kp;
	kp.registerCallback(keypad_cb);
	kp.start();

	puts("KeyPad Started");

	while(1)
	{
		sleep(100);
	}
	///checkIfFileExists("Amp");
	//writeBoneLeds();



#ifdef lcdDebug
	UINT8 *tmp = (UINT8*)"Hello World";
	error = I2C_Open(&self.I2C_handle, 0, bus_speed::BAUD_100K, 0, 0);
	error = I2C_Write(&self.I2C_handle, 0x12, tmp, sizeof("Hello World"));
	error = I2C_Close(&self.I2C_handle);
#endif



	puts("RTS - Traffic Light Project");
	return EXIT_SUCCESS;
}


/*-----------------------------------------------------------------------------
* Local Function Definitions
*---------------------------------------------------------------------------*/


void keypad_cb(char keypress)
{
	puts("Key Pressed");

}








































void threadInit(_thread *th)
{
    pthread_attr_init (&th->attr);
    pthread_attr_setschedpolicy(&th->attr, SCHED_RR);
    th->sch_prm.sched_priority = th->priority;
    pthread_attr_setschedparam (&th->attr, &th->sch_prm);
    pthread_attr_setinheritsched (&th->attr, PTHREAD_EXPLICIT_SCHED);
    pthread_attr_setstacksize (&th->attr, 8000);
}


void serverInit(void)
{
	DEBUGF("serverInit()->Initializing Server:\n");

	self.serverPID = getpid(); 		// get server process ID
	self.serverCHID = ChannelCreate(_NTO_CHF_DISCONNECT); // Create Channel

	if (self.serverCHID == -1)  // _NTO_CHF_DISCONNECT flag used to allow detach
	{
		DEBUGF("serverInit()->Failed to create communication channel on server\n");
		return;
	}

	DEBUGF("serverInit()->Writing server details to a file:");
	DEBUGF("serverInit()->Process ID   : %d \n", self.serverPID);
	DEBUGF("serverInit()->Channel ID   : %d \n", self.serverCHID);

	write_pid_chid_ToFile(self.serverPID, self.serverCHID, SERVERINFO);

	DEBUGF("serverInit()->Server listening for clients:\n");
	
	
	pthread_create(&self.serverThread.thread, NULL, server, NULL);

	//server(self.serverCHID);

	DEBUGF("serverInit()->Finished server Init:\n");
	return;
}


//TODO: Fix this for appData

/* ----------------------------------------------------	*
 *	@server Implementation:								*
 *	@brief:												*
 *	@return:											*
 * ---------------------------------------------------	*/
void *server(void *chid)
{
//	int rcvid=0, msgnum=0;  	// no message received yet
//	int Stay_alive=0, living=1;	// server stays running (ignores _PULSE_CODE_DISCONNECT request)
//    int server_coid = 0, mode = 0;
//    int read;
//	my_data msg;
//	my_reply replymsg; 			// replymsg structure for sending back to client
//	replymsg.hdr.type = 0x01;
//	replymsg.hdr.subtype = 0x00;
//
//
//    while (living)
//    {
//        // Do your MsgReceive's here now with the chid
//        rcvid = MsgReceive(chid, &msg, sizeof(msg), NULL);
//        DEBUGF("Message Received:\n");
//
//        if (rcvid == -1) {
//        	DEBUGF("Failed to MsgReceive\n");
//            break;
//        }
//        if (rcvid == 0) {
//            switch (msg.hdr.code){
//
//                case _PULSE_CODE_DISCONNECT:
//                    if( Stay_alive == 0) {
//                        ConnectDetach(msg.hdr.scoid);
//                        DEBUGF("Server was told to Detach from ClientID:%d ...\n", msg.ClientID);
//                        living = 0; // kill while loop
//                        continue;
//                    } else {
//                    	DEBUGF("Server received Detach pulse from ClientID:%d but rejected it ...\n", msg.ClientID);
//                    }
//                    break;
//
//                case _PULSE_CODE_UNBLOCK:
//                	DEBUGF("Server got _PULSE_CODE_UNBLOCK after %d, msgnum\n", msgnum);
//                    break;
//
//                case _PULSE_CODE_COIDDEATH:  // from the kernel
//                	DEBUGF("Server got _PULSE_CODE_COIDDEATH after %d, msgnum\n", msgnum);
//                    break;
//
//                case _PULSE_CODE_THREADDEATH: // from the kernel
//                	DEBUGF("Server got _PULSE_CODE_THREADDEATH after %d, msgnum\n", msgnum);
//                    break;
//
//                default: // Some other pulse sent by one of your processes or the kernel
//                	DEBUGF("Server got some other pulse after %d, msgnum\n", msgnum);
//                    break;
//
//            }
//            continue;
//        }
//
//        if(rcvid > 0) {
//            msgnum++;
//
//            if (msg.hdr.type == _IO_CONNECT ) {
//                MsgReply( rcvid, EOK, NULL, 0 );
//                DEBUGF("gns service is running....\n");
//                continue;	// go back to top of while loop
//            }
//
//            if (msg.hdr.type > _IO_BASE && msg.hdr.type <= _IO_MAX ) {
//                MsgError( rcvid, ENOSYS );
//                DEBUGF("Server received and IO message and rejected it....\n");
//                continue;	// go back to top of while loop
//            }
//
//            if (msg.data == 1) { mode = 1; }
//            else { mode = 0; }
//
//            read = readTrafficLightSensor(mode);
//            replymsg.buf[0] = read + '0';
//
//            DEBUGF("Server received data packet with value of '%d' from client (ID:%d)\n", msg.data, msg.ClientID);
//            fflush(stdout);
//
//            DEBUGF("replying with: '%s'\n",replymsg.buf);
//            MsgReply(rcvid, EOK, &replymsg, sizeof(replymsg));
//        }
//        else
//        {
//        	DEBUGF("ERROR: Server received something, but could not handle it correctly\n");
//        }
//
//    }
    //return 0;
	return NULL;
}































//
///*
// * @breif: use to open and init i2c port
// * @ret: returns error code
// *
// */
//int I2C_Open(I2C_HANDLE *handle, int port, UINT32 i2cFrequency, UINT8 notUsed1, UINT8 notUsed2)
//{
//	int error = 0;
//	//_Uint32t speed = 10000; // nice and slow (will work with 200000)
//	//i2c_addr_t address;
//	//address.fmt = I2C_ADDRFMT_7BIT;
//	//address.addr = 0x90;
//
//	if (port > 1 || port < 0)
//	{ return -1; }
//
//	handle->devName[8] = ('0' + port);
//	handle->bus_speed = i2cFrequency;
//
//	if ((handle->fd = open("/dev/i2c1", O_RDWR)) < 0)
//	{ return -1; }
//
//	return devctl(handle->fd, DCMD_I2C_SET_BUS_SPEED, &(i2cFrequency), sizeof(i2cFrequency), NULL);
//	//fprintf(stderr, "Error setting the I2C bus speed: %s\n",strerror ( error ));
//
//
//	//error = devctl(handle->fd,DCMD_I2C_SET_SLAVE_ADDR,&address,sizeof(address),NULL);
//
//	//fprintf(stderr, "Error setting the slave address: %s\n",strerror ( error ));
//
//	//return error;
//}
//
///*
// * @breif: use to close i2c port
// * @ret: returns error code
// *
// */
//int I2C_Close(I2C_HANDLE *handle)
//{
//	return close(handle->fd);
//}
//
///*
// * @breif: use to write data to a i2c salve
// * @ret: returns error code
// *
// * @note: addr is not address of slave but address of register at slave
// *
// */
//int I2C_Write(I2C_HANDLE *handle, UINT8 addr, UINT8* data, int size)
//{
//	i2c_send_t hdr;
//	iov_t siov[2];
//
//	hdr.slave.addr = addr;
//	hdr.slave.fmt = I2C_ADDRFMT_7BIT;
//	hdr.len = size;
//	hdr.stop = 1;
//
//	SETIOV(&siov[0], &hdr, sizeof(hdr));
//	SETIOV(&siov[1], &data[0], size);
//
//
//	int error = devctlv(handle->fd, DCMD_I2C_SEND, 2, 0, siov, NULL, NULL);
//
//	fprintf(stderr, "Error sendding i2c msg: %s\n",strerror ( error ));
//
//	return 0;
//}
//
///*
// * @breif: use to write then read a i2c salve.. i.e. reading a register from the slave
// * @ret: returns error code
// *
// * @note: addr is not address of slave but address of register at slave
// *
// */
//int I2C_Transaction(I2C_HANDLE *handle, UINT8 addr, UINT8 *sndBuf, int size, UINT8 *retBuf, int size2)
//{
//	i2c_sendrecv_t  hdr;
//	iov_t siov[2] = {};
//	iov_t riov[2] = {};
//
//    hdr.slave.addr = addr;
//    hdr.slave.fmt = I2C_ADDRFMT_7BIT;
//    hdr.send_len = size;
//    hdr.recv_len = size2;
//    hdr.stop = 1;
//
//    SETIOV(&siov[0], &hdr, sizeof(hdr));	// setup siov
//    SETIOV(&siov[1], &sndBuf[0], size);
//
//    SETIOV(&riov[0], &hdr, sizeof(hdr));	// setup riov
//    SETIOV(&riov[1], retBuf, size2);
//
//    // return success??
//	return devctlv(handle->fd, DCMD_I2C_SENDRECV, 2, 2, siov, riov, NULL);
//
//	return 0;
//}
//

