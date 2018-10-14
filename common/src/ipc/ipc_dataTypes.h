/******************************************************************************
*
* RTS Traffic Light Project 2018
*
*****************************************************************************/
/**
* @file     ipc_dataTypes.h
* @brief    RTS Traffic Light Project
* @author   Michael Stekla	s3545953
* 			Simon Maddison	s3493550
* 			Shawn Buschmann	s3478646
*
* Idea is to create a thread that can be init to different i2c ports with different i2c addrs.
* You can then register a call back that will get called everytime there is a key press...
* There isnt a limit on the number of callbacks that you register..... This could then be inited
* by a main function and then multiple threads can register to get updates????
*
*/


#ifndef SRC_IPC_ICP_H_
#define SRC_IPC_ICP_H_

/*-----------------------------------------------------------------------------
* Included Files
*---------------------------------------------------------------------------*/

#include "../../public/debug.h"
//
#include <stdint.h>        // for unit32 types
#include <stdlib.h>
#include <sys/iofunc.h>

//#include <stdio.h>
#include <time.h>
/*-----------------------------------------------------------------------------
* Definitions
*---------------------------------------------------------------------------*/

#define MSG_SIZE 10
#define CONTROLHUB_SERVER "ControlServer.info"
//#define TRAFFIC_L1	0x10
//#define TRAFFIC_L2	0x20
//#define TRAIN_I1	0x30
//#define CONTROL_H1	0x40




enum clients
{
	DEFAULT_CLT = 0,
	TRAFFIC_L1,
	TRAFFIC_L2,
	TRAIN_I1,
	CONTROL_H1
};

enum trafficLightStates
{
    DEFAULT_TLS = 0,            // Default state for start up
    NSG,                        // North/South Green
    NSY,                        // North/South Yellow
    NSTG,                       // North/South Turn Green
    NSTY,                       // North/South Turn Yellow
    EWG,                        // East/West Green
    EWY,                        // East/West Yellow
    EWTG,                       // East/West Turn Green
    EWTY,                       // East/West Turn Yellow
    RED,                        // All Red
    ERROR_1,                    // Error States 1   TODO: Define Error states
    ERROR_2,                    // Error States 2
    ERROR_3,                    // Error States 3
    ERROR_4                     // Error States 4
};

enum trainStationStates
{
    DEFAULT_TSS = 0,            // Default state for start up
    T1_ARIVING,                 // Line 1 Train Arriving
    T1_DEPARTING,               // Line 1 Train Departing
    T2_ARIVING,                 // Line 2 Train Arriving
    T2_DEPARTING,               // Line 2 Train Departing
    T1_BOOM_GATE_ERROR,         // Line 1 Boom Gate Error
    T2_BOOM_GATE_ERROR,         // Line 2 BoomGate Error
    T1_SENSOR_ERROR,            // Line 1 Train Arriving Sensor Error
    T2_SENSOR_ERROR             // Line 2 Train Arriving Sensor Error
};


typedef struct
{
    uint8_t nsStraight;         // North/South Straight Light timing
    uint8_t nsTurn;             // North/South Turn Light timing
    uint8_t ewStraight;         // East/West Straight Light timing
    uint8_t ewTurn;             // East/West Turn Light timing
    uint8_t yellow;				// Yellow Light timing
}trafficLightTiming;


typedef struct _controlHubTointersection
{
    trafficLightTiming lightTiming;
    trafficLightStates currentState;
}controler2Intersection;


typedef struct
{
    time_t time;				// Gets the time
	struct tm currentTime;	// Required time_t to get a full date and time

    time_t morningPeakStart;
    time_t morningPeakFinish;
    time_t eveningPeakStart;
    time_t eveningPeakFinish;

}systemTimeAlignment;



typedef struct
{
	struct _pulse hdr; // Our real data comes after this header
	int ClientID; // our data (unique id from client)
	controler2Intersection data;     // our data
} _data;

typedef struct
{
	struct _pulse hdr;  // Our real data comes after this header
    char buf[MSG_SIZE]; // Message we send back to clients to tell them the messages was processed correctly.
} _reply;



/*-----------------------------------------------------------------------------
* Global Variables and Buffers
*---------------------------------------------------------------------------*/








/*-----------------------------------------------------------------------------
* Global Function Declarations
*---------------------------------------------------------------------------*/


#endif /* SRC_IPC_ICP_H_ */
