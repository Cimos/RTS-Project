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

#include "lcdThread.h"
#include "DelayTimer.h"
#include "boneGpio.h"
#include <FT800.h>
#include "file_io.h"
#include "keyPad.h"
#include "debug.h"
#include "workerThread.h"
#include "trafficTime.h"

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string.h>
#include <stdint.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <sys/wait.h>

//#include <semaphore.h>
//#include <fcntl.h>

//#include <netinet/in.h>
//#include <net/if.h>
//
//#include <sys/neutrino.h>
//#include <sys/netmgr.h>
//#include <sys/iofunc.h>
//
//#include <sys/dispatch.h>
//#include <sys/socket.h>
//#include <sys/types.h>
//#include <sys/ioctl.h>
//#include <sys/iomsg.h>


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
	int priority;
	pthread_t thread;
	pthread_attr_t attr;
	struct sched_param sch_prm;
}_thread;


struct self
{

	FILE *fp = NULL;
	char line[255] = {};
	char buf[MESSAGESIZE] = {};
	int errorVal = 0;
	char *progName = "Control Hub";

	// ********* Keypad
	struct
	{
		keyPad *_kp = new keyPad(true);
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
		_thread serverThread = {0};
		char *threadName = "Control Hub Server";
		int living = 1;
		int serverPID = -1;
		int serverCHID = -1;
//		_data msg;			// received msg
//		_data replymsg;	// replying msg
		trafficLightStates T1 =trafficLightStates::DEFAULT_TLS;
		trafficLightStates T2 =trafficLightStates::DEFAULT_TLS;
		trainStationStates T3 =trainStationStates::DEFAULT_TSS;
		time_t updatedTime = 0;
		// in 10 min incriments
		// default times
		uint8_t morningOnPeak = 45;		// 7:30am
		uint8_t morningOffPeak = 63;	// 10:30am
		uint8_t eveningOnPeak = 93;		// 3:30pm
		uint8_t eveningOffPeak = 111;	// 6:60pm

		// Updates timing information of states of lights
		trafficLightTiming TrafficSequenceT1 = {10,5,10,5,3};

		// Updates timing information of states of lights
		trafficLightTiming TrafficSequenceT2 = {12,7,10,5,3};
	}server;


	// ******** Client Struct
	struct
	{
	WorkerThread client;
	pthread_mutex_t Mtx = PTHREAD_MUTEX_INITIALIZER;
	_thread clientWorkThread = {0};
	_thread clientInitThread = {0};
	char *workingthreadName = "Train Service Work";
	char *servicethreadName = "Train Station Service";
	int living = 1;
	int serverPID = 0;
	int serverCHID = 0;
	int *server_coid = 0;
	int nodeDescriptor = 0;
	trainStationStates Train1 = trainStationStates::DEFAULT_TSS;
	_data* msg;
	_data *reply;
	}client;

}self;



/*-----------------------------------------------------------------------------
* Local Function Declarations
*---------------------------------------------------------------------------*/
//Server/Client
void *serverReceiver(void *chid);
void clientDisconnect(int server_coid);
int clientConnect(int serverPID,  int serverChID, int nodeDescriptor);
void *train_client(void *appData);
void *clientService(void *notUsed);

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
void logTrainData(_data *toLog);
void logIntersectionData(_data *toLog);
//void logData(_data *toLog);
void logKeyPress(char key);

// printing menu
int printMenu(int mode);

// changing peakHour times
void RequestTrafficLightDecreasePeakHour(void);
void RequestTrafficLightIncreasePeakHour(void);

