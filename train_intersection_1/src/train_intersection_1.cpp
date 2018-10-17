/******************************************************************************
*
* RTS Traffic Light Project 2018
*
*****************************************************************************/
/**
* @file     train_intersection_1.c
* @brief    RTS Traffic Light Project
* @author   Michael Stekla	s3545953
* 			Simon Maddison	s3493550
* 			Shawn Buschmann	s3478646
*/
/*
 * Key pad bindings:
 * 1 = Train 1 Inbound sensor 1
 * 2 = Train 1 Inbound sesnor 2
 * 3 = Train 1 Boom gate down sensor
 * 4 = Train 1 Boomgate down Sensor
 * 5 = Train 1 Outbound sensor
 *
 * 9  = Train 2 Inbound sensor 1
 * 10 = Train 2 Inbound sesnor 2
 * 11 = Train 2 Boom gate down sensor
 * 12 = Train 2 Boomgate down Sensor
 * 13 = Train 2 Outbound sensor
 *
 * LED mapping
 * LED 0 = train 1 stop light
 * LED 1 = train 2 stop light
 * LED 2 = car stop light
 * LED 3 = boomgate error
 */

/*-----------------------------------------------------------------------------
* Included Files
*---------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <keyPad.h>
#include <iostream>
#include <pthread.h>
#include <sys/iofunc.h>
#include <errno.h>

#include "workerThread.h"
#include "DelayTimer.h"
#include "boneGpio.h"
#include "ipc_dataTypes.h"
#include "file_io.h"

using namespace std;

/*-----------------------------------------------------------------------------
* Definitions
*---------------------------------------------------------------------------*/
#define IPC_BUFF_SIZE 100

#define LED0	(1<<21)   // GPIO1_21
#define LED1	(1<<22)   // GPIO1_22
#define LED2	(1<<23)   // GPIO1_23
#define LED3	(1<<24)   // GPIO1_24

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


typedef struct
{
	int priority;
	pthread_t thread;
	pthread_attr_t attr;
	struct sched_param sch_prm;
}_thread;


// ******** Client Struct ********
struct
{
	WorkerThread client;
	pthread_mutex_t Mtx = PTHREAD_MUTEX_INITIALIZER;
	_thread clientWorkThread = {0};
	_thread clientInitThread = {0};
	char *workingthreadName = "control HUB client thread";
	char *servicethreadName = "Train Station Service thread";
	int living = 1;
	int serverPID = 0;
	int serverCHID = 0;
	int *server_coid = 0;
	int nodeDescriptor = 0;
	trainStationStates Train1 = trainStationStates::DEFAULT_TSS;
	_data* msg;
	_data *reply;
}client;

/*-----------------------------------------------------------------------------
* Local Variables and Buffers
*---------------------------------------------------------------------------*/
enum states{
	initState,
	boomgateUpState,
	boomAlarmState,
	boomgateDownState
};


typedef struct
{
	struct _pulse hdr; // Our real data comes after this header
	int ClientID; // our data (unique id from client)
	char data;     // our data
} my_data;

typedef struct
{
	struct _pulse hdr; // Our real data comes after this header
	int ClientID; // our data (unique id from client)
	char data[2];     // our data
} client_data;

typedef struct
{
	struct _pulse hdr;  // Our real data comes after this header
	char buf[IPC_BUFF_SIZE]; // Message we send back to clients to tell them the messages was processed correctly.
} my_reply;

//for sending PID and channel ID to client
typedef struct
{
	int PID;
	int CHID;
} file_params;

size_t read_data(FILE *fp, file_params *p)
{
	return(fread(p, sizeof(file_params), 1, fp));
}


int currentState, oldCurrentState = initState;
bool train_1_arrive_1 = false;
bool train_1_arrive_2 = false;
bool train_2_arrive_1 = false;
bool train_2_arrive_2 = false;
bool train_1_boom_down = false;
bool train_2_boom_down = false;
char sensorInput;
char KeyPress = 0;
int keepLooping = 1;
char lastSensorInputReset = '0';
WorkerThread keyPadBuffer;
my_data controlHubMsg;
//_data controlHubMsg;
//my_data *pmsg;
bool LED_retval = false;
WorkerThread serverWorker;
char serverRead = '0';
int boomStatus = true;	//true = boom down, false = boom up

