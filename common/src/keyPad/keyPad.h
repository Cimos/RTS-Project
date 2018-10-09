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






/*
//Example: Name does not matter for the callback but the form does.. ie return void, and takes one para of char (not char *)

void keypad_cb(char keypress);


int main()
{
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
}

void keypad_cb(char keypress)
{
 	 // Warning, cant print in here.
}

*/



#ifndef SRC_KEYPAD_KEYPAD_H_
#define SRC_KEYPAD_KEYPAD_H_

/*-----------------------------------------------------------------------------
* Included Files
*---------------------------------------------------------------------------*/

//#include "../../public/debug.h"

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


 struct _cbTable
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


	_cbTable *cbTable = NULL;
	//ISR_data ISR_area_data;

	bool threadRun = true;

	static void *mainWorkThread(void *appData);



public:

	// Keep alive bool
	bool kA = true;

	// Mutex Pointers
	pthread_mutex_t *workerMutex;


    keyPad();
    ~keyPad();

    void start(pthread_attr_t *_threadAttr = NULL);
    void stop();

    bool registerCallback(void (*_cb)(char));
    bool deregisterCallback(void (*_cb)(char));
    _cbTable* getCallback(void);

};








/*-----------------------------------------------------------------------------
* Global Function Declarations
*---------------------------------------------------------------------------*/


#endif /* SRC_KEYPAD_KEYPAD_H_ */
