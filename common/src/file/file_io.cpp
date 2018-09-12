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

#include "file_io.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <pthread.h>
//#include <semaphore.h>
//#include <mqueue.h>
//#include <time.h>
//#include <fcntl.h>
#include <errno.h>
//#include <sched.h>
//#include <sys/iofunc.h>
#include <unistd.h>
//#include <String>
//#include <iostream>

/*-----------------------------------------------------------------------------
* Definitions
*---------------------------------------------------------------------------*/

//#define DEBUGF printf("DEBUG->"); printf
#define DEBUGF //

/*-----------------------------------------------------------------------------
* Local Variables and Buffers
*---------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
* Threads Declarations
*---------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
* Local Function Declarations
*---------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
* Local Function Definitions
*---------------------------------------------------------------------------*/



bool checkIfFileExists(const char *fileName, int mode = F_OK | R_OK | W_OK)
{
	int error = -1;

	DEBUGF("Checking If File Exists:\n");

	error = access( fileName, F_OK );

	DEBUGF("Does File Exists: %d", error);
	if (error == 0)
		return true;

	return false;
}


void write_pid_chid_ToFile(void)
{

}

void read_pid_chid_FromFile(void)
{

}



