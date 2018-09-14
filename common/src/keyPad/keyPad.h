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


/*-----------------------------------------------------------------------------
* Definitions
*---------------------------------------------------------------------------*/



/*-----------------------------------------------------------------------------
* Global Variables and Buffers
*---------------------------------------------------------------------------*/



class keyPad
{

private:

    void mainWorkThread(void *appData);

public:

    keyPad();
    ~keyPad();
	
    void start();
	void stop();
    
    bool registerCallback(void *_cb);
    bool deregisterCallback(void *_cb);
    const struct sigevent* Inthandler( void* area, int id );


};








/*-----------------------------------------------------------------------------
* Global Function Declarations
*---------------------------------------------------------------------------*/


#endif /* SRC_KEYPAD_KEYPAD_H_ */
