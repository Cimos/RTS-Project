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
#define Buffer_size			100	//for ring buffer
#define Number_of_packets	100	//for ring buffer


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
int keepLooping = 1;
char lastSensorInputReset = '0';


char KeyPress = 0;

//ring buffer stuff
int data_ready = 0;
char buffer[Buffer_size];
int count = 1;

typedef struct
{
	//char buffer[Buffer_size];
	//int readFinnised;
	pthread_mutex_t mutex;  // needs to be set to PTHREAD_MUTEX_INITIALIZER;
	pthread_cond_t condvar; // needs to be set to PTHREAD_COND_INITIALIZER;
}app_data;


/*-----------------------------------------------------------------------------
* Threads Declarations
*---------------------------------------------------------------------------*/
void *trainStateMachine_ex(void *data);

void *sensorInput_ex(void *data);


/*-----------------------------------------------------------------------------
* Local Function Declarations
*---------------------------------------------------------------------------*/
//int printCurrentState(int currentState);
void getSensorInput();
void trainStateMachine();
void keypad_cb(char keypress);
DelayTimer boomGateTimer(true, 0, 3, 0, 3);	//boomgate error delay timer
DelayTimer inboundSensorFaultTimer(true, 0, 2, 0, 2); //inbound train sensor fault timer
DelayTimer twoMilSecTimer(true, 1, 2000000,1,2000000);

void *producer(void *Data);
void *consumer(void *Data);

/*
void *work_cb(workBuf *work)
{
	printf("Buf = %s\n",work->data->c_str());
	printf("Size = %d\n",work->size);
	printf("Mode = %d\n",work->mode);

}
*/

