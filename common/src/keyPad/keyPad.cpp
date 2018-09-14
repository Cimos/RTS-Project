/******************************************************************************
*
* RTS Traffic Light Project 2018
*
*****************************************************************************/
/**
* @file     keyPad.cpp
* @brief    RTS Traffic Light Project
* @author   Michael Stekla	s3545953
* 			Simon Maddison	s3493550
* 			Shawn Buschmann	s3478646
*/





/*-----------------------------------------------------------------------------
* Included Files
*---------------------------------------------------------------------------*/


#include "keyPad.h"
#include "../../public/debug.h"


/*-----------------------------------------------------------------------------
* Definitions
*---------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
* Global Variables and Buffers
*---------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
* Global  Function Implementation
*---------------------------------------------------------------------------*/




/* ----------------------------------------------------	*
 *	@getMsgQueId keyPad constructer:		   			*
 *	@breif:  											*
 *	@return:                                 			*
 * ---------------------------------------------------	*/
keyPad::keyPad()
{
    // create thread 
    // kA = true;
    // malloc any memory needed 
    // Create and init any variables needed
}



/* ----------------------------------------------------	*
 *	@getMsgQueId keyPad deconstructer:		    		*
 *	@breif:  											*
 *	@return:                                 			*
 * ---------------------------------------------------	*/
keyPad::~keyPad()
{
    // kA = false;
    // mainWorkerThread->join();
}


/* ----------------------------------------------------	*
 *	@getMsgQueId keyPad deconstructer:		    		*
 *	@breif:  											*
 *	@return:                                 			*
 * ---------------------------------------------------	*/
  void keyPad::start()
  {
      // if not started thread start//
      // threadRun = true;
  }

/* ----------------------------------------------------	*
 *	@getMsgQueId keyPad deconstructer:		    		*
 *	@breif:  											*
 *	@return:                         		        	*
 * ---------------------------------------------------	*/
void keyPad::stop()
{
    // if thread not running.. nothing
    // else threadRun = false;
}
  
	

/* ----------------------------------------------------	*
 *	@getMsgQueId mainWorkThread:						*
 *	@breif:  											*
 *	@return:         		                        	*
 * ---------------------------------------------------	*/
void keyPad::mainWorkThread(void *appData)
{
    // creates interrupt for data ready from i2c keyboard...
    
    // wait while we wait for data, then call any callbacks that were registered.
}




/* ----------------------------------------------------	*
 *	@getMsgQueId registerCallback:						*
 *	@breif:  											*
 *	@return:         		                        	*
 * ---------------------------------------------------	*/
bool keyPad::registerCallback(void *_cb)
{
    // add _cb to callback registry
}



/* ----------------------------------------------------	*
 *	@getMsgQueId deregisterCallback:					*
 *	@breif:  											*
 *	@return:         		                        	*
 * ---------------------------------------------------	*/
bool keyPad::deregisterCallback(void *_cb)
{
    // remove _cb from callback registry

}


/* ----------------------------------------------------	*
 *	@getMsgQueId Inthandler:					    	*
 *	@breif:  											*
 *	@return:         		                        	*
 * ---------------------------------------------------	*/
const struct sigevent* keyPad::Inthandler( void* area, int id )
{
    // handler that is registed with the os that lets us know about new information from the board
}








