/******************************************************************************
*
* RTS Traffic Light Project 2018
*
*****************************************************************************/
/**
* @file     keyPad.h
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


#ifndef SRC_KEYPAD_KEYPAD_H_
#define SRC_KEYPAD_KEYPAD_H_

/*-----------------------------------------------------------------------------
* Included Files
*---------------------------------------------------------------------------*/

#include "../../public/debug.h"

#include <stdint.h>        // for unit32 types
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

/*-----------------------------------------------------------------------------
* Definitions
*---------------------------------------------------------------------------*/

typedef struct
{
	int count_thread;
	uintptr_t gpio1_base;
	struct sigevent pevent; // remember to fill in "event" structure in main

	uint64_t  timeout;

}ISR_data;


typedef struct _cbTable
{
	void (*cb)(char);
	struct _cbTable *head;
	struct _cbTable *next;
};

/*-----------------------------------------------------------------------------
* Global Variables and Buffers
*---------------------------------------------------------------------------*/


class keyPad
{

private:

	// Thread Pointers
	pthread_t *workerThread = NULL;
	pthread_attr_t *workerThreadAttr = NULL;

	// Mutex Pointers
	pthread_mutex_t *workerMutex;

	_cbTable *cbTable = NULL;
	ISR_data ISR_area_data;

	// Keep alive bool
	bool kA = true;
	bool threadRun = true;



public:

    keyPad();
    ~keyPad();

    void start(pthread_attr_t *_threadAttr);
    void stop();

    bool registerCallback(void (*_cb)(char));
    bool deregisterCallback(void (*_cb)(char));
};








/*-----------------------------------------------------------------------------
* Global Function Declarations
*---------------------------------------------------------------------------*/


#endif /* SRC_KEYPAD_KEYPAD_H_ */
