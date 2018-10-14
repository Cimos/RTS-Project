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
#include <pthread.h>
#include <unistd.h>
#include <iostream>
#include <semaphore.h>
#include <fcntl.h>
#include <String>
#include <unistd.h>
#include <sys/iofunc.h>
#include <sys/netmgr.h>

#include "keyPad.h"
#include "DelayTimer.h"
#include "file_io.h"
#include "ipc_dataTypes.h"

#include "debug.h"
#include "threadTemplate.h"

/*-----------------------------------------------------------------------------
* Definitions
*---------------------------------------------------------------------------*/
//#define SHAWN
//#define SIMON
#define MICHAEL

#define TIMER_STRAIGHT	3
#define TIMER_TURN		2
#define TIMER_YELLOW	1

#define DEBUG_RUN_SERVER 0

#define SEM_FLAGS O_CREAT | O_EXCL

/*-----------------------------------------------------------------------------
* Local Variables and Buffers
*---------------------------------------------------------------------------*/
enum LIGHT_COLORS
{
	LED_OFF = 0,
	LED_GREEN,
	LED_YELLOW,
	LED_RED,
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
	bool ns_straight;
	bool ns_turn;
	bool ns_pedestrian;
	bool ew_straight;
	bool ew_turn;
	bool ew_pedestrian;
	bool train;
};

struct traffic_data
{
	trafficLightStates current_state;
	trafficLightStates next_state;
	trafficLightStates prev_state;

	trafficLightTiming timing;
	TRAFFIC_LIGHTS lights;
	TRAFFIC_SENSORS sensors;

	bool keep_running;
	bool change_state;

	sem_t sem;
	char sem_name[10];
};

//WorkerThread pingpong;

/*-----------------------------------------------------------------------------
* Threads Declarations
*---------------------------------------------------------------------------*/
void *th_statemachine(void *Data);
// TODO - Server read thread
void *th_sensors(void *Data);
// TODO - Timer Threads

void *th_fakeserver(void *Data);

/*-----------------------------------------------------------------------------
* Local Function Declarations
*---------------------------------------------------------------------------*/
void state_display(traffic_data *data);
void state_perform(traffic_data *data);
void state_transition(traffic_data *data);

void init_traffic_data(traffic_data *data);

int client(int serverPID,  int serverChID);

void keypad_cb(char keypress);

/*-----------------------------------------------------------------------------
* Main Function
*---------------------------------------------------------------------------*/
int main(void)
{
	printf("TLI-1 - Main started.\n");


 	pthread_attr_t keyPad_attr;
	struct sched_param keyPad_param;
    pthread_attr_init(&keyPad_attr);
    pthread_attr_setschedpolicy(&keyPad_attr, SCHED_RR);
    keyPad_param.sched_priority = 5;
    pthread_attr_setschedparam (&keyPad_attr, &keyPad_param);
    pthread_attr_setinheritsched (&keyPad_attr, PTHREAD_EXPLICIT_SCHED);
    pthread_attr_setstacksize (&keyPad_attr, 8000);

	keyPad kp;
	kp.registerCallback(keypad_cb);
	// Note: can do kp.start() which will just give it default attributes and priority
	kp.start(&keyPad_attr);





//	// Setup state machine
//	traffic_data traffic;
//	init_traffic_data(&traffic);
//
//	// Setup Keypad
//
//	// Create the named Semaphore
//	traffic.sem = *sem_open(traffic.sem_name, SEM_FLAGS, S_IRWXG, 1); // Group access, 1 = unlocked.
//
//#if DEBUG_RUN_SERVER == 1
//	// Setup Server Connection
//	int pid = 0;
//	int chid = 0;
//
//	read_pid_chid_FromFile(&pid, &chid, CONTROLHUB_SERVER);
//
//	printf("PID = %d\tCID = %d\n", pid, chid);
//
//	if (client(pid, chid))
//	{
//		// Handle Error
//		printf("TLI-1 - Server Connection Error. Main terminated.\n");
//		return EXIT_FAILURE;
//	}
//#else
//	pthread_t th_fake;
//#endif // DEBUG_NO_SERVER == 1
//
//	// Declare Threads
//	pthread_t th_traffic_sm;
////	pthread_t th_sensor;
//	void *retval;
//
//	// Create Threads
//	pthread_create(&th_traffic_sm, NULL, th_statemachine, &traffic);
////	pthread_create(&th_sensor, NULL, th_sensors, &traffic);
//
//#if DEBUG_RUN_SERVER == 0
//	pthread_create(&th_fake, NULL, th_fakeserver, &traffic);
//	pthread_join(th_fake, &retval);
//#endif // DEBUG_RUN_SERVER == 0
//
//	// Join Threads
//	pthread_join(th_traffic_sm, &retval);
////	pthread_join(th_sensor, &retval);
//
//	// Close the named semaphore
//	sem_close(&traffic.sem);
//
//	// Destroy the named semaphore
//	sem_unlink(traffic.sem_name);

	// End of Main
	while(1)
	{
		sleep(1);
	}
	printf("TLI-1 - Main terminated.\n");
	return EXIT_SUCCESS;
}