// connection data (you may need to edit this)
int HUBserverPID = 0;	// CHANGE THIS Value to PID of the server process
int	HUBserverCHID = 0;			// CHANGE THIS Value to Channel ID of the server process (typically 1)

int trainStateToSend = trainStationStates::DEFAULT_TSS;
pthread_mutex_t controlHubRqstMtx = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t boomGateStatusMTX = PTHREAD_MUTEX_INITIALIZER;
int controlHubRqst = 0;

int _chid;

//timer setup
DelayTimer boomGateTimer(true, 0, 3, 0, 3);
DelayTimer inboundSensorFaultTimer(true, 0, 2, 0, 2);
DelayTimer oneMilSecTimer(true, 1, 1000000,1,1000000);
DelayTimer flashLEDTimer(true, 1, 4500000000, 1, 4500000000);
DelayTimer oneSecTimer(true, 0, 1, 0, 1);


/*-----------------------------------------------------------------------------
* Threads Declarations
*---------------------------------------------------------------------------*/
void *trainStateMachine_ex(void *data);
void *server_ex(void *data);
void *client_ex(void *data);
void Client_Start(int prio);
void *clientService(void *notUsed);
int clientConnect(int serverPID,  int serverChID, int nodeDescriptor);
void clientDisconnect(int server_coid);
void *serverReceiver(void *chid);


/*-----------------------------------------------------------------------------
* Local Function Declarations
*---------------------------------------------------------------------------*/
//int printCurrentState(int currentState);
void getSensorInput(char input);
void trainStateMachine();
void keypad_cb(char keypress);
void *work_cb(workBuf *work);
void keypad_cb(char keypress);
int server();
bool writeBoneLeds(uint32_t pin, bool setTo);
void flashLED(int LED_num);
//void *server_cb(workBuf *work);
int _client(int serverPID, int serverChID, int nd);
void resetControlHubRqst();

void threadInit(_thread *th);

void serverInit(void);



/*-----------------------------------------------------------------------------
* Main Function
*---------------------------------------------------------------------------*/
int main() {

	printf("Welcome: Road railway crossing state machine\n");

	keyPadBuffer.setWorkFunction(work_cb);
	//serverWorker.setWorkFunction(server_cb);	//TODO


	FILE *clientFp;
	file_params my_file;
	//open file
	//fp = fopen("/net/BBB_CimosDirect/fs/shawn_file", "r");

	//clientFp = fopen(fileName.c_str(), "r");

//	//read in PID and CHID from file
//	if (clientFp != NULL){
//		while (read_data(clientFp, &my_file) != 0){
//			printf("\nPID = %d", my_file.PID);
//			printf("\nCHAN ID = %d\n\n", my_file.CHID);
//			serverPID = my_file.PID;
//			serverCHID = my_file.CHID;
//		}
//
//		fclose(clientFp);
//	}


	//Thread setup
	//setup Keypad thread
	pthread_attr_t keyPad_attr;
	struct sched_param keyPad_param;
	pthread_attr_init(&keyPad_attr);
	pthread_attr_setschedpolicy(&keyPad_attr, SCHED_RR);
	keyPad_param.sched_priority = 5;
	pthread_attr_setschedparam (&keyPad_attr, &keyPad_param);
	pthread_attr_setinheritsched (&keyPad_attr, PTHREAD_EXPLICIT_SCHED);
	pthread_attr_setstacksize (&keyPad_attr, 8000);
	keyPad kp;						//create keypad object
	kp.registerCallback(keypad_cb);
	kp.start(&keyPad_attr);			//start keypad


	//create state machine thread.
	pthread_t SMThread;
	void *SMThreadRetval;
	pthread_create(&SMThread, NULL, trainStateMachine_ex, NULL);

	//setup server thread
//	pthread_t serverThread;
//	void *serverThreadRetval;
//	pthread_create(&serverThread, NULL, server_ex, NULL);

	serverInit();

	pthread_t clientThread;
	void *clientThreadRetval;
	//pthread_create(&clientThread, NULL, client_ex, NULL);
	Client_Start(10);

	/*
	pthread_attr_t client_attr;
	struct sched_param client_param;
	pthread_attr_init(&client_attr);
	pthread_attr_setschedpolicy(&client_attr, SCHED_RR);
	keyPad_param.sched_priority = 10;
	pthread_attr_setschedparam (&client_attr, &client_param);
	pthread_attr_setinheritsched (&client_attr, PTHREAD_EXPLICIT_SCHED);
	pthread_attr_setstacksize (&client_attr, 8000);
	*/

	//server code
	//int ret = 0;
	//ret = server();

	//close threads
	pthread_join(SMThread, &SMThreadRetval);
//	pthread_join(serverThread, &serverThreadRetval);
	//pthread_join(clientThread, &clientThreadRetval);


	printf("\nMain Terminating...\n");
	return EXIT_SUCCESS;
}

