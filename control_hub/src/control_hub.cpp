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
#include "lcdThread.h"

//#include "threadTemplate.h"
//#include "lcdThread.h"
//#include "DelayTimer.h"

#include "boneGpio.h"
#include <FT800.h>
#include "file_io.h"
#include "keyPad.h"
#include "debug.h"
#include "threadTemplate.h"


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
#include <sys/netmgr.h>

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
#define MENUPRINT printf


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




struct self
{

	FILE *fp = NULL;
	char line[255] = {};
	char buf[MESSAGESIZE] = {};
	int errorVal = 0;
	char *progName = "Control Hub";

	// ******** thread handlers

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
		WorkerThread kpWorker;

	}kp;


	// ******** Current Time
	struct
	{
		time_t time;
		struct tm *currentTime;
		pthread_mutex_t Mtx = PTHREAD_MUTEX_INITIALIZER;

	}tm;


	// ******** Server Struct
	struct
	{
		WorkerThread server;
		pthread_mutex_t Mtx = PTHREAD_MUTEX_INITIALIZER;
		_thread serverThread;
		char *threadName = "control server";
		int living = 1;
		int chid = 0;
		int serverPID = -1;
		int serverCHID = -1;

	}server;


	struct
	{
	int serverPID;
	int serverChID;
	int server_coid;
	_data* msg;
	_reply *reply;
	}client;

}self;



/*-----------------------------------------------------------------------------
* Local Function Declarations
*---------------------------------------------------------------------------*/
//Server/Client
void *serverReceiver(void *chid);
void *serverSender(workBuf *work);
void clientDisconnect(int server_coid);
_reply* clientSendMsg(_data* msg, _reply *reply);
int clientConnect(int serverPID,  int serverChID);

//init different services
void threadInit(_thread *th);
void serverInit(void);
bool keypadInit(int prio);

// init  main
void destroy(void);
void init(void);

// callback
void keypad_cb(char keyPress);
void *kpWork(workBuf *work);

//logging
void logData(_data *toLog);
void logData(_reply *toLog);
void logKeyPress(char key);

// printing menu
int printMenu(int mode);


/*-----------------------------------------------------------------------------
* Main Function
*---------------------------------------------------------------------------*/
int main(void)		//TODO: set date and time
{
	int i =25 ;

	std::string tmp(STARTUP_MSG);
	int PID = 0;
	int CHID = 0;


//	std::string tt(CONTROLHUB);
//	tt.append(CONTROLHUB_SERVER);
//	read_pid_chid_FromFile(&PID, &CHID, tt.c_str());
//
//	std::cout << "PID=" << PID << std::endl;
//	std::cout << "CHID=" << CHID << std::endl;
//

	init();



//	char input = printMenu(1);
	//splash_screen2();


// SCREEN:

	while(1)
	{
		sleep(1);
	}


	destroy();
	return EXIT_SUCCESS;
}



/*-----------------------------------------------------------------------------
* Local Function Definitions
*---------------------------------------------------------------------------*/
/* ----------------------------------------------------	*
 *	@kpWork Implementation:							*
 *	@brief:												*
 *	@return:											*
 * ---------------------------------------------------	*/
void *kpWork(workBuf *work)
{
	if (work == NULL)
		if (work->data == NULL)
			return NULL;

	char tmp = work->data->c_str()[0];

	logKeyPress(tmp);

	//self.server.server.doWork(buf, size, mode)
	switch(tmp)
	{
	case '1':
		//RequestBoomGateChange(DOWN);
		break;
	case '2':
		//RequestBoomGateChange(UP);
		break;
	case '3':
		//UpdateNetworkTime();
		break;
	case '4':
		//RequestTrafficLight1Change(NSG);
		break;
	case '5':
		//RequestTrafficLight1Change(EWG);
		break;
	case '6':
		//RequestTrafficLight2Change(NSG);
		break;
	case '7':
		//RequestTrafficLight2Change(EWG);
		break;
	case '8':
		//RequestTrafficLight2Change(NSTG);
		break;
	case '9':
		//RequestTrafficLight2Change(EWTG);
		break;
	case 'A':
		//RequestTrafficLight1Change(NSTG);
		break;
	case 'B':
		//RequestTrafficLight1Change(EWTG);
		break;
	case 'C':
		//RequestTrafficLight1DecreaseStateChange();
		break;
	case 'D':
		//RequestTrafficLight1IncreaseStateChange();
		break;
	case 'E':
		//RequestTrafficLight2DecreaseStateChange();
		break;
	case 'F':
		//RequestTrafficLight2IncreaseStateChange();
		break;
	case 'G':
		break;
	default:
		//Error();
		;
	}


	return NULL;
}


