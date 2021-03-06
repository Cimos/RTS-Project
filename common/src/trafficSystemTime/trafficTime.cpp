/******************************************************************************
*
* RTS Traffic Light Project 2018
*
*****************************************************************************/
/**
* @file     trafficTime.cpp
* @brief    RTS Traffic Light Project
* @author   Michael Stekla	s3545953
* 			Simon Maddison	s3493550
* 			Shawn Buschmann	s3478646
*
*
*/





/*-----------------------------------------------------------------------------
* Included Files
*---------------------------------------------------------------------------*/
#include "trafficTime.h"
#include <time.h>

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
/*-----------------------------------------------------------------------------
* Definitions
*---------------------------------------------------------------------------*/

//#define DEBUGF printf("DEBUG->"); printf
#define DEBUGF //

/*-----------------------------------------------------------------------------
* Global Function Definitions
*---------------------------------------------------------------------------*/



/* ----------------------------------------------------	*
 *	@setTimeDate Implementation:						*
 *	@brief:												*
 *	@return:											*
 * ---------------------------------------------------	*/
int setTimeDate(struct tm* _tm)
  {
    int rc;
    char buf[20] = {};

    strftime(buf, 20, "date %m%d%H%M%Y", _tm);
    DEBUGF("%s\n", buf);
    rc = system( buf );
    if( rc == -1 )
    {
    	DEBUGF( "setTimeDate->shell could not be run\n" );
    	return -1;
    } else
    {
    	DEBUGF("setTimeDate->result of running command is %d\n", WEXITSTATUS( rc ) );
    }
    return 0;
}


int setTimeDateControlHub(void)
{
	int rc;
    // 	month, day, 24h time, year
    // 101812302018
	rc = system( "date 101815452018" );
	if( rc == -1 )
	{
		DEBUGF( "setTimeDate->shell could not be run\n" );
      	return -1;
	} else
	{
      	DEBUGF("setTimeDate->result of running command is %d\n", WEXITSTATUS( rc ) );
	}
	return 0;
}

bool checkIfpeak(time_t *_currenttime, systemTimeAlignment *timing)
{
	char buf[5] = {};

	uint8_t currenthourMin = 0;
	struct tm *currentTime = localtime(((_currenttime == NULL) ? &timing->time : _currenttime));

    strftime(buf, 5, "%H", currentTime);
    uint8_t currentHour = atoi(buf);
    memset(buf, 0, 5);
    strftime(buf, 5, "%M", currentTime);
    uint8_t currentMinute = atoi(buf);

    currentHour *= 6;
    currentMinute *= 6;
    currenthourMin = currentHour+currentMinute;
    DEBUGF("Hour=%d\n", currentHour);
    DEBUGF("Min=%d\n", currentMinute);
    DEBUGF("currenthourMin=%d\n", currenthourMin);
    DEBUGF("timing->morningPeakStart=%d\n", timing->morningPeakStart);
    DEBUGF("timing->morningPeakFinish=%d\n", timing->morningPeakFinish);
    DEBUGF("timing->eveningPeakStart=%d\n", timing->eveningPeakStart);
    DEBUGF("timing->eveningPeakFinish=%d\n", timing->eveningPeakFinish);


    if ((currenthourMin > timing->morningPeakStart
    		&& currenthourMin < timing->morningPeakFinish)
			|| (currenthourMin > timing->eveningPeakStart
			&& currenthourMin < timing->eveningPeakFinish))
    {
    	DEBUGF("trafficTime->On Peak hour\n");
    	return true;
    }

    DEBUGF("trafficTime->Off Peak hour\n");
    return false;

}







/*-----------------------------------------------------------------------------
* Local Variables and Buffers
*---------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
* Local Function Definitions
*---------------------------------------------------------------------------*/