/*-----------------------------------------------------------------------------
* Thread Definitions
*---------------------------------------------------------------------------*/

void *trainStateMachine_ex(void *data){
	int done = 0;
	trainStateMachine();
	while (!done){
		if (controlHubMsg.data == 'q'){		//terminate program condition
			done = 1;
		}
	}
	return 0;
}

void *server_ex(void *data){
	cout << "starting server thread" << endl;
	int ret2 = 0;
	//ret2 = server();
	return 0;
}

void *client_ex(void *data)
{


	return NULL;
}


/*-----------------------------------------------------------------------------
* Local Function Definitions
*---------------------------------------------------------------------------*/

//Ring buffer worker function
void *work_cb(workBuf *work)
{
	//printf("Buf = %s\n",work->data->c_str());
	//printf("Size = %d\n",work->size);
	//printf("Mode = %d\n",work->mode);
	KeyPress = *work->data->c_str();
	cout << "sensorInput: " << KeyPress << endl;
	getSensorInput(KeyPress);
	return NULL;
}

/*
void *server_cb(workBuf *work)		//TODO - Do I need this for recieving? or only sending?
{
	serverRead = *work->data->c_str();
	cout << "server read:  " << serverRead << endl;
	return NULL;
}	*/

//get keypress
void keypad_cb(char keypress){
 	//cout << keypress << endl;
 	keyPadBuffer.doWork(&keypress, sizeof(keypress), 0);
 	oneMilSecTimer.createTimer();
}

//sensor input
void getSensorInput(char input){
	//sensorInput = getchar();			//if using keyboard
	sensorInput = input;				//if using keyPad
	//***************************** train 1 ****************************
	if (sensorInput == '1')			// train 1 inbound sensor 1
	{
		train_1_arrive_1 = true;
	}
	else if (sensorInput == '2')	// train 1 inbound sensor 2
	{
		train_1_arrive_2 = true;
	}
	else if (sensorInput == '3')	// train 1 boomgate down sensor
	{
		train_1_boom_down = true;
	}
	else if (sensorInput == '4')	// train 1 boomgate sensor RESET
	{
		train_1_boom_down = false;
	}
	else if (sensorInput == '5')	// train 1 outbound sensor
	{
		train_1_arrive_1 = false;
		train_1_arrive_2 = false;
	}//***************************** train 2 ****************************
	else if (sensorInput == '9')
	{	// train 2 inbound sensor 1
		train_2_arrive_1 = true;
	}
	else if (sensorInput == 'A')
	{	// train 2 inbound sensor 1
		train_2_arrive_2 = true;
	}
	else if (sensorInput == 'B')
	{	// train 2 boomgate down sensor
		train_2_boom_down = true;
	}
	else if (sensorInput == 'C')
	{	// train 2 boomgate down RESET
		train_2_boom_down = false;
	}
	else if (sensorInput == 'D')
	{	// train 2 outbound sensor
		train_2_arrive_1 = false;
		train_2_arrive_2 = false;
	}
	//flush input buffer of '\n' (prevent printing twice)	If using keyboard
	//while ((sensorInput = getchar() != '\n') && (sensorInput != EOF));
}

