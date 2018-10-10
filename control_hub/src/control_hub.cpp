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

//#include "threadTemplate.h"
//#include "lcdThread.h"
//#include "DelayTimer.h"
//#include "boneGpio.h"

#include "FT800.h"

#include "file_io.h"
#include "keyPad.h"
#include "debug.h"
#include <iostream>

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
#include <semaphore.h>
#include <time.h>

#include <sys/iofunc.h>
#include <sys/dispatch.h>



/*-----------------------------------------------------------------------------
* Definitions
*---------------------------------------------------------------------------*/
#define MAX_RUNTIME_BUFFER 21
#define Co_Ctrl   0x00  // mode to tell LCD we are sending a single command
#define DATA_SEND 0x40  // sets the Rs value high



// Use to lock mutex
#define Lock(_Mutex) {		 											\
	do { 																\
		pthread_mutex_lock(&_Mutex); 									\
	} while(0); 														\
}

// Use to unlock mutex
#define Unlock(_Mutex) { 												\
	do { 																\
		pthread_mutex_unlock(&_Mutex);									\
	} while(0); 														\
}

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



typedef struct
{
	struct _pulse hdr; // Our real data comes after this header
	int ClientID; // our data (unique id from client)
	controler2Intersection data;     // our data
} _data;

typedef struct
{
	struct _pulse hdr;  // Our real data comes after this header
    char buf[BUF_SIZE]; // Message we send back to clients to tell them the messages was processed correctly.
} _reply;


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



static struct
{
	int serverPID = 0;
	int serverCHID = 0;

	FILE *fp = NULL;
	char line[255] = {};
	char buf[MESSAGESIZE] = {};
	int chid = 0;
	int errorVal = 0;

	// ******** thread handlers
	_thread serverThread;

	// ********* I2C
	I2C_HANDLE I2C_handle = {"/dev/i2c", 1, bus_speed::BAUD_100K};
	UINT8 slave_addr;

	// ********* IntrSect's
	controler2Intersection intrSect_1;
	controler2Intersection intrSect_2;

	// ********* Keypad
	struct
	{
		keyPad thread;
		pthread_attr_t keyPad_attr;
		pthread_mutex_t Mtx = PTHREAD_MUTEX_INITIALIZER;
		struct sched_param keyPad_param;
		char UserInput = 0;
	}kp;


	// ******** Current Time
	time_t time;
	 struct tm *currentTime;
}self;



/*-----------------------------------------------------------------------------
* Local Function Declarations
*---------------------------------------------------------------------------*/
void *server(void *chid);
void threadInit(_thread *th);
void serverInit(void);
void init(void);
bool keypadInit(int prio);
void keypad_cb(char keyPress);
void logData(_data *toLog);

//int I2C_Open(I2C_HANDLE *handle, int port, UINT32 i2cFrequency, UINT8 notUsed1, UINT8 notUsed2);
//int I2C_Close(I2C_HANDLE *handle);
//int I2C_Write(I2C_HANDLE *handle, UINT8 addr, UINT8* data, int size);
//int I2C_Transaction(I2C_HANDLE *handle, UINT8 addr, UINT8 *sndBuf, int size, UINT8 *retBuf, int size2);
int  I2cWrite_(int fd, uint8_t Address, uint8_t mode, uint8_t *pBuffer, uint32_t NbData);





/*-----------------------------------------------------------------------------
* Main Function
*---------------------------------------------------------------------------*/
int main(void)		//TODO: set date and time
{
	init();
//	sem_t sem,*ptr_sema = &sem;
//
//	int sem_init( sem_t * sem,
//	              int pshared,
//	              unsigned value );

//	self.time = time(NULL);
//	self.currentTime = localtime(&self.time);

	_data tmp;

	tmp.ClientID = 1;
	tmp.data.currentState = trafficLightStates::EWYG;
	tmp.data.lightTiming.ewStright = 100;
	tmp.data.lightTiming.nsStright = 200;
	tmp.data.lightTiming.ewTurn = 101;
	tmp.data.lightTiming.nsTurn = 202;
	logData(&tmp);


	while(1)
	{
		usleep(500);
	}



	return EXIT_SUCCESS;
}




/*-----------------------------------------------------------------------------
* Local Function Definitions
*---------------------------------------------------------------------------*/



void init(void)
{
	std::string tmp(STARTUP_MSG);
	write_string_ToFile(&tmp, CHLOG, "a+");

	serverInit();
	keypadInit(5);
}

/* ----------------------------------------------------	*
 *	@server Implementation:								*
 *	@brief:												*
 *	@return:											*
 * ---------------------------------------------------	*/
