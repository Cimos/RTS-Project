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

/*-----------------------------------------------------------------------------
* Definitions
*---------------------------------------------------------------------------*/
//#define SHAWN
//#define SIMON
//#define MICHAEL


/*-----------------------------------------------------------------------------
* Local Variables and Buffers
*---------------------------------------------------------------------------*/
enum states{
	initState,
	boomgateUpState,
	boomAlarmState,
	boomgateDownState
};

int currentState = initState;
int train_north_arrive = 0;
int train_south_arrive = 0;
char sensorInput;
int keepLooping = 1;

/*-----------------------------------------------------------------------------
* Threads Declarations
*---------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
* Local Function Declarations
*---------------------------------------------------------------------------*/
int printCurrentState(int currentState);
void getSensorInput();
void trainStateMachine();

/*-----------------------------------------------------------------------------
* Main Function
*---------------------------------------------------------------------------*/

int main() {

	printf("Welcome: Road railway crossing state machine\n");

	trainStateMachine();

	printf("\nMain Terminating...\n");
	return EXIT_SUCCESS;
}


/*-----------------------------------------------------------------------------
* Thread Definitions
*---------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------
* Local Function Definitions
*---------------------------------------------------------------------------*/

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
	sleep(1);
	return currentState;
}

void getSensorInput(){
	sensorInput = getchar();
	if (sensorInput == '1'){
		train_north_arrive = 1;
		printf("North train arriving!\n");
	}
	else if (sensorInput == '2'){
		train_south_arrive = 1;
		printf("South train arriving!\n");
	}
	else if (sensorInput == '3'){
		train_north_arrive = 0;
		printf("North train Leaving!\n");
	}
	else if (sensorInput == '4'){
		train_south_arrive = 0;
		printf("South train Leaving!\n");
	}
	//flush input buffer of '\n' (prevent printing twice
	while ((sensorInput = getchar() != '\n') && (sensorInput != EOF));
}

void trainStateMachine(){
	while (keepLooping){
		switch (currentState){

		case initState:
			currentState = printCurrentState(initState);
			currentState = boomgateUpState;
			break;

		case boomgateUpState:
			currentState = printCurrentState(currentState);
			getSensorInput();
			if (train_north_arrive || train_south_arrive){
				currentState = boomAlarmState;
			}
			else{
				currentState = boomgateUpState;
			}
			break;

		case boomAlarmState:
			currentState = printCurrentState(boomAlarmState);
			currentState = boomgateDownState;
			break;

		case boomgateDownState:
			currentState = printCurrentState(boomgateDownState);
			getSensorInput();
			if ((train_north_arrive == 0) && (train_south_arrive == 0)){
				currentState = boomgateUpState;
				break;
			}
			else{
				currentState = boomgateDownState;
				break;
			}
			break;

		default:
			currentState = initState;
		}
	}
}