/*-----------------------------------------------------------------------------
* Main Function
*---------------------------------------------------------------------------*/
int main(void)		//TODO: set date and time
{
	std::string tmp(STARTUP_MSG);

//
//	struct tm *currentTime = localtime(&self.tm.time);
//
//	setTimeDateControlHub();
////	setTimeDate(currentTime);
//
//
//	while(1)
//	{
//		sleep(1);
//	}

	init();


	//char input = printMenu(1);


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
 *	@kpWork Implementation:								*
 *	@brief:												*
 *	@return:											*
 * ---------------------------------------------------	*/
void *kpWork(workBuf *work)
{
	if (work == NULL)
		if (work->data == NULL)
			return NULL;
	DEBUGF("kpWork->Key Pressed %c\n", work->data->c_str()[0]);

	char tmp = work->data->c_str()[0];

	logKeyPress(tmp);

	Lock(self.server.Mtx);

	switch(tmp)
	{
	case '1':
		self.server.T3 =trainStationStates::REQUEST_BOOM_GATE_UP;
		break;
	case '2':
		self.server.T3 =trainStationStates::REQUEST_BOOM_GATE_DOWN;
		break;
	case '3':
		self.server.updatedTime = time(NULL);
		self.server.T1 = trafficLightStates::TIMEING_UPDATE;
		self.server.T2 = trafficLightStates::TIMEING_UPDATE;
		break;
	case '4':
		self.server.T1 =trafficLightStates::NSG;
		break;
	case '5':
		self.server.T1 =trafficLightStates::EWG;
		break;
	case '6':
		self.server.T2 =trafficLightStates::NSG;
		break;
	case '7':
		self.server.T2 =trafficLightStates::EWG;
		break;
	case '8':
		self.server.T1 =trafficLightStates::NSTG;
		break;
	case '9':
		self.server.T2 =trafficLightStates::EWTG;
		break;
	case 'A':
		self.server.T1 =trafficLightStates::NSTG;
		break;
	case 'B':
		self.server.T1 =trafficLightStates::EWTG;
		break;
	case 'C':
		//TODO: get this working
		RequestTrafficLightIncreasePeakHour();
		self.server.T1 = trafficLightStates::TIMEING_UPDATE;
		self.server.T2 = trafficLightStates::TIMEING_UPDATE;
		break;
	case 'D':
		RequestTrafficLightDecreasePeakHour();
		self.server.T1 = trafficLightStates::TIMEING_UPDATE;
		self.server.T2 = trafficLightStates::TIMEING_UPDATE;
		break;
	case 'E':
		// T1,T2 standard
		self.server.TrafficSequenceT1 = {10,5,10,5,3};
		self.server.TrafficSequenceT1 = {12,7,10,5,3};
		self.server.T1 = trafficLightStates::TIMEING_UPDATE;
		self.server.T2 = trafficLightStates::TIMEING_UPDATE;
		break;
	case 'F':
		//T1,T2 doubles
		self.server.TrafficSequenceT1 = {20,10,20,10,6};
		self.server.TrafficSequenceT1 = {24,14,20,10,6};
		self.server.T1 = trafficLightStates::TIMEING_UPDATE;
		self.server.T2 = trafficLightStates::TIMEING_UPDATE;
		break;
	case 'G':
		break;
	default:
		//Error();
		;
	}
	Unlock(self.server.Mtx);

	return NULL;
}


/* ----------------------------------------------------	*
 *	@keypad_cb Implementation:							*
 *	@brief:												*
 *	@return:											*
 * ---------------------------------------------------	*/
void keypad_cb(char keyPress)
{
	Lock(self.kp.Mtx);
	// Posting work to thread which isnt time dependent
	self.kp.kpWorker.doWork(&keyPress, 1, 0);
	Unlock(self.kp.Mtx);

	// 1us second delay to reduce multiple key presses
	DelayTimer timer(false, 1, 1000, 0, 0);
	timer.createTimer();
}



/* ----------------------------------------------------	*
 *	@init Implementation:								*
 *	@brief:												*
 *	@return:											*
 * ---------------------------------------------------	*/
void init(void)
{
	// geting io priv's
	ThreadCtl(_NTO_TCTL_IO_PRIV , (void*)0);

	// setting name of program
	pthread_setname_np(pthread_self(), 	self.progName);


	// Writing a new header to the log file
	std::string tmp(STARTUP_MSG);
	write_string_ToFile(&tmp, CHLOG, "a+");

	// starting server, keypad and lcd services.
	setTimeDateControlHub();
	serverInit();
	keypadInit(20);
//	FT800_Init();
//	splash_screen();


}


/* ----------------------------------------------------	*
 *	@destroy Implementation:							*
 *	@brief:												*
 *	@return:											*
 * ---------------------------------------------------	*/
void destroy(void)
{
	// Shut everything down. especially threads
	Lock(self.client.Mtx);
	self.client.living = false;
	Unlock(self.client.Mtx);
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

    // setting up callbacks and worker threads
	self.kp._kp->registerCallback(keypad_cb);
	self.kp._kp->start(&self.kp.keyPad_attr);
	self.kp.kpWorker.setWorkFunction(kpWork);

	return true;
}


/* ----------------------------------------------------	*
 *	@serverInit Implementation:							*
 *	@brief:												*
 *	@return:											*
 * ---------------------------------------------------	*/
void serverInit(void)
{
	DEBUGF("serverInit()->Initializing Server:\n");

	// locking server mutex
	Lock(self.server.Mtx);

	// Initializing thread attributes
	threadInit(&self.server.serverThread);

	// get server process ID
	self.server.serverPID = getpid();

	// Create Channel
	self.server.serverCHID = ChannelCreate(_NTO_CHF_DISCONNECT);

	// _NTO_CHF_DISCONNECT flag used to allow detach
	if (self.server.serverCHID == -1)
	{
		DEBUGF("serverInit()->Failed to create communication channel on server\n");

		// unlocking mutex
		Unlock(self.server.Mtx);
		return;
	}

	DEBUGF("serverInit()->Writing server details to a file:\n");
	DEBUGF("serverInit()->Process ID   : %d \n", self.server.serverPID);
	DEBUGF("serverInit()->Channel ID   : %d \n", self.server.serverCHID);

	// writing server info to control hub file
	write_pid_chid_ToFile(self.server.serverPID, self.server.serverCHID, CONTROLHUB_SERVER);

	DEBUGF("serverInit()->Server listening for clients:\n");

	// starting server receiver
	pthread_create(&self.server.serverThread.thread, NULL, serverReceiver, (void *)&self);

	DEBUGF("serverInit()->Finished server Init:\n");

	// unlocking mutex
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
	// setting up thread attributes
    pthread_attr_init (&th->attr);
    pthread_attr_setschedpolicy(&th->attr, SCHED_RR);
    th->sch_prm.sched_priority = th->priority;
    pthread_attr_setschedparam (&th->attr, &th->sch_prm);
    pthread_attr_setinheritsched (&th->attr, PTHREAD_EXPLICIT_SCHED);
    pthread_attr_setstacksize (&th->attr, 8000);
}


/* ----------------------------------------------------	*
 *	@serverReceiver Implementation:						*
 *	@brief:												*
 *	@return:											*
 * ---------------------------------------------------	*/
void *serverReceiver(void *appData)
{
	int rcvid=0, msgnum=0;  	// no message received yet
	int Stay_alive=0;
	int error = 0;
	_data msg;
	_data replymsg;

	// getting self (bit pointless considering its a global struct
	struct self* self = (struct self*)appData;

	// mutex locking
	Lock(self->server.Mtx);

	int *living = &self->server.living;	// stay alive and living for controlling the server status
    int chid = self->server.serverCHID;

	// naming thread
	pthread_setname_np(pthread_self(),self->server.threadName);

	// Mutex unlocking
	Unlock(self->server.Mtx);

	replymsg.hdr.type = 0x01;
	replymsg.hdr.subtype = 0x00;

    while (living)
    {
    	rcvid = MsgReceive(chid, &msg, sizeof(msg), NULL);

        DEBUGF("Server->Message Received:\n");

        if (rcvid == -1) {
        	DEBUGF("Server->Failed to MsgReceive\n");
        	error = errno;
    	    DEBUGF( "Server->That means: %s\n", strerror( error ) );
            break;
        }
        if (rcvid == 0) {
            switch (msg.hdr.code){

            DEBUGF("Server->Pulse code received");

                case _PULSE_CODE_DISCONNECT:
                    if(Stay_alive == 0)
                    {
                        ConnectDetach(msg.hdr.scoid);
                        DEBUGF("Server->was told to Detach from ClientID:%d ...\n", msg.ClientID);
                        //living = 0; // kill while loop
                        continue;
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
                DEBUGF("Server->gns service is running....\n");
                continue;	// Next loop
            }

            if (msg.hdr.type > _IO_BASE && msg.hdr.type <= _IO_MAX )
            {
                MsgError( rcvid, ENOSYS );
                DEBUGF("Server->received and IO message and rejected it....\n");
                continue; // Next loop
            }

            DEBUGF("Server->logging received message\n");

            // Locking sever mutex
    		Lock(self->server.Mtx);


        	switch(msg.ClientID)
        	{
        	case clients::TRAFFIC_L1:
        		//updateTrafficLight1Status();	// ie screen or gui
        		DEBUGF("Server->message received from Traffic light 1\n");
        		logIntersectionData(&msg);
        		replymsg.inter_data.currentState = self->server.T1;

        		if (self->server.T1 == trafficLightStates::TIMEING_UPDATE)
        		{
        			// updating timing information
					replymsg.timing.time = time(NULL);
					replymsg.timing.morningPeakStart = self->server.morningOnPeak;
					replymsg.timing.morningPeakFinish = self->server.morningOffPeak;
					replymsg.timing.eveningPeakStart = self->server.eveningOnPeak;
					replymsg.timing.eveningPeakFinish = self->server.eveningOffPeak;
					replymsg.inter_data.lightTiming = self->server.TrafficSequenceT1;
				}

                self->server.T1 = trafficLightStates::DEFAULT_TLS;
                MsgReply(rcvid, EOK, &replymsg, sizeof(replymsg));

        		break;
        	case clients::TRAFFIC_L2:
        		//updateTrafficLight2Status();	// ie screen or gui
        		DEBUGF("Server->message received from Traffic light 2\n");
        		logIntersectionData(&msg);
        		replymsg.inter_data.currentState = self->server.T2;

        		if (self->server.T2 == trafficLightStates::TIMEING_UPDATE)
        		{
        			// updating timing information
					replymsg.timing.time = time(NULL);
					replymsg.timing.morningPeakStart = self->server.morningOnPeak;
					replymsg.timing.morningPeakFinish = self->server.morningOffPeak;
					replymsg.timing.eveningPeakStart = self->server.eveningOnPeak;
					replymsg.timing.eveningPeakFinish = self->server.eveningOffPeak;
					replymsg.inter_data.lightTiming = self->server.TrafficSequenceT2;

				}

        		self->server.T2 = trafficLightStates::DEFAULT_TLS;
                MsgReply(rcvid, EOK, &replymsg, sizeof(replymsg));

        		break;
        	case clients::TRAIN_I1:
        		//updateTrainIntersection1Status();	// ie screen or gui
        		DEBUGF("Server->message received from Train Station 1\n");
        		logTrainData(&msg);
        		replymsg.train_data.currentState = self->server.T3;
        		replymsg.timing.time = time(NULL);
        		self->server.T3 = trainStationStates::DEFAULT_TSS;
                MsgReply(rcvid, EOK, &replymsg, sizeof(replymsg));

        		break;
        	default:
        		break;
        	}
        	// Unlocking mutex
    		Unlock(self->server.Mtx);
        }
        else
        {
        	DEBUGF("Server->ERROR->received something, but could not handle it correctly\n");
        }
    }
	return NULL;
}


/* ----------------------------------------------------	*
 *	@logIntersectionData Implementation:				*
 *	@brief:												*
 *	@return:											*
 * ---------------------------------------------------	*/
void logIntersectionData(_data *toLog)
{
	std::string toLogData;
	Lock(self.tm.Mtx)
	self.tm.time = time(NULL);
	self.tm.currentTime = localtime(&self.tm.time);
	std::string time(asctime(self.tm.currentTime));

	// creating cvs formate with time and date stamp
	toLogData.append(time.substr(0,time.length()-1));
	toLogData.append(":Msg->ClientID=");
	toLogData.append(std::to_string(toLog->ClientID));
	toLogData.append(",nsStright=");
	toLogData.append(std::to_string(toLog->inter_data.lightTiming.nsStraight));
	toLogData.append(",nsTurn=");
	toLogData.append(std::to_string(toLog->inter_data.lightTiming.nsTurn));
	toLogData.append(",ewStright=");
	toLogData.append(std::to_string(toLog->inter_data.lightTiming.ewStraight));
	toLogData.append(",ewTurn=");
	toLogData.append(std::to_string(toLog->inter_data.lightTiming.ewTurn));
	toLogData.append(",currentState=");
	toLogData.append(std::to_string(toLog->inter_data.currentState));
	toLogData.append(";\n");
	write_string_ToFile(&toLogData, CHLOG, "a+");
	Unlock(self.tm.Mtx)
}


/* ----------------------------------------------------	*
 *	@logTrainData Implementation:						*
 *	@brief:												*
 *	@return:											*
 * ---------------------------------------------------	*/
void logTrainData(_data *toLog)
{
	std::string toLogData;
	Lock(self.tm.Mtx)
	self.tm.time = time(NULL);
	self.tm.currentTime = localtime(&self.tm.time);
	std::string time(asctime(self.tm.currentTime));

	// creating cvs formate with time and date stamp
	toLogData.append(time.substr(0,time.length()-1));
	toLogData.append(":Msg->ClientID=");
	toLogData.append(std::to_string(toLog->ClientID));
	toLogData.append(",trainState=");
	toLogData.append(std::to_string(toLog->train_data.currentState));
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
	char tmp[2] = {};
	std::string toLogData;
	Lock(self.tm.Mtx)
	self.tm.time = time(NULL);
	self.tm.currentTime = localtime(&self.tm.time);
	std::string time(asctime(self.tm.currentTime));
	tmp[0] = key;
	// creating cvs formate with time and date stamp
	toLogData.append(time.substr(0,time.length()-1));
	toLogData.append(":Sensor->KeyPress=");
	toLogData.append(tmp);
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
 *	@DecreaseStateChange Implementation:				*
 *	@brief:												*
 *	@return:											*
 * ---------------------------------------------------	*/
void RequestTrafficLightDecreasePeakHour(void)

{
	self.server.morningOnPeak-=3;
	self.server.morningOffPeak-=3;
	self.server.eveningOnPeak-=3;
	self.server.eveningOffPeak-=3;
}



/* ----------------------------------------------------	*
 *	@IncreaseStateChange Implementation:				*
 *	@brief:												*
 *	@return:											*
 * ---------------------------------------------------	*/
void RequestTrafficLightIncreasePeakHour(void)
{
	self.server.morningOnPeak+=3;
	self.server.morningOffPeak+=3;
	self.server.eveningOnPeak+=3;
	self.server.eveningOffPeak+=3;
}