void logData(_data *toLog)
{
	std::string toLogData;
	self.time = time(NULL);
	self.currentTime = localtime(&self.time);
	std::string time(asctime(self.currentTime));

	// creating cvs formate with time and date stamp
	toLogData.append(time.substr(0,time.length()-1));
	toLogData.append(":ClientID=");
	toLogData.append(std::to_string(toLog->ClientID));
	toLogData.append(",nsStright=");
	toLogData.append(std::to_string(toLog->data.lightTiming.nsStright));
	toLogData.append(",nsTurn=");
	toLogData.append(std::to_string(toLog->data.lightTiming.nsTurn));
	toLogData.append(",ewStright=");
	toLogData.append(std::to_string(toLog->data.lightTiming.ewStright));
	toLogData.append(",ewTurn=");
	toLogData.append(std::to_string(toLog->data.lightTiming.ewTurn));
	toLogData.append(",currentState=");
	toLogData.append(std::to_string(toLog->data.currentState));
	toLogData.append(";\n");
	write_string_ToFile(&toLogData, CHLOG, "a+");
}


/* ----------------------------------------------------	*
 *	@init_keypad Implementation:						*
 *	@brief:												*
 *	@return:											*
 * ---------------------------------------------------	*/
bool keypadInit(int prio)
{
	// Set up thread atributes so that a custom prio if software is too slow for pulse from keyboard.
    pthread_attr_init(&self.kp.keyPad_attr);
    pthread_attr_setschedpolicy(&self.kp.keyPad_attr, SCHED_RR);
    self.kp.keyPad_param.sched_priority = prio;
    pthread_attr_setschedparam (&self.kp.keyPad_attr, &self.kp.keyPad_param);
    pthread_attr_setinheritsched (&self.kp.keyPad_attr, PTHREAD_EXPLICIT_SCHED);
    pthread_attr_setstacksize (&self.kp.keyPad_attr, 8000);

	self.kp.thread.registerCallback(keypad_cb);
	// Note: can do kp.start() which will just give it default attributes and priority
	self.kp.thread.start(&self.kp.keyPad_attr);
}

/* ----------------------------------------------------	*
 *	@serverInit Implementation:							*
 *	@brief:												*
 *	@return:											*
 * ---------------------------------------------------	*/
void serverInit(void)
{
	DEBUGF("serverInit()->Initializing Server:\n");

	threadInit(&self.serverThread);

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

	write_pid_chid_ToFile(self.serverPID, self.serverCHID, CONTROLHUB_SERVER);

	DEBUGF("serverInit()->Server listening for clients:\n");


	pthread_create(&self.serverThread.thread, NULL, server, (void *)&self);

	//server(self.serverCHID);

	DEBUGF("serverInit()->Finished server Init:\n");
	return;
}


/* ----------------------------------------------------	*
 *	@threadInit Implementation:							*
 *	@brief:												*
 *	@return:											*
 * ---------------------------------------------------	*/
void threadInit(_thread *th)
{
    pthread_attr_init (&th->attr);
    pthread_attr_setschedpolicy(&th->attr, SCHED_RR);
    th->sch_prm.sched_priority = th->priority;
    pthread_attr_setschedparam (&th->attr, &th->sch_prm);
    pthread_attr_setinheritsched (&th->attr, PTHREAD_EXPLICIT_SCHED);
    pthread_attr_setstacksize (&th->attr, 8000);
}


//TODO: Fix this for appData
/* ----------------------------------------------------	*
 *	@server Implementation:								*
 *	@brief:												*
 *	@return:											*
 * ---------------------------------------------------	*/
void *server(void *appData)
{
//	self *tmp = (self *)appData;
	int rcvid=0, msgnum=0;  	// no message received yet
	int Stay_alive=0, living=1;	// stay alive and living for contorling the server status
    int server_coid = 0;
    int read = 0, chid = self.chid;
    _data msg;			// received msg
	_reply replymsg;	// replying msg

	replymsg.hdr.type = 0x01;
	replymsg.hdr.subtype = 0x00;


    while (living)
    {
        rcvid = MsgReceive(chid, &msg, sizeof(msg), NULL);
        DEBUGF("Message Received:\n");

        if (rcvid == -1) {
        	DEBUGF("Failed to MsgReceive\n");
            break;
        }
        if (rcvid == 0) {
            switch (msg.hdr.code){

                case _PULSE_CODE_DISCONNECT:
                    if(Stay_alive == 0)
                    {
                        ConnectDetach(msg.hdr.scoid);
                        DEBUGF("Server->was told to Detach from ClientID:%d ...\n", msg.ClientID);
                        //living = 0; // kill while loop
                        //continue;
                    }
                    else
                    {
                    	DEBUGF("Server->received Detach pulse from ClientID:%d but rejected it ...\n", msg.ClientID);
                    }
                    break;
                case _PULSE_CODE_UNBLOCK:
                	DEBUGF("Server->got _PULSE_CODE_UNBLOCK after %d, msgnum\n", msgnum);
                    break;
                case _PULSE_CODE_COIDDEATH:  // from the kernel
                	DEBUGF("Server->got _PULSE_CODE_COIDDEATH after %d, msgnum\n", msgnum);
                    break;
                case _PULSE_CODE_THREADDEATH: // from the kernel
                	DEBUGF("Server->got _PULSE_CODE_THREAD_DEATH after %d, msgnum\n", msgnum);
                    living = 0; // kill while loop
                    break;
                default: // Other pulse code, but it isnt handled
                	DEBUGF("Server->got some other pulse after %d, msgnum\n", msgnum);
                    break;
            }
            continue;
        }

        if(rcvid > 0)
        {
            msgnum++;

            if (msg.hdr.type == _IO_CONNECT )
            {
                MsgReply( rcvid, EOK, NULL, 0 );
                DEBUGF("gns service is running....\n");
                continue;	// go back to top of while loop
            }

            if (msg.hdr.type > _IO_BASE && msg.hdr.type <= _IO_MAX )
            {
                MsgError( rcvid, ENOSYS );
                DEBUGF("Server received and IO message and rejected it....\n");
                continue;	// go back to top of while loop
            }

            // if (msg.data == 1) { mode = 1; }
            // else { mode = 0; }
            DEBUGF("Server->logging received message");
        	logData(&msg);


        	// TODO: place this into a control function that does all the work
        	switch(msg.ClientID)
        	{
        	case clients::TRAFFIC_L1:
        		printf("Server->Do Something");
        		break;
        	case clients::TRAFFIC_L2:
        		break;
        	case clients::TRAIN_I1:
        		break;
        	default:
        		break;
        	}

            replymsg.buf[0] = read + '0';

            DEBUGF("Server->received data packet from client (ID:%d)\n", msg.ClientID);
            fflush(stdout);

            DEBUGF("Server->replying with: '%s'\n",replymsg.buf);
            MsgReply(rcvid, EOK, &replymsg, sizeof(replymsg));
        }
        else
        {
        	DEBUGF("Server->ERROR->received something, but could not handle it correctly\n");
        }

    }
	return NULL;
}