//train state machine function
void trainStateMachine(){

	char lastSensorInput = lastSensorInputReset;
	char oldMsgData = lastSensorInputReset;
	bool hubCommand = false;
	//turn off LEDs
	LED_retval = writeBoneLeds(LED0, true);
	LED_retval = writeBoneLeds(LED1, true);
	LED_retval = writeBoneLeds(LED2, true);
	LED_retval = writeBoneLeds(LED3, true);

	while (keepLooping)
	{

		if((sensorInput != lastSensorInput) || (controlHubRqst != oldMsgData)) 	// change state if new input
		{
			oldMsgData = controlHubRqst;
			lastSensorInput = sensorInput;
		}
		else
		{
			oneMilSecTimer.createTimer();
			continue;
		}

		switch (currentState)
		{

		case initState:
			cout << "Initial state" << endl;
			currentState = boomgateUpState;
			lastSensorInput = lastSensorInputReset;
			break;

		case boomgateUpState:
			if (train_1_arrive_1 || train_1_arrive_2)
			{
				currentState = boomAlarmState;
				trainStateToSend = trainStationStates::T1_ARIVING;
				if(!train_1_arrive_1 || !train_1_arrive_2)
				{
					trainStateToSend = trainStationStates::T1_SENSOR_ERROR;
					cout << "Train 1 In-Bound sensor fault" << endl;	//TODO send sensor fault message here
				}
				lastSensorInput = lastSensorInputReset;
			}
			else if(train_2_arrive_1 || train_2_arrive_2)
			{
				currentState = boomAlarmState;
				trainStateToSend = trainStationStates::T2_ARIVING;
				inboundSensorFaultTimer.createTimer();
				if(!train_2_arrive_1 && !train_2_arrive_2)
				{
					trainStateToSend = trainStationStates::T1_SENSOR_ERROR;
					cout << "Train 2 In-Bound sensor fault" << endl;	//TODO send sensor message fault here
				}
				lastSensorInput = lastSensorInputReset;
			}
			else if (controlHubRqst == 11)		//control hub boom down request
			{
				//hubCommand = true;
				cout << "control Hub message received: " << controlHubRqst << endl;
				currentState = boomAlarmState;
				//controlHubMsg.data = lastSensorInputReset;
				resetControlHubRqst();
			}
			else{
				currentState = boomgateUpState;
				trainStateToSend = trainStationStates::BOOM_GATE_UP;
				cout << "BOOMGATE UP" << endl << endl;
			}
			break;

		case boomAlarmState:
			currentState = boomgateDownState;
			cout << "TRAIN INBOUND" << endl;
			Lock(boomGateStatusMTX);
			boomStatus = true;
			Unlock(boomGateStatusMTX);
			boomGateTimer.createTimer();
			if((train_1_boom_down == false) || (train_2_boom_down == false))
			{
				trainStateToSend = trainStationStates::BOOM_GATE_ERROR;
				cout << "Boomgate DOWN FAULT" << endl;			//TODO send fault message
				LED_retval = writeBoneLeds(LED0, false);		//turn on in bound train stop lights
				LED_retval = writeBoneLeds(LED1, false);
				LED_retval = writeBoneLeds(LED2, false);		//car stop light
				while((train_1_boom_down == false) || (train_2_boom_down == false))
				{
					flashLED(3);							// flash boomgate fault lights
				}
				LED_retval = writeBoneLeds(LED0, true);		// turn off train stop lights
				LED_retval = writeBoneLeds(LED1, true);
			}
			lastSensorInput = lastSensorInputReset;
			break;

		case boomgateDownState:
			trainStateToSend = trainStationStates::BOOM_GATE_DOWN;
			/*
			if( controlHubMsg.data == 'o')
			{
				hubCommand = false;
			}*/
//			if (!(train_1_arrive_1 && train_1_arrive_2 && train_2_arrive_1 && train_2_arrive_2))

			if ((!train_1_arrive_1 && !train_1_arrive_2) && (!train_2_arrive_1 && !train_2_arrive_2)) //(hubCommand == false) && (controlHubRqst == 10)
			{
				if (controlHubRqst == 10)
				{
					resetControlHubRqst();
					break;
				}
				resetControlHubRqst();
				currentState = boomgateUpState;
				cout << "TRAIN OUTBOUND" << endl;
				boomGateTimer.createTimer();
				if((train_1_boom_down) || (train_2_boom_down))
				{
					trainStateToSend = trainStationStates::BOOM_GATE_ERROR;
					cout << "Boomgate UP FAULT" << endl;		//TODO send fault message
					while(train_1_boom_down || train_2_boom_down)
					{
						flashLED(3);		//flash boomgate fault lights
					}
				}												//TODO turn on in bound train stop lights
				lastSensorInput = lastSensorInputReset;
				LED_retval = writeBoneLeds(LED2, true);			//turn off car stop lights
				trainStateToSend = trainStationStates::BOOM_GATE_UP;
				Lock(boomGateStatusMTX);
				boomStatus = false;
				Unlock(boomGateStatusMTX);
				break;
			}
			else
			{
				trainStateToSend = trainStationStates::BOOM_GATE_DOWN;
				currentState = boomgateDownState;
				cout << "BOOMGATE DOWN!" << endl;
				Lock(boomGateStatusMTX);
				boomStatus = true;
				Unlock(boomGateStatusMTX);
				break;
			}
			break;

		default:
			currentState = initState;
		}
	}
}

