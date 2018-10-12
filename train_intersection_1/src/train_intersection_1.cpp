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
#include <vector>

#include "threadTemplate.h"
#include "DelayTimer.h"

using namespace std;

/*-----------------------------------------------------------------------------
* Definitions
*---------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------
* Local Variables and Buffers
*---------------------------------------------------------------------------*/
enum states{
	initState,
	boomgateUpState,
	boomAlarmState,
	boomgateDownState
};

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

	/*
	//setup sensor input thread
	pthread_attr_t sensorInput_attr;
	struct sched_param sensorInput_param;
	pthread_attr_init(&sensorInput_attr);
	pthread_attr_setschedpolicy(&sensorInput_attr, SCHED_RR);
	sensorInput_param.sched_priority = 20;
	pthread_attr_setschedparam (&sensorInput_attr, &sensorInput_param);
	pthread_attr_setinheritsched (&sensorInput_attr, PTHREAD_EXPLICIT_SCHED);
	pthread_attr_setstacksize (&sensorInput_attr, 8000);

	pthread_t sesorInputThread;
	void *sensorInputRetval;
	pthread_create(&sesorInputThread, &sensorInput_attr, sensorInput_ex, NULL);
	*/

	while(1){

		sleep(1);
	}



	//close threads
	pthread_join(SMThread, &SMThreadRetval);
	//pthread_join(sesorInputThread, &sensorInputRetval);

	printf("\nMain Terminating...\n");
	return EXIT_SUCCESS;
}

/*-----------------------------------------------------------------------------
* Thread Definitions
*---------------------------------------------------------------------------*/

void *trainStateMachine_ex(void *data){
	trainStateMachine();
	return 0;
}

/*
void *sensorInput_ex(void *data){
	getSensorInput();
	return 0;
}
*/

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
 	usleep(1);
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

	while (keepLooping)
	{
		//getSensorInput();					//read key pad

		if(sensorInput != lastSensorInput) 	// change state if new input
		{
			lastSensorInput = sensorInput;
		}
		else								// no new input - back to top
		{
			//twoMilSecTimer.createTimer();
			usleep(1);
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
				inboundSensorFaultTimer.createTimer();
				//getSensorInput();
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
				cout << "Boomgate 1 DOWN FAULT" << endl;		//TODO send fault message
			}													//TODO turn on in bound train stop lights
			lastSensorInput = lastSensorInputReset;
			break;

		case boomgateDownState:
			if ((!train_1_arrive_1 && !train_1_arrive_2) && (!train_2_arrive_1 && !train_2_arrive_2))
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



