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
//#include <pthread.h>
#include <unistd.h>

//#include "lcdThread.h"

/*-----------------------------------------------------------------------------
* Definitions
*---------------------------------------------------------------------------*/
//#define SHAWN
//#define SIMON
#define MICHAEL

#define SLEEP_GREEN		2
#define SLEEP_RED		1
#define SLEEP_YELLOW	1

/*-----------------------------------------------------------------------------
* Local Variables and Buffers
*---------------------------------------------------------------------------*/
enum TRAFFIC_STATES {
	INITIALIZE = 0,
	EWNS_STOP,
	EWG_NSR,
	EWY_NSR,
	EWR_NSR,
	EWR_NSG,
	EWR_NSY,
	INVALID_STATE
};

typedef struct
{
	enum 	TRAFFIC_STATES current_state;	// Current State of traffic lights
	char 	sensor;				// Traffic light sensor input
	int 	counter;			// Runtime counter
} traffic_data;

typedef struct
{
	int 	pid;				// Server Process ID
	int 	chid;				// Server Channel ID
	char 	message;			// Message passed through server
	int 	running;			// Server running status
} server_info;

/*-----------------------------------------------------------------------------
* Threads Declarations
*---------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
* Local Function Declarations
*---------------------------------------------------------------------------*/
void print_state(TRAFFIC_STATES *current_state);
void trafficlights_change(TRAFFIC_STATES *current_state);

/*-----------------------------------------------------------------------------
* Main Function
*---------------------------------------------------------------------------*/
int main(void)
{
	printf("Traffic Lights - Intersection 1 - Main started.\n");

	traffic_data traffic;

	traffic.current_state = INITIALIZE;
	traffic.counter = 0;
	traffic.sensor = '0';

	for (int i = 0; i < 10; i++)
	{
		print_state( &traffic.current_state );
		trafficlights_change( &traffic.current_state );
	}

	printf("Traffic Lights - Intersection 1 - Main terminated.\n");
	return EXIT_SUCCESS;
}

/*-----------------------------------------------------------------------------
* Thread Definitions
*---------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------
* Local Function Definitions
*---------------------------------------------------------------------------*/
void print_state(TRAFFIC_STATES *current_state)
{
	switch(*current_state)
	{
	case INITIALIZE:
		// EWR-NSR(0)
		// Initial State
		printf("Current state: EWR-NSR(0)\t Lights: EW(RED) \tNS(RED)\n");
		break;
	case EWNS_STOP:
		// EWR-NSR(1)
		printf("Current state: EWR-NSR(1)\t Lights: EW(RED) \tNS(RED)\n");
		break;
	case EWG_NSR:
		// EWG-NSR(2)
		printf("Current state: EWG-NSR(2)\t Lights: EW(GREEN) \tNS(RED)\n");
		break;
	case EWY_NSR:
		// EWY-NSR(3)
		printf("Current state: EWY-NSR(3)\t Lights: EW(YELLOW) \tNS(RED)\n");
		break;
	case EWR_NSR:
		// EWR-NSR(4)
		printf("Current state: EWR-NSR(4)\t Lights: EW(RED) \tNS(RED)\n");
		break;
	case EWR_NSG:
		// EWR-NSG(5)
		printf("Current state: EWR-NSG(5)\t Lights: EW(RED) \tNS(GREEN)\n");
		break;
	case EWR_NSY:
		// EWR-NSY(6)
		printf("Current state: EWR-NSY(6)\t Lights: EW(RED) \tNS(YELLOW)\n");
		break;
	default:
		// Unknown State
		printf("ERROR: Unknown state\n");
	}
	fflush(stdout);
}

void trafficlights_change(TRAFFIC_STATES *current_state)
{
	switch(*current_state)
	{
	case INITIALIZE:
		// EWR-NSR(0)
		// Initial State
		sleep(SLEEP_RED);
		*current_state = EWNS_STOP;
		break;
	case EWNS_STOP:
		// EWR-NSR(1)
		sleep(SLEEP_RED);
		*current_state = EWG_NSR;
		break;
	case EWG_NSR:
		// EWG-NSR(2)
		sleep(SLEEP_GREEN);
		*current_state = EWY_NSR;
		break;
	case EWY_NSR:
		// EWY-NSR(3)
		*current_state = EWR_NSR;
		sleep(SLEEP_YELLOW);
		break;
	case EWR_NSR:
		// EWR-NSR(4)
		sleep(SLEEP_RED);
		*current_state = EWR_NSG;
		break;
	case EWR_NSG:
		// EWR-NSG(5)
		sleep(SLEEP_GREEN);
		*current_state = EWR_NSY;
		break;
	case EWR_NSY:
		// EWR-NSY(6)
		sleep(SLEEP_YELLOW);
		*current_state = EWNS_STOP;
		break;
	default:
		// Unknown State
		printf("ERROR: Invalid state\n");
	}
	fflush(stdout);
}