//flash LED function
void flashLED(int LED_num)
{
	int led_num = LED_num;
	flashLEDTimer.createTimer();

	if(led_num == 0)
	{
		LED_retval = writeBoneLeds(LED0, false);
		flashLEDTimer.createTimer();
		LED_retval = writeBoneLeds(LED0, true);
	}else if(led_num == 1)
	{
		LED_retval = writeBoneLeds(LED1, false);
		flashLEDTimer.createTimer();
		LED_retval = writeBoneLeds(LED1, true);
	}else if (led_num == 2)
	{
		LED_retval = writeBoneLeds(LED2, false);
		flashLEDTimer.createTimer();
		LED_retval = writeBoneLeds(LED2, true);
	}else if(led_num == 3)
	{
		LED_retval = writeBoneLeds(LED3, false);
		flashLEDTimer.createTimer();
		LED_retval = writeBoneLeds(LED3, true);
	}else
	{
		cout << "Error: wrong input led number into flashLED function." << endl;
	}
}



void serverInit(void)
{
	DEBUGF("serverInit()->Initializing Server:\n");

	// locking server mutex
	Lock(boomGateStatusMTX);

	// Initializing thread attributes
	_thread serverThread = {0};

	threadInit(&serverThread);

	// get server process ID
	 int serverPID = getpid();

	// Create Channel
	 int serverCHID = ChannelCreate(_NTO_CHF_DISCONNECT);

	// _NTO_CHF_DISCONNECT flag used to allow detach
	if (serverCHID == -1)
	{
		DEBUGF("serverInit()->Failed to create communication channel on server\n");

		// unlocking mutex
		Unlock(boomGateStatusMTX);
		return;
	}

	DEBUGF("serverInit()->Writing server details to a file:\n");
	DEBUGF("serverInit()->Process ID   : %d \n", serverPID);
	DEBUGF("serverInit()->Channel ID   : %d \n", serverCHID);

	// writing server info to control hub file
	write_pid_chid_ToFile(serverPID, serverCHID, TRAIN_SERVER);
	_chid = serverCHID;
	DEBUGF("serverInit()->Server listening for clients:\n");

	// starting server receiver
	pthread_create(&serverThread.thread, NULL, serverReceiver, NULL);

	DEBUGF("serverInit()->Finished server Init:\n");

	// unlocking mutex
	Unlock(boomGateStatusMTX);
	return;
}