/* ----------------------------------------------------	*
 *	@keypad_cb Implementation:							*
 *	@brief:												*
 *	@return:											*
 * ---------------------------------------------------	*/
void keypad_cb(char keyPress)
{
	self.kp.kpWorker.doWork(&keyPress, 1, 0);
}



/* ----------------------------------------------------	*
 *	@init Implementation:								*
 *	@brief:												*
 *	@return:											*
 * ---------------------------------------------------	*/
void init(void)
{
	ThreadCtl(_NTO_TCTL_IO_PRIV , (void*)0);
	pthread_setname_np(pthread_self(), 	self.progName);


	std::string tmp(STARTUP_MSG);
	write_string_ToFile(&tmp, CHLOG, "a+");

	serverInit();
	//keypadInit(5);
	//FT800_Init();

}



/* ----------------------------------------------------	*
 *	@destroy Implementation:							*
 *	@brief:												*
 *	@return:											*
 * ---------------------------------------------------	*/
void destroy(void)
{

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
	self.kp.thread.start(&self.kp.keyPad_attr);
	self.kp.kpWorker.setWorkFunction(kpWork);
}



/* ----------------------------------------------------	*
 *	@serverInit Implementation:							*
 *	@brief:												*
 *	@return:											*
 * ---------------------------------------------------	*/