//MsgSend()

/* ----------------------------------------------------	*
 *	@keypad_cb Implementation:							*
 *	@brief:												*
 *	@return:											*
 * ---------------------------------------------------	*/
void keypad_cb(char keyPress)
{
	Lock(self.kp.Mtx);
	self.kp.UserInput = keyPress;
	Unlock(self.kp.Mtx);
 	DEBUGF("Key Pressed: %c", keyPress);
}




















// Writes to I2C
int  I2cWrite_(int fd, uint8_t Address, uint8_t mode, uint8_t *pBuffer, uint32_t NbData)
{
	i2c_send_t hdr;
    iov_t sv[2];
    int status, i;

    uint8_t LCDpacket[21] = {};  // limited to 21 characters  (1 control bit + 20 bytes)

    // set the mode for the write (control or data)
    LCDpacket[0] = mode;  // set the mode (data or control)

	// copy data to send to send buffer (after the mode bit)
	for (i=0;i<NbData+1;i++)
		LCDpacket[i+1] = *pBuffer++;

    hdr.slave.addr = Address;
    hdr.slave.fmt = I2C_ADDRFMT_7BIT;
    hdr.len = NbData + 1;  // 1 extra for control (mode) bit
    hdr.stop = 1;

    SETIOV(&sv[0], &hdr, sizeof(hdr));
    SETIOV(&sv[1], &LCDpacket[0], NbData + 1); // 1 extra for control (mode) bit
      // int devctlv(int filedes, int dcmd,     int sparts, int rparts, const iov_t *sv, const iov_t *rv, int *dev_info_ptr);
    status = devctlv(fd, 		  DCMD_I2C_SEND, 2,          0,          sv,              NULL,           NULL);

    if (status != EOK)
    	printf("status = %s\n", strerror ( status ));

    return status;
}




/*
 *
//	UINT8 *tmp = (UINT8*)"Hello World";
//	error = I2C_Open(&self.I2C_handle, 1, 20000, 0, 0);
//	printf("Error %d\n", error);
//	// 0x23 0x7C
//	while (1)
//	{
//	error = I2C_Write(&self.I2C_handle, 0x23<<1, tmp, sizeof("Hello World"));
//	printf("Error %d\n", error);
//	sleep(1);
//	}
//
//	error = I2C_Close(&self.I2C_handle);
//	printf("Error %d\n", error);


	//serverInit();
 *
 *
 */





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

	error = devctl(handle->fd, DCMD_I2C_SET_BUS_SPEED, &(i2cFrequency), sizeof(i2cFrequency), NULL);
	fprintf(stderr, "Error setting the I2C bus speed: %s\n",strerror ( error ));

	return error;
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

	fprintf(stderr, "Error sending i2c msg: %s\n",strerror ( error ));

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

	return 0;
}



/*
 *
 * int file;
	volatile uint8_t LCDi2cAdd = 0x3C;	// i2c address
	uint8_t	LCDcontrol = 0x00;


	if ((file = open("/dev/i2c1",O_RDWR)) < 0)	  // OPEN I2C1
	{
		printf("Error while opening Device File.!!\n");
		exit(EXIT_FAILURE);
	}



	_Uint32t speed = 10000; // nice and slow (will work with 200000)

	error = devctl(file,DCMD_I2C_SET_BUS_SPEED,&(speed),sizeof(speed),NULL);  // Set Bus speed

	LCDcontrol = 0x38;  // data byte for FUNC_SET_TBL1


	while(1)
	{
		I2cWrite_(file, LCDi2cAdd, Co_Ctrl, &LCDcontrol, 1);		// write data to I2C

		usleep(500);
	}
 *
 *
 */