/*-----------------------------------------------------------------------------
* Thread Definitions
*---------------------------------------------------------------------------*/
void *th_statemachine(void *Data)
{
	printf("TLI-1 - State Machine - Thread Started\n");

	// Cast pointer
	traffic_data *data = (traffic_data*) Data;

	// Default Transitions to setup intersection
	state_display( data );
	state_perform( data );
	state_transition( data );

	// Run traffic continuously
	while(data->keep_running)
	{
		// Display state data
		state_display( data );
		state_perform( data );

		// Check if a state transition is needed
		if (data->change_state == 1)
		{
			// Change state if one of the sensors has been tripped
			state_transition( data );

			// Reset change state variable
			data->change_state = 0;
		}
	}

	printf("TLI-1 - State Machine - Thread Terminating\n");
	return EXIT_SUCCESS;
}

void *th_sensors(void *Data)
{
	printf("TLI-1 - Sensors - Thread Started\n");

	// Cast Pointer
	traffic_data *data = (traffic_data*) Data;

	// Read Sensors Continuously
	while(data->keep_running)
	{
		// Read Sensor Data
		// TODO

	}


	printf("TLI-1 - Sensors - Thread Terminating\n");
	return EXIT_SUCCESS;
}

void *th_fakeserver(void *Data)
{
	printf("TLI-1 - Fake Server - Thread Started\n");

	// Cast pointer
	traffic_data *data = (traffic_data*) Data;

	// Create Timer
	DelayTimer timer(false, 0, 10, 0, 0);

	// Wait for timer to finish
	timer.createTimer();

	// Send signal to stop system
	data->keep_running = 0;

	printf("TLI-1 - Fake Server - Thread Terminating\n");
	return EXIT_SUCCESS;
}

/*-----------------------------------------------------------------------------
* Local Function Definitions
*---------------------------------------------------------------------------*/
void state_display(traffic_data *data)
{
	sem_wait(&data->sem);
	switch(data->current_state)
	{
	case DEFAULT_TLS:
		// Initial State
		printf("Current state: DEFAULT_TLS\t\n");
		break;
	case NSG:
		// North South Green
		printf("Current state: NSG\t(North South Green)\n");
		break;
	case NSY:
		// North South Yellow
		printf("Current state: NSY\t(North South Yellow)\n");
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
	case ERROR_1:
		// Error State 1
		printf("Current state: ERROR_1\n");
		break;
	case ERROR_2:
		// Error State 2
		printf("Current state: ERROR_2\n");
		break;
	case ERROR_3:
		// Error State 3
		printf("Current state: ERROR_3\n");
		break;
	case ERROR_4:
		// Error State 4
		printf("Current state: ERROR_4\n");
		break;
	default:
		// Unknown State
		printf("state_display: Unknown state\n");
	}
	fflush(stdout);
	sem_post(&data->sem);
}