/* ----------------------------------------------------	*
 *	@serverReceiver Implementation:						*
 *	@brief:												*
 *	@return:											*
 * ---------------------------------------------------	*/
void *serverReceiver(void *data)
{
	int chid = _chid;

	int rcvid=0, msgnum=0;  	// no message received yet
	int Stay_alive=0;
	int error = 0;
	_data msg;
	_data replymsg;

	// getting self (bit pointless considering its a global struct
//	struct self* self = (struct self*)appData;

	// mutex locking
	Lock(boomGateStatusMTX);

	int living = 1;	// stay alive and living for controlling the server status
//    int chid = serverCHID;

	// naming thread
//	pthread_setname_np(pthread_self(),self->server.threadName);

	// Mutex unlocking
	Unlock(boomGateStatusMTX);

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


            if (boomStatus)
            {
            	printf("True\n");
            }
            else
            {
            	printf("false\n");
            }

            // Locking sever mutex
    		Lock(boomGateStatusMTX);
            // Reply message
			MsgReply(rcvid, EOK, &boomStatus, sizeof(boomStatus));


        	// Unlocking mutex
    		Unlock(boomGateStatusMTX);
        }
        else
        {
        	DEBUGF("Server->ERROR->received something, but could not handle it correctly\n");
        }
    }

    // destroyed channel before exiting
	ChannelDestroy(chid);

	return NULL;
}





/*** Server code ***/
int server()
{
	// Get PID and create CHID
	int serverPID = 0, chid = 0; 	// Server PID and channel ID
	serverPID = getpid(); 			// get server process ID

	// Create Channel
	chid = ChannelCreate(_NTO_CHF_DISCONNECT);
	if (chid == -1)  // _NTO_CHF_DISCONNECT flag used to allow detach
	{
		printf("\nFailed to create communication channel on server\n");
		return EXIT_FAILURE;
	}

	printf("Server Listening for Clients on:\n");
	//printf("These printf statements can be removed when the myServer.info file is implemented\n");
	printf("  --> Process ID   : %d \n", serverPID);
	printf("  --> Channel ID   : %d \n", chid);

	// create file and populate with PID and CHID
	FILE *fp;
	file_params my_file;
	my_file.CHID = chid;
	my_file.PID = serverPID;
	int file_test = 0;

	write_pid_chid_ToFile( my_file.PID, my_file.CHID, "/net/BBB_CimosDirect/fs/TrainServer.info", "w");

	int rcvid = 0, msgnum = 0;  	// no message received yet
	int Stay_alive = 0, living = 0;	// server stays running (ignores _PULSE_CODE_DISCONNECT request)

	_reply replymsg; 			// replymsg structure for sending back to client
	replymsg.hdr.type = 0x01;
	replymsg.hdr.subtype = 0x00;

	living = 1;
	while (living)
	{
		// Do your MsgReceive's here now with the chid
		rcvid = MsgReceive(chid, &controlHubMsg, sizeof(controlHubMsg), NULL);

		if (rcvid == -1)  // Error condition, exit
		{
			printf("\nFailed to MsgReceive\n");
			break;
		}

		// did we receive a Pulse or message?
		// for Pulses:
		if (rcvid == 0)  //  Pulse received, work out what type
		{
			switch (controlHubMsg.hdr.code)
			{
			case _PULSE_CODE_DISCONNECT:
				// A client disconnected all its connections by running
				// name_close() for each name_open()  or terminated
				if (Stay_alive == 0)
				{
					ConnectDetach(controlHubMsg.hdr.scoid);
					printf("\nServer was told to Detach from ClientID:%d ...\n", controlHubMsg.ClientID);
					living = 0; // kill while loop
					continue;
				}
				else
				{
					printf("\nServer received Detach pulse from ClientID:%d but rejected it ...\n", controlHubMsg.ClientID);
				}
				break;

			case _PULSE_CODE_UNBLOCK:
				// REPLY blocked client wants to unblock (was hit by a signal
				// or timed out).  It's up to you if you reply now or later.
				printf("\nServer got _PULSE_CODE_UNBLOCK after %d, msgnum\n", msgnum);
				break;

			case _PULSE_CODE_COIDDEATH:  // from the kernel
				printf("\nServer got _PULSE_CODE_COIDDEATH after %d, msgnum\n", msgnum);
				break;

			case _PULSE_CODE_THREADDEATH: // from the kernel
				printf("\nServer got _PULSE_CODE_THREADDEATH after %d, msgnum\n", msgnum);
				break;

			default:
				// Some other pulse sent by one of your processes or the kernel
				printf("\nServer got some other pulse after %d, msgnum\n", msgnum);
				break;

			}
			continue;// go back to top of while loop
		}

		// for messages:
		if (rcvid > 0) // if true then A message was received
		{
			msgnum++;

			// If the Global Name Service (gns) is running, name_open() sends a connect message. The server must EOK it.
			if (controlHubMsg.hdr.type == _IO_CONNECT)
			{
				MsgReply(rcvid, EOK, NULL, 0);
				printf("\n gns service is running....");
				continue;	// go back to top of while loop
			}

			// Some other I/O message was received; reject it
			if (controlHubMsg.hdr.type > _IO_BASE && controlHubMsg.hdr.type <= _IO_MAX)
			{
				MsgError(rcvid, ENOSYS);
				printf("\n Server received and IO message and rejected it....");
				continue;	// go back to top of while loop
			}

			printf("%d\n", boomStatus);
			// Reply message
			MsgReply(rcvid, EOK, &boomStatus, sizeof(boomStatus));
		}
		else
		{
			printf("\nERROR: Server received something, but could not handle it correctly\n");
		}

	}

	printf("\nServer received Destroy command\n");
	// destroyed channel before exiting
	ChannelDestroy(chid);

	return EXIT_SUCCESS;
}

