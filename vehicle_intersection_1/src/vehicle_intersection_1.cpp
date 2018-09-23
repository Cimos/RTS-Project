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
enum TRAFFIC_STATES
{
	DEFAULT = 0,
	NSG,
	NSY,
	NSTG,
	NSTY,
	EWG,
	EWY,
	EWTG,
	EWTY,
	RED,
	ERROR
};

enum LIGHT_COLORS
{
	LED_OFF = 0,
	LED_GREEN,
	LED_YELLOW,
	LED_RED,
};

struct TRAFFIC_TIMING
{
	int ns_straight;
	int ns_turn;
	int ew_straight;
	int ew_turn;
};

struct TRAFFIC_LIGHTS
{
	LIGHT_COLORS north_s;
	LIGHT_COLORS north_t;
	LIGHT_COLORS east_s;
	LIGHT_COLORS east_t;
	LIGHT_COLORS south_s;
	LIGHT_COLORS south_t;
	LIGHT_COLORS west_s;
	LIGHT_COLORS west_t;
};

struct TRAFFIC_SENSORS
{
	int ns_straight;
	int ns_turn;
	int ns_pedestrian;
	int ew_straight;
	int ew_turn;
	int ew_pedestrian;
	int train;
};

struct traffic_data
{
	TRAFFIC_STATES current_state;
	TRAFFIC_STATES next_state;
	TRAFFIC_STATES prev_state;

	TRAFFIC_TIMING timing;
	TRAFFIC_LIGHTS lights;
	TRAFFIC_SENSORS sensors;
};

/*-----------------------------------------------------------------------------
* Threads Declarations
*---------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
* Local Function Declarations
*---------------------------------------------------------------------------*/
void state_display(traffic_data *data);
void state_perform(traffic_data *data);
void state_transition(traffic_data *data);

void init_traffic_data(traffic_data *data);