void state_perform(traffic_data *data)
{
	sem_wait(&data->sem);

	// Create Timers
	DelayTimer timer_ns(false, 0, data->timing.nsStraight, 0, 0);
	DelayTimer timer_nst(false, 0, data->timing.nsTurn, 0, 0);
	DelayTimer timer_ew(false, 0, data->timing.ewStraight, 0, 0);
	DelayTimer timer_ewt(false, 0, data->timing.ewTurn, 0, 0);
	DelayTimer timer_y(false, 0, data->timing.yellow, 0, 0);

	switch(data->current_state)
	{
	case DEFAULT_TLS:
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
		timer_ns.createTimer();
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
		timer_y.createTimer();
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
		timer_nst.createTimer();
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
		timer_y.createTimer();
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
		timer_ew.createTimer();
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
		timer_y.createTimer();
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
		timer_ewt.createTimer();
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
		timer_y.createTimer();
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
	case ERROR_1:
	case ERROR_2:
	case ERROR_3:
	case ERROR_4:
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
		printf("state_perform: ERROR State %d", (data->current_state - 9) );
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
	sem_post(&data->sem);
}

void state_transition(traffic_data *data)
{
	sem_wait(&data->sem);

	// Transition States
	data->prev_state = data->current_state;
	data->current_state = data->next_state;

	// Load Next State
	switch(data->current_state)
	{
	case DEFAULT_TLS:
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
			else
			{
				printf("\tERROR - Transition Triggered without sensor data\n");
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
			else
			{
				printf("\tERROR - Transition Triggered without sensor data\n");
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
			else
			{
				printf("\tERROR - Transition Triggered without sensor data\n");
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
			else
			{
				printf("\tERROR - Transition Triggered without sensor data\n");
			}
		}
		else if (data->prev_state == RED)
		{
			printf("\tERROR - Transition Triggered without sensor data\n");
		}
		else
		{
			printf("ERROR - Something really bad has happened with state transitions");
		}
		break;
	case ERROR_1:
	case ERROR_2:
	case ERROR_3:
	case ERROR_4:
		// Error State
		printf("state_transition: ERROR State\n");
		break;
	default:
		// Unknown State
		printf("state_transition: Unknown state\n");
	}
	fflush(stdout);
	sem_post(&data->sem);
}

void init_traffic_data(traffic_data *data)
{
	// Initialize States
	data->current_state = DEFAULT_TLS;
	data->next_state = NSG;
	data->prev_state = DEFAULT_TLS;

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
			TIMER_STRAIGHT,	// NS Straight
			TIMER_TURN,		// NS Turn
			TIMER_STRAIGHT,	// EW Straight
			TIMER_TURN,		// EW Turn
			TIMER_YELLOW,	// Yellow
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

	// Run state machine continuously
	data->keep_running = 1;

	data->change_state = 0;

	// Set the Semaphore name
	strcpy(data->sem_name, "sem_1");
}

int client(int serverPID,  int serverChID)
{
	_data msg_;
    _reply reply_;

    msg_.ClientID = 509;

    int server_coid;
    int index = 0;

	printf("\t--> Trying to connect (server) process which has a PID: %d\n",   serverPID);
	printf("\t--> on channel: %d\n", serverChID);

	// set up message passing channel
    server_coid = ConnectAttach(ND_LOCAL_NODE, serverPID, serverChID, _NTO_SIDE_CHANNEL, 0);
	if (server_coid == -1)
	{
        printf("\n\tERROR, could not connect to server!\n\n");
        return EXIT_FAILURE;
	}

    printf("Connection established to process with PID:%d, Ch:%d\n", serverPID, serverChID);

    // We would have pre-defined data to stuff here
    msg_.hdr.type = 0x00;
    msg_.hdr.subtype = 0x00;

    // Do whatever work you wanted with server connection
    for (index=0; index < 5; index++) // send data packets
    {
    	// set up data packet
    	//msg_.data=10+index;

    	// the data we are sending is in msg.data
        printf("Client (ID:%d), sending data packet with the integer value: %d \n", msg_.ClientID, msg_.data);
        fflush(stdout);

        if (MsgSend(server_coid, &msg_, sizeof(msg_), &reply_, sizeof(reply_)) == -1)
        {
            printf(" Error data '%d' NOT sent to server\n", msg_.data);
            	// maybe we did not get a reply from the server
            break;
        }
        else
        { // now process the reply
            printf("   -->Reply is: '%s'\n", reply_.buf);
        }

        sleep(5);	// wait a few seconds before sending the next data packet
    }

    // Close the connection
    printf("\n Sending message to server to tell it to close the connection\n");
    ConnectDetach(server_coid);

    return EXIT_SUCCESS;
}

void keypad_cb(char keypress)
{
	printf("Key has been pressed\n");
}