/*** Client code ***/
int _client(int serverPID, int serverChID, int nd)
{
	printf("Client running\n");
	//client_data msg;
	//_reply reply;

	_data reply;
	_data HUBmsg;

	HUBmsg.ClientID = clients::TRAIN_I1;

	int server_coid;
	int index = 0;

	printf("   --> Trying to connect (server) process which has a PID: %d\n", serverPID);
	printf("   --> on channel: %d\n\n", serverChID);

	// set up message passing channel
	server_coid = ConnectAttach(nd, serverPID, serverChID, _NTO_SIDE_CHANNEL, 0);
	if (server_coid == -1)
	{
		printf("\n    ERROR, could not connect to server!\n\n");
		return EXIT_FAILURE;
	}

	printf("Connection established to process with PID:%d, Ch:%d\n", serverPID, serverChID);

	HUBmsg.hdr.type = 0x00;
	HUBmsg.hdr.subtype = 0x00;

	int done = 0;

	while (!done){

		//sleep(1);			//TODO replace with timer
		oneSecTimer.createTimer();
		HUBmsg.train_data.currentState = (trainStationStates)trainStateToSend;

		fflush(stdout);
		int error = 0;
		if (MsgSend(server_coid, &HUBmsg, sizeof(_data), &reply, sizeof(reply)) == -1)
		{
			error = errno;
			printf("Error was: %s\n", strerror(error));
			printf(" Error data NOT sent to server\n");
			// maybe we did not get a reply from the server
			break;
		}
		else
		{ // now process the reply
			//cout << "HUB Reply: " << reply.train_data.currentState << endl;
			DEBUGF("client->(%d) reply message: %d\n", HUBmsg.ClientID, reply.train_data.currentState);

			if((reply.train_data.currentState == 10)||(reply.train_data.currentState == 11)){
				Lock(controlHubRqstMtx);
				controlHubRqst = reply.train_data.currentState;
				Unlock(controlHubRqstMtx);
				cout << "Control Hub Request: " << controlHubRqst << endl;
			}

		}

	}//end while

	// Close the connection
	printf("\n Sending message to server to tell it to close the connection\n");
	ConnectDetach(server_coid);

	return EXIT_SUCCESS;
}

