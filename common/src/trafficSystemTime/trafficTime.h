/******************************************************************************
*
* RTS Traffic Light Project 2018
*
*****************************************************************************/
/**
* @file     trafficTime.h
* @brief    RTS Traffic Light Project
* @author   Michael Stekla	s3545953
* 			Simon Maddison	s3493550
* 			Shawn Buschmann	s3478646
*
*
*
*/


#ifndef SRC_TRAFFICSYSTEMTIME_TRAFFICTIME_H_
#define SRC_TRAFFICSYSTEMTIME_TRAFFICTIME_H_

/*-----------------------------------------------------------------------------
* Included Files
*---------------------------------------------------------------------------*/
#include <time.h>
#include "../ipc/ipc_dataTypes.h"

/*-----------------------------------------------------------------------------
* Definitions
*---------------------------------------------------------------------------*/
int setTimeDate(struct tm* _tm);
int setTimeDateControlHub(void);
bool checkIfpeak(_reply msg, systemTimeAlignment *timing);


/*-----------------------------------------------------------------------------
* Global Variables and Buffers
*---------------------------------------------------------------------------*/








/*-----------------------------------------------------------------------------
* Global Function Declarations
*---------------------------------------------------------------------------*/


#endif /* SRC_TRAFFICSYSTEMTIME_TRAFFICTIME_H_ */