/*-----------------------------------------------------------------------------
* Main Function
*---------------------------------------------------------------------------*/
int main(void)
{
	printf("Traffic Lights - Intersection 1 - Main started.\n");

	traffic_data traffic;

	init_traffic_data(&traffic);

	for (int i = 0; i < 10; i++)
	{
		state_display( &traffic );
		state_perform( &traffic );
		state_transition( &traffic );
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
void state_display(traffic_data *data)
{
	switch(data->current_state)
	{
	case DEFAULT:
		// Initial State
		printf("Current state: DEFAULT\t\n");
		break;
	case NSG:
		// North South Green
		printf("Current state: NSG\t(North South Green)\n");
		break;
	case NSY:
		// North South Yellow
		printf("Current state: NSG\t(North South Yellow)\n");
		break;
	case NSTG:
		// North South Turn Green
		printf("Current state: NSTG\t(North South Turn Green)\n");
		break;
	case NSTY:
		// North South Turn Yellow
		printf("Current state: NSTY\t(North South Turn Yellow)\n");
		break;
	case EWG:
		// East West Green
		printf("Current state: EWG\t(East West Green Green)\n");
		break;
	case EWY:
		// East West Yellow
		printf("Current state: EWY\t(East West Yellow)\n");
		break;
	case EWTG:
		// East West Turn Green
		printf("Current state: EWTG\t(East West Turn Green)\n");
		break;
	case EWTY:
		// East West Turn Yellow
		printf("Current state: EWTY\t(East West Turn Yellow)\n");
		break;
	case RED:
		// All Red
		printf("Current state: RED\t(All Red)\n");
		break;
	case ERROR:
		// Error State
		printf("Current state: ERROR\n");
		break;
	default:
		// Unknown State
		printf("state_display: Unknown state\n");
	}
	fflush(stdout);
}

void state_perform(traffic_data *data)
{
	switch(data->current_state)
	{
	case DEFAULT:
		// Initial State
		data->lights =	{
				LED_OFF, 	// N Straight
				LED_OFF, 	// N Turn
				LED_OFF,	// E Straight
				LED_OFF, 	// E Turn
				LED_OFF,	// S Straight
				LED_OFF, 	// S Turn
				LED_OFF, 	// W Straight
				LED_OFF		// W Turn
		};

		break;
	case NSG:
		// North South Green
		data->lights =	{
				LED_GREEN, 	// N Straight
				LED_RED, 	// N Turn
				LED_RED,	// E Straight
				LED_RED, 	// E Turn
				LED_GREEN,	// S Straight
				LED_RED, 	// S Turn
				LED_RED, 	// W Straight
				LED_RED		// W Turn
		};
		break;
	case NSY:
		// North South Yellow
		data->lights =	{
				LED_YELLOW, // N Straight
				LED_RED, 	// N Turn
				LED_RED,	// E Straight
				LED_RED, 	// E Turn
				LED_YELLOW,	// S Straight
				LED_RED, 	// S Turn
				LED_RED, 	// W Straight
				LED_RED		// W Turn
		};
		break;
	case NSTG:
		// North South Turn Green
		data->lights =	{
				LED_RED, 	// N Straight
				LED_GREEN, 	// N Turn
				LED_RED,	// E Straight
				LED_RED, 	// E Turn
				LED_RED,	// S Straight
				LED_GREEN, 	// S Turn
				LED_RED, 	// W Straight
				LED_RED		// W Turn
		};
		break;
	case NSTY:
		// North South Turn Yellow
		data->lights =	{
				LED_RED, 	// N Straight
				LED_YELLOW, // N Turn
				LED_RED,	// E Straight
				LED_RED, 	// E Turn
				LED_RED,	// S Straight
				LED_YELLOW, // S Turn
				LED_RED, 	// W Straight
				LED_RED		// W Turn
		};
		break;
	case EWG:
		// East West Green
		data->lights =	{
				LED_RED, 	// N Straight
				LED_RED, 	// N Turn
				LED_GREEN,	// E Straight
				LED_RED, 	// E Turn
				LED_RED,	// S Straight
				LED_RED, 	// S Turn
				LED_GREEN, 	// W Straight
				LED_RED		// W Turn
		};
		break;
	case EWY:
		// East West Yellow
		data->lights =	{
				LED_RED, 	// N Straight
				LED_RED, 	// N Turn
				LED_YELLOW,	// E Straight
				LED_RED, 	// E Turn
				LED_RED,	// S Straight
				LED_RED, 	// S Turn
				LED_YELLOW, // W Straight
				LED_RED		// W Turn
		};
		break;
	case EWTG:
		// East West Turn Green
		data->lights =	{
				LED_RED, 	// N Straight
				LED_RED, 	// N Turn
				LED_RED,	// E Straight
				LED_GREEN, 	// E Turn
				LED_RED,	// S Straight
				LED_RED, 	// S Turn
				LED_RED, 	// W Straight
				LED_GREEN	// W Turn
		};
		break;
	case EWTY:
		// East West Turn Yellow
		data->lights =	{
				LED_RED, 	// N Straight
				LED_RED, 	// N Turn
				LED_RED,	// E Straight
				LED_YELLOW, 	// E Turn
				LED_RED,	// S Straight
				LED_RED, 	// S Turn
				LED_RED, 	// W Straight
				LED_YELLOW	// W Turn
		};
		break;
	case RED:
		data->lights =	{
				LED_RED, 	// N Straight
				LED_RED, 	// N Turn
				LED_RED,	// E Straight
				LED_RED, 	// E Turn
				LED_RED,	// S Straight
				LED_RED, 	// S Turn
				LED_RED, 	// W Straight
				LED_RED		// W Turn
		};
		break;
	case ERROR:
		data->lights =	{
				LED_RED, 	// N Straight
				LED_RED, 	// N Turn
				LED_RED,	// E Straight
				LED_RED, 	// E Turn
				LED_RED,	// S Straight
				LED_RED, 	// S Turn
				LED_RED, 	// W Straight
				LED_RED		// W Turn
		};
		printf("state_perform: ERROR State");
		break;
	default:
		data->lights =	{
				LED_RED, 	// N Straight
				LED_RED, 	// N Turn
				LED_RED,	// E Straight
				LED_RED, 	// E Turn
				LED_RED,	// S Straight
				LED_RED, 	// S Turn
				LED_RED, 	// W Straight
				LED_RED		// W Turn
		};
		printf("state_perform: Unknown State");
	}
	fflush(stdout);
}

void state_transition(traffic_data *data)
{
	// Transition States
	data->prev_state = data->current_state;
	data->current_state = data->next_state;

	// Load Next State
	switch(data->current_state)
	{
	case DEFAULT:
		// Initial State
		data->next_state = NSG;
		break;
	case NSG:
		// North South Green
		data->next_state = NSY;
		break;
	case NSY:
		// North South Yellow
		data->next_state = RED;
		break;
	case NSTG:
		// North South Turn Green
		data->next_state = NSTY;
		break;
	case NSTY:
		// North South Turn Yellow
		data->next_state = RED;
		break;
	case EWG:
		// East West Green
		data->next_state = EWY;
		break;
	case EWY:
		// East West Yellow
		data->next_state = RED;
		break;
	case EWTG:
		// East West Turn Green
		data->next_state = EWTY;
		break;
	case EWTY:
		// East West Turn Yellow
		data->next_state = RED;
		break;
	case RED:
		// All Red State
		if (data->prev_state == NSY)
		{
			if (data->sensors.train == 1)
			{
				// Train is present
				data->next_state = EWG;
			}
			else if (data->sensors.ew_turn == 1)
			{
				// Car on EW wants to turn
				data->next_state = EWTG;

				// Reset Sensor
				data->sensors.ew_turn = 0;
			}
			else if (data->sensors.ew_straight == 1 || data->sensors.ew_pedestrian == 1)
			{
				data->next_state = EWG;

				// Reset Sensors
				data->sensors.ew_straight = 0;
				data->sensors.ew_pedestrian = 0;
			}
			else if (data->sensors.ns_turn == 1)
			{
				// NS Car wants to turn
				data->next_state = NSTG;

				// Reset Sensors
				data->sensors.ns_turn = 0;
			}
		}
		else if (data->prev_state == EWTY)
		{
			if (data->sensors.ew_straight == 1 || data->sensors.train)
			{
				// Car on EW wants to go straight, or train is approaching
				data->next_state = EWG;

				// Reset Sensor
				data->sensors.ew_straight = 0;
			}
			else if (data->sensors.ns_turn == 1)
			{
				// NS Car wants to turn
				data->next_state = NSTG;

				// Reset sensor
				data->sensors.ns_turn = 0;
			}
			else if (data->sensors.ns_straight == 1 || data->sensors.ns_pedestrian == 1)
			{
				// NS Car wants to go straight
				data->next_state = NSG;

				// Reset Sensor
				data->sensors.ns_straight = 0;
				data->sensors.ns_pedestrian = 0;
			}
		}
		else if (data->prev_state == EWY)
		{
			if (data->sensors.train == 1)
			{
				// Train is present
				data->next_state = NSTG;
			}
			else if (data->sensors.ns_turn == 1)
			{
				// EW Car wants to turn
				data->next_state = EWTG;

				// Reset sensor
				data->sensors.ns_turn = 0;
			}
			else if (data->sensors.ns_pedestrian == 1 || data->sensors.ns_straight == 1)
			{
				// Pedestrian wants to cross or NS car wants to go straight
				data->next_state = NSG;

				// Reset Sensor
				data->sensors.ns_pedestrian = 0;
				data->sensors.ns_straight = 0;
			}
			else if (data->sensors.ew_turn == 1)
			{
				// EW Car wants to turn
				data->next_state = EWTG;

				// Reset Sensor
				data->sensors.ew_turn = 0;
			}
		}
		else if (data->prev_state == NSTY)
		{
			if (data->sensors.train == 1)
			{
				// Train is present
				data->next_state = EWG;
			}
			else if (data->sensors.ns_straight == 1 || data->sensors.ns_pedestrian == 1)
			{
				// NS car wants to go straight or NS pedestrian wants to cross
				data->next_state = NSG;

				// Reset Sensor
				data->sensors.ns_straight = 0;
				data->sensors.ns_pedestrian = 0;
			}
			else if (data->sensors.ew_turn == 1)
			{
				// EW Car wants to turn
				data->next_state = EWTG;

				// Reset Sensor
				data->sensors.ew_turn = 0;
			}
			else if (data->sensors.ew_straight == 1)
			{
				// EW car wants to go straight
				data->next_state = EWG;

				// Reset Sensor
				data->sensors.ew_straight = 0;
			}
		}
		break;
	case ERROR:
		// Error State
		printf("state_transition: ERROR State\n");
		break;
	default:
		// Unknown State
		printf("state_transition: Unknown state\n");
	}
	fflush(stdout);
}

void init_traffic_data(traffic_data *data)
{
	// Initialize States
	data->current_state = DEFAULT;
	data->next_state = DEFAULT;
	data->prev_state = DEFAULT;

	// Initialize LED Lights
	data->lights =	{
			LED_OFF, 	// N Straight
			LED_OFF, 	// N Turn
			LED_OFF,	// E Straight
			LED_OFF, 	// E Turn
			LED_OFF,	// S Straight
			LED_OFF, 	// S Turn
			LED_OFF, 	// W Straight
			LED_OFF		// W Turn
	};

	// Initialize Timing
	data->timing =	{
			3,	// NS Straight
			1,	// NS Turn
			3,	// EW Straight
			1	// EW Turn
	};

	// Initialize Sensors
	data->sensors = {
			0,	// NS Straight
			0,	// NS Turn
			0,	// NS Pedestrian
			0,	// EW Straight
			0,	// EW Turn
			0,	// EW Pedestrian
			0	// Train
	};
}