void resetControlHubRqst(){
	Lock(controlHubRqstMtx);
	controlHubRqst = 0;
	Unlock(controlHubRqstMtx)
}



/* ----------------------------------------------------	*
 *	@Client_Start Implementation:						*
 *	@brief:												*
 *	@return:											*
 * ---------------------------------------------------	*/
void Client_Start(int prio)
{
	Lock(client.Mtx);
	client.clientInitThread.priority = prio;
	client.clientWorkThread.priority = prio;
	threadInit(&client.clientInitThread);
	pthread_create(&client.clientInitThread.thread, &client.clientInitThread.attr, clientService, NULL);
	usleep(1);
	Unlock(client.Mtx);
}


/* ----------------------------------------------------	*
 *	@clientService Implementation:						*
 *	@brief:												*
 *	@return:	server_coid								*
 * ---------------------------------------------------	*/
void *clientService(void *notUsed)
{
	bool fileExists = false;
	int nD = -1;
	DelayTimer timout(false, 0, 3, 0, 0);
	int pid = 0;
	int chid = 0;

	Lock(client.Mtx);
	// naming thread
	pthread_setname_np(pthread_self(),client.servicethreadName);
	Unlock(client.Mtx);

	std::string fullFilePath(CONTROLHUB);
	fullFilePath.append(CONTROLHUB_SERVER);

	while(true)
	{
		//checking if file for server exists
		do
		{
			DEBUGF("clientService->checking for file with train server details\n");

			fileExists = checkIfFileExists(fullFilePath.c_str());
			timout.createTimer();

		}while(!fileExists);

		nD = read_pid_chid_FromFile(&pid, &chid, CONTROLHUB, CONTROLHUB_SERVER);

		if (nD != 0)
		{
			break;
			DEBUGF("clientService->Server file found with a valid node descriptor\n");
		}
		timout.createTimer();
	}

	Lock(client.Mtx);
	client.serverPID = pid;
	client.serverCHID = chid;
	client.nodeDescriptor = nD;
	Unlock(client.Mtx);

	// start client service for train station
	client.clientWorkThread.priority = 10;
	threadInit(&client.clientWorkThread);

	//pthread_create(&client.clientWorkThread.thread, &client.clientWorkThread.attr, client_ex, NULL);
	_client(client.serverPID, client.serverCHID, client.nodeDescriptor);
	// wait for working thread to finish
	//pthread_join(client.clientWorkThread.thread, NULL);


	//locking mutex
	Lock(client.Mtx);

	// Check if living and if node has failed.. i.e. a drop.
	if (client.living == 0)
	{
		Unlock(client.Mtx);
		return NULL;
	}
	// create a thread that is this function and then exit this thread.
	pthread_create(&client.clientInitThread.thread, &client.clientInitThread.attr, clientService, NULL);

	// reconnection counter
	// create longer delay?
	Unlock(client.Mtx);

	return NULL;
}



/* ----------------------------------------------------	*
 *	@clientConnect Implementation:						*
 *	@brief:												*
 *	@return:	int										*
 * ---------------------------------------------------	*/
int clientConnect(int serverPID,  int serverChID, int nodeDescriptor)
{
    int server_coid;

    DEBUGF("clientConnect->trying to connect (server) process which has a PID: %d\n", serverPID);
    DEBUGF("clientConnect->on channel: %d\n", serverChID);

	// set up message passing channel
    server_coid = ConnectAttach(nodeDescriptor, serverPID, serverChID, _NTO_SIDE_CHANNEL, 0);
	if (server_coid == -1)
	{
		DEBUGF("clientConnect->ERROR, could not connect to server!\n\n");
        return server_coid;
	}

	DEBUGF("clientConnect->connection established to process with PID:%d, Ch:%d\n", serverPID, serverChID);
    return server_coid;
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