/*-----------------------------------------------------------------------------
* Main Function
*---------------------------------------------------------------------------*/
int main() {
	printf("Welcome: Road railway crossing state machine\n");
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
<<<<<<< HEAD
	//create state machine thread.
	pthread_t SMThread;
	void *SMThreadRetval;
	//pthread_create(&SMThread, NULL, trainStateMachine_ex, NULL);

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

=======
>>>>>>> 1-project_dev

	//create keypad object
	keyPad kp;
	kp.registerCallback(keypad_cb);
	kp.start(&keyPad_attr);	//start keypad

	//void* func(workBuf *work)
	//WorkerThread pingpong;
	//pingpong.setWorkFunction(work_cb);
	//pingpong.doWork("Hello World", sizeof("Hello World"), 0);

<<<<<<< HEAD

	//*****************************************************************************************
	//		Consumer/producer stuff
=======
	while(1){
>>>>>>> 1-project_dev

	// set up location for data
		data_ready = 0;

<<<<<<< HEAD
		// initialize the data, mutex and condvar
		app_data data = { PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER };
=======
		if (KeyPress == '1'){
			cout << "rum" << endl;
		}


		cout << "print" << endl;
>>>>>>> 1-project_dev

		pthread_t  th1, th2;

		void *retval;

		//setup for thread priority

		//producer thread
		pthread_attr_t th1_attr;
		struct sched_param th1_param;
		pthread_attr_init(&th1_attr);
		pthread_attr_setschedpolicy(&th1_attr, SCHED_RR);
		th1_param.sched_priority = 5;
		pthread_attr_setschedparam(&th1_attr, &th1_param);
		pthread_attr_setinheritsched(&th1_attr, PTHREAD_EXPLICIT_SCHED);

		//consumer thread
		pthread_attr_t th2_attr;
		struct sched_param th2_param;
		pthread_attr_init(&th2_attr);
		pthread_attr_setschedpolicy(&th2_attr, SCHED_RR);
		th2_param.sched_priority = 4;
		pthread_attr_setschedparam(&th2_attr, &th2_param);
		pthread_attr_setinheritsched(&th2_attr, PTHREAD_EXPLICIT_SCHED);

		// create the producer and consumer threads
		pthread_create(&th1, &th1_attr, producer, &data);
		pthread_create(&th2, &th2_attr, consumer, &data);

		// let the threads run for a bit
		//sleep(Number_of_packets);

		//			END PRoducer/consumer
		//*******************************************************************************

	//close state machine thread
	//pthread_join(SMThread, &SMThreadRetval);

	//pthread_join(sesorInputThread, &sensorInputRetval);

	//join threads to wait until finished for program terminate
	pthread_join(th1, &retval);
	pthread_join(th2, &retval);


	printf("\nMain Terminating...\n");
	return EXIT_SUCCESS;
}

/*-----------------------------------------------------------------------------
* Thread Definitions
*---------------------------------------------------------------------------*/
/*
void *trainStateMachine_ex(void *data){
	trainStateMachine();
	return 0;
}

void *sensorInput_ex(void *data){
	getSensorInput();
	return 0;
}

*/
/*-----------------------------------------------------------------------------
* Local Function Definitions
*---------------------------------------------------------------------------*/






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
void getSensorInput(){
	//sensorInput = getchar();			//if using keyboard
	sensorInput = KeyPress;				//if using keyPad
	//***************************** train 1 ****************************
	if (sensorInput == '1'){		// train 1 inbound sensor 1
		train_1_arrive_1 = true;
	}
	else if (sensorInput == '2'){	// train 1 inbound sensor 2
		train_1_arrive_2 = true;
	}
	else if (sensorInput == '3'){	// train 1 boomgate down sensor
		train_1_boom_down = true;
	}
	else if (sensorInput == '4'){	// train 1 boomgate sensor RESET
		train_1_boom_down = false;
	}
	else if (sensorInput == '5'){	// train 1 outbound sensor
		train_1_arrive_1 = false;
		train_1_arrive_2 = false;
	}//***************************** train 2 ****************************
	else if (sensorInput == '9'){	// train 2 inbound sensor 1
		train_2_arrive_1 = true;
	}
	else if (sensorInput == 'A'){	// train 2 inbound sensor 1
		train_2_arrive_2 = true;
	}
	else if (sensorInput == 'B'){	// train 2 boomgate down sensor
		train_2_boom_down = true;
	}
	else if (sensorInput == 'C'){	// train 2 boomgate down RESET
		train_2_boom_down = false;
	}
	else if (sensorInput == 'D'){	// train 2 outbound sensor
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
		getSensorInput();					//read key pad

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

		switch (currentState){

		case initState:
			cout << "Initial state" << endl;
			currentState = boomgateUpState;
			lastSensorInput = lastSensorInputReset;
			break;

		case boomgateUpState:
			if (train_1_arrive_1 || train_1_arrive_2){
				currentState = boomAlarmState;
				inboundSensorFaultTimer.createTimer();
				getSensorInput();
				if(!train_1_arrive_1 || !train_1_arrive_2){
					cout << "Train 1 In-Bound sensor fault" << endl;	//TODO send sensor fault message here
				}
				lastSensorInput = lastSensorInputReset;
			}
			else if(train_2_arrive_1 || train_2_arrive_2){
				currentState = boomAlarmState;
				inboundSensorFaultTimer.createTimer();
				getSensorInput();
				if(!train_2_arrive_1 && !train_2_arrive_2){
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
			//getSensorInput();
			getSensorInput();
			if(train_1_boom_down || train_2_boom_down){
				boomGateTimer.createTimer();
				getSensorInput();
				if(!train_1_boom_down || !train_2_boom_down){
					cout << "Boomgate 1 DOWN FAULT" << endl;		//TODO send fault message
				}
			}													//TODO turn on in bound train stop lights
			lastSensorInput = lastSensorInputReset;
			break;

		case boomgateDownState:
			//getSensorInput();
			if ((!train_1_arrive_1 && !train_1_arrive_2) && (!train_2_arrive_1 && !train_2_arrive_2)){
				currentState = boomgateUpState;
				cout << "TRAIN OUTBOUND" << endl;
				boomGateTimer.createTimer();
				getSensorInput();
				if((train_1_boom_down) && (train_2_boom_down)){
					cout << "Boomgate UP FAULT" << endl;		//TODO send fault message
															//TODO turn on in bound train stop lights
				}
				lastSensorInput = lastSensorInputReset;
				break;
			}
			else{
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

*/

//get keypress
void keypad_cb(char keypress){
 	KeyPress = keypress;
 	cout << KeyPress << endl;
}


void *consumer(void *Data)
{
	printf("->Consumer thread started...\n");

	app_data *tdc = (app_data*)Data;

	int index = 0;
	char data_value;
	int count = 1;

	while (1)
	{
		pthread_mutex_lock(&tdc->mutex);

		// test the condition and wait until it is true
		while (!data_ready)
		{

		}

		// process data
		data_value = buffer[index];
		index = (++index) % Buffer_size;

		//print consumer data
		printf("consumer:  got data from producer : %c\t\n", data_value);

		count++;
		//sleep(1);

		//used to keep consumer thread alive after the producer has terminated
		if (count < (Buffer_size - 1) || count > Number_of_packets){
			// now change the condition and signal that it has changed
			data_ready = 0;
		}
		else{
			data_ready = 1;
			pthread_cond_signal(&tdc->condvar);
			pthread_mutex_unlock(&tdc->mutex);
		}

		pthread_cond_signal(&tdc->condvar);
		pthread_mutex_unlock(&tdc->mutex);

		//exit condition of consumer thread to terminate
		if (data_value == 'Z')
		{
			pthread_mutex_unlock(&tdc->mutex);
			pthread_exit(NULL);
		}
	}
}


void *producer(void *Data)
{
	printf("->Producer thread started...\n");

	int index = 0;
	char data_value = 'R';
	// initialize the fake data:
	app_data *tdp = (app_data*)Data;

	data_ready = 1;

	char oldInput = 'R';

	while (1)
	{

		sensorInput = KeyPress;

		if(sensorInput != oldInput){ 	// change state if new input

		oldInput = sensorInput;

		data_value = sensorInput;
		sensorInput = 0;

		pthread_mutex_lock(&tdp->mutex);

		//increment data_value and do ring check
		buffer[index] = data_value;
		index = (++index) % Buffer_size;

		//print first value in buffer
		printf("producer:  Data(%c) from simulated h/w ready\n", data_value);
		//data_value++;

		/*
		//terminate condition of producer thread
		if (data_value != 0)
		{
			// now change the condition and signal that it has changed
			data_ready = 1;
			pthread_cond_signal(&tdp->condvar);
			pthread_mutex_unlock(&tdp->mutex);
			pthread_exit(NULL);
			break;
		}
		*/

		// test the condition and wait until it is true
		while (data_ready)
		{
			pthread_cond_wait(&tdp->condvar, &tdp->mutex);
		}

		// now change the condition and signal that it has changed
		data_ready = 1;
		pthread_cond_signal(&tdp->condvar);
		pthread_mutex_unlock(&tdp->mutex);
		}
	} //end while
}