void serverInit(void)
{
	Lock(self.server.Mtx);
	DEBUGF("serverInit()->Initializing Server:\n");
	threadInit(&self.server.serverThread);

	self.server.serverPID = getpid(); 		// get server process ID
	self.server.serverCHID = ChannelCreate(_NTO_CHF_DISCONNECT); // Create Channel

	if (self.server.serverCHID == -1)  // _NTO_CHF_DISCONNECT flag used to allow detach
	{
		DEBUGF("serverInit()->Failed to create communication channel on server\n");
		Unlock(self.server.Mtx);
		return;
	}

	DEBUGF("serverInit()->Writing server details to a file:");
	DEBUGF("serverInit()->Process ID   : %d \n", self.server.serverPID);
	DEBUGF("serverInit()->Channel ID   : %d \n", self.server.serverCHID);

	write_pid_chid_ToFile(self.server.serverPID, self.server.serverCHID, CONTROLHUB_SERVER);

	DEBUGF("serverInit()->Server listening for clients:\n");

	pthread_create(&self.server.serverThread.thread, NULL, serverReceiver, (void *)&self);

	DEBUGF("serverInit()->Finished server Init:\n");

	self.server.server.setWorkFunction(serverSender);

	Unlock(self.server.Mtx);
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
 *	@serverReceiver Implementation:						*
 *	@brief:												*
 *	@return:											*
 * ---------------------------------------------------	*/
void *serverReceiver(void *appData)
{
	struct self* self = (struct self*)appData;

	pthread_setname_np(pthread_self(),self->server.threadName);

	int rcvid=0, msgnum=0;  	// no message received yet
	int Stay_alive=0;
	int *living = &self->server.living;	// stay alive and living for controlling the server status
    int read = 0, chid = self->server.chid;
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

            DEBUGF("Server->logging received message");
        	logData(&msg);


        	// TODO: place this into a control function that does all the work
        	switch(msg.ClientID)
        	{
        	case clients::TRAFFIC_L1:
        		//updateTrafficLight1Status();	// ie screen or gui

        		break;
        	case clients::TRAFFIC_L2:
        		//updateTrafficLight2Status();	// ie screen or gui
        		break;
        	case clients::TRAIN_I1:
        		//updateTrainIntersection1Status();	// ie screen or gui
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


/* ----------------------------------------------------	*
 *	@serverReceiver Implementation:						*
 *	@brief:												*
 *	@return:											*
 * ---------------------------------------------------	*/
void *serverSender(workBuf *work)
{
	_data msg;
	_reply reply;
	long rcvid = 0;


	Lock(self.server.Mtx);


	rcvid = MsgSend(self.server.chid, &msg, sizeof(msg), &reply, sizeof(reply));

	Unlock(self.server.Mtx);

	return NULL;
}


/* ----------------------------------------------------	*
 *	@logData Implementation:							*
 *	@brief:												*
 *	@return:											*
 * ---------------------------------------------------	*/
void logData(_data *toLog)
{
	std::string toLogData;
	Lock(self.tm.Mtx)
	self.tm.time = time(NULL);
	self.tm.currentTime = localtime(&self.tm.time);
	std::string time(asctime(self.tm.currentTime));

	// creating cvs formate with time and date stamp
	toLogData.append(time.substr(0,time.length()-1));
	toLogData.append(":Sending->ClientID=");
	toLogData.append(std::to_string(toLog->ClientID));
	toLogData.append(",nsStright=");
	toLogData.append(std::to_string(toLog->data.lightTiming.nsStraight));
	toLogData.append(",nsTurn=");
	toLogData.append(std::to_string(toLog->data.lightTiming.nsTurn));
	toLogData.append(",ewStright=");
	toLogData.append(std::to_string(toLog->data.lightTiming.ewStraight));
	toLogData.append(",ewTurn=");
	toLogData.append(std::to_string(toLog->data.lightTiming.ewTurn));
	toLogData.append(",currentState=");
	toLogData.append(std::to_string(toLog->data.currentState));
	toLogData.append(";\n");
	write_string_ToFile(&toLogData, CHLOG, "a+");
	Unlock(self.tm.Mtx)
}


/* ----------------------------------------------------	*
 *	@server Implementation:								*
 *	@brief:												*
 *	@return:											*
 * ---------------------------------------------------	*/
void logData(_reply *toLog)
{
	std::string toLogData;
	Lock(self.tm.Mtx)
	self.tm.time = time(NULL);
	self.tm.currentTime = localtime(&self.tm.time);
	std::string time(asctime(self.tm.currentTime));
	Unlock(self.tm.Mtx)

	// creating cvs formate with time and date stamp
	toLogData.append(time.substr(0,time.length()-1));
	toLogData.append(":Reply->ClientID=");
	toLogData.append(";\n");
	write_string_ToFile(&toLogData, CHLOG, "a+");
	Unlock(self.tm.Mtx)
}


/* ----------------------------------------------------	*
 *	@logKeyPress Implementation:						*
 *	@brief:												*
 *	@return:											*
 * ---------------------------------------------------	*/
void logKeyPress(char key)
{
	std::string toLogData;
	Lock(self.tm.Mtx)
	self.tm.time = time(NULL);
	self.tm.currentTime = localtime(&self.tm.time);
	std::string time(asctime(self.tm.currentTime));

	// creating cvs formate with time and date stamp
	toLogData.append(time.substr(0,time.length()-1));
	toLogData.append(":Reply->ClientID=");
	toLogData.append(";\n");
	write_string_ToFile(&toLogData, CHLOG, "a+");
	Unlock(self.tm.Mtx)
}



/* ----------------------------------------------------	*
 *	@printMenu Implementation:							*
 *	@brief:												*
 *	@return:											*
 * ---------------------------------------------------	*/
int printMenu(int mode)
{
	char input = 0;
	MENUPRINT("-> 1. update TL 1:\n");
	MENUPRINT("-> 2. update TL 2:\n");
	MENUPRINT("-> 3. update TL 3:\n");
	MENUPRINT("Enter command or 'q' for quit:\n");

	std::cin >> input;

	return input;

}




/* ----------------------------------------------------	*
 *	@printMenu Implementation:							*
 *	@brief:												*
 *	@return:	server_coid								*
 * ---------------------------------------------------	*/
int clientConnect(int serverPID,  int serverChID)
{
    int server_coid;

    DEBUGF("\t--> Trying to connect (server) process which has a PID: %d\n",   serverPID);
    DEBUGF("\t--> on channel: %d\n", serverChID);

	// set up message passing channel
    server_coid = ConnectAttach(ND_LOCAL_NODE, serverPID, serverChID, _NTO_SIDE_CHANNEL, 0);
	if (server_coid == -1)
	{
		DEBUGF("\n\tERROR, could not connect to server!\n\n");
        return server_coid;
	}

	DEBUGF("Connection established to process with PID:%d, Ch:%d\n", serverPID, serverChID);
    return server_coid;
}



/* ----------------------------------------------------	*
 *	@clientSendMsg Implementation:						*
 *	@brief:												*
 *	@return:	server_coid								*
 * ---------------------------------------------------	*/
_reply* clientSendMsg(int server_coid, _data* msg, _reply *reply)
{

    // the data we are sending is in msg.data
    DEBUGF("Client (ID:%d), sending data packet\n", msg->ClientID);
    fflush(stdout);

    if (MsgSend(server_coid, &msg, sizeof(msg), reply, sizeof(_reply)) == -1)
    {
    	DEBUGF(" Error data NOT sent to server\n");
    	return NULL;
    }
    else
    { // now process the reply
    	DEBUGF("-->Reply is: '%s'\n", reply->buf);
    }


    return reply;
}



/* ----------------------------------------------------	*
 *	@clientDisconnect Implementation:					*
 *	@brief:												*
 *	@return:	server_coid								*
 * ---------------------------------------------------	*/
void clientDisconnect(int server_coid)
{
    DEBUGF("\n Sending message to server to tell it to close the connection\n");
    ConnectDetach(server_coid);
    return;
}






