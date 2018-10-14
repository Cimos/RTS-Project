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

#include "threadTemplate.h"
#include "DelayTimer.h"

using namespace std;

/*-----------------------------------------------------------------------------
* Definitions
*---------------------------------------------------------------------------*/
#define IPC_BUFF_SIZE 100

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
	struct _pulse hdr;  // Our real data comes after this header
	char buf[IPC_BUFF_SIZE]; // Message we send back to clients to tell them the messages was processed correctly.
} my_reply;

//for sending PID and channel ID to client
typedef struct
{
	int PID;
	int CHID;
} file_params;

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
my_data *pmsg;



/*-----------------------------------------------------------------------------
* Threads Declarations
*---------------------------------------------------------------------------*/
void *trainStateMachine_ex(void *data);
//void *sensorInput_ex(void *data);


/*-----------------------------------------------------------------------------
* Local Function Declarations
*---------------------------------------------------------------------------*/
//int printCurrentState(int currentState);
void getSensorInput(char input);
void trainStateMachine();
void keypad_cb(char keypress);
DelayTimer boomGateTimer(true, 0, 3, 0, 3);	//boomgate error delay timer
DelayTimer inboundSensorFaultTimer(true, 0, 2, 0, 2); //inbound train sensor fault timer
DelayTimer twoMilSecTimer(true, 1, 2000000,1,2000000);
void *work_cb(workBuf *work);
void keypad_cb(char keypress);
int server();


/*-----------------------------------------------------------------------------
* Main Function
*---------------------------------------------------------------------------*/
int main() {

	printf("Welcome: Road railway crossing state machine\n");

	keyPadBuffer.setWorkFunction(work_cb);

	//Thread setup
	//setup Keypad thread
	pthread_attr_t keyPad_attr;
	struct sched_param keyPad_param;
	pthread_attr_init(&keyPad_attr);
	pthread_attr_setschedpolicy(&keyPad_attr, SCHED_RR);
	keyPad_param.sched_priority = 6;
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

	//server code
	int ret = 0;
	ret = server();


	//close threads
	pthread_join(SMThread, &SMThreadRetval);

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

//get keypress
void keypad_cb(char keypress){
 	//cout << keypress << endl;
 	keyPadBuffer.doWork(&keypress, sizeof(keypress), 0);
 	//usleep(1);
 	twoMilSecTimer.createTimer();
}

/*
int printCurrentState(int currentState){

	printf("current state is: %d ", currentState);
	if (currentState == 0){
		printf("init state\n");
	}
	else if (currentState == 1){
		printf("Boomgate UP\n");
	}
	else if (currentState == 2){
		printf("Boom Alarm\n");
	}
	else if (currentState == 3){
		printf("Boomgate DOWN\n");
	}
	else{
		printf("Error: current state fault!\n");
	}

	return currentState;
}
*/

/*
 * getInputSensor() Key pad bindings:
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
 */
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


void trainStateMachine(){

	char lastSensorInput = lastSensorInputReset;
	char oldMsgData = lastSensorInputReset;

	while (keepLooping)
	{

		if((sensorInput != lastSensorInput) || (controlHubMsg.data != oldMsgData)) 	// change state if new input
		{
			oldMsgData = controlHubMsg.data;
			lastSensorInput = sensorInput;
		}
		else								// no new input - back to top
		{
			twoMilSecTimer.createTimer();
			//usleep(1);
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
			if ((train_1_arrive_1 || train_1_arrive_2))
			{
				currentState = boomAlarmState;
				inboundSensorFaultTimer.createTimer();

				if(!train_1_arrive_1 || !train_1_arrive_2)
				{
					cout << "Train 1 In-Bound sensor fault" << endl;	//TODO send sensor fault message here
				}
				lastSensorInput = lastSensorInputReset;
			}
			else if(train_2_arrive_1 || train_2_arrive_2)
			{
				currentState = boomAlarmState;
				inboundSensorFaultTimer.createTimer();
				if(!train_2_arrive_1 && !train_2_arrive_2)
				{
					cout << "Train 2 In-Bound sensor fault" << endl;	//TODO send sensor message fault here
				}
				lastSensorInput = lastSensorInputReset;
			}
			else if (controlHubMsg.data == 'i')		//control hub boom down request
			{
				cout << "control Hub message received: " << controlHubMsg.data << endl;
				currentState = boomAlarmState;
				controlHubMsg.data = lastSensorInputReset;
			}
			else{
				currentState = boomgateUpState;
				cout << "BOOMGATE UP" << endl;
			}
			break;

		case boomAlarmState:
			currentState = boomgateDownState;
			cout << "TRAIN INBOUND" << endl;
			boomGateTimer.createTimer();
			if((train_1_boom_down == false) || (train_2_boom_down == false))
			{
				cout << "Boomgate DOWN FAULT" << endl;			//TODO send fault message
			}													//TODO turn on in bound train stop lights
			lastSensorInput = lastSensorInputReset;
			break;

		case boomgateDownState:
			if ((!train_1_arrive_1 && !train_1_arrive_2) && (!train_2_arrive_1 && !train_2_arrive_2) && (controlHubMsg.data != '0'))
			{
				currentState = boomgateUpState;
				cout << "TRAIN OUTBOUND" << endl;
				boomGateTimer.createTimer();
				if((train_1_boom_down) || (train_2_boom_down))
				{
					cout << "Boomgate UP FAULT" << endl;		//TODO send fault message															//TODO turn on in bound train stop lights
				}
				lastSensorInput = lastSensorInputReset;
				break;
			}
			else
			{
				currentState = boomgateDownState;
				cout << "BOOMGATE DOWN!" << endl;
				break;
			}
			break;

		default:
			currentState = initState;
		}
	}
}


/*** Server code ***/
int server()
{
	// Get PID and create CHID
	int serverPID = 0, chid = 0; 	// Server PID and channel ID
	serverPID = getpid(); 		// get server process ID

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

	/*
	*   Your code here to write this information to a file at a known location so a client can grab it...
	*
	*   The data should be written to a file like:
	*   /tmp/myServer.info
	*	 serverPID  (first line of file)
	*	 Channel ID (second line of file)
	*/
	//**************************************************************************************************
	// create file and populate with PID and CHID
	FILE *fp;
	file_params my_file;
	my_file.CHID = chid;
	my_file.PID = serverPID;
	int file_test = 0;

	//create file to write "w" to names shawns file
	fp = fopen("/net/BBB_CimosDirect/fs/shawn_file", "w");

	//write to file
	file_test = fwrite(&my_file, sizeof(file_params), 1, fp);

	fclose(fp);


	//**************************************************************************************************


	int rcvid = 0, msgnum = 0;  	// no message received yet
	int Stay_alive = 0, living = 0;	// server stays running (ignores _PULSE_CODE_DISCONNECT request)

	my_reply replymsg; 			// replymsg structure for sending back to client
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

			// A message (presumably ours) received

			// put your message handling code here and assemble a reply message
			//sprintf(replymsg.buf, "Message %d received", msgnum);
			//printf("Server received data packet with value of %c from client (ID:%d), ", msg.data, msg.ClientID);
			//fflush(stdout);
			//sleep(1); // Delay the reply by a second (just for demonstration purposes)

			//printf("\n    -----> replying with: '%s'\n",replymsg.buf);
			MsgReply(rcvid, EOK, &replymsg, sizeof(replymsg));
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


