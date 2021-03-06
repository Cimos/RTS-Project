/******************************************************************************
*
* RTS Traffic Light Project 2018
*
*****************************************************************************/
/**
* @file     workerThread.h
* @brief    RTS Traffic Light Project
* @author   Michael Stekla	s3545953
* 			Simon Maddison	s3493550
* 			Shawn Buschmann	s3478646
*
*/

/*
	Example:

	How to use, create function that returns a void* and takes a (workBuf *)
							eg: void* func(workBuf *work)




WorkerThread pingpong;

pingpong.setWorkFunction(work_cb);


pingpong.doWork("Hello World", sizeof("Hello World"), 0);


void *work_cb(workBuf *work)
{
	printf("Buf = %s\n",work->data->c_str());
	printf("Size = %d\n",work->size);
	printf("Mode = %d\n",work->mode);

	return NULL;
}
*/


#ifndef SRC_WORKERTHREAD_WORKERTHREAD_H_
#define SRC_WORKERTHREAD_WORKERTHREAD_H_

/*-----------------------------------------------------------------------------
* Included Files
*---------------------------------------------------------------------------*/

//#include "../../public/debug.h"
#include <string>

/*-----------------------------------------------------------------------------
* Definitions
*---------------------------------------------------------------------------*/
// Can add other modes which could be used in doWork
#define DATA_MODE 1
#define CMD_MODE 2
/*-----------------------------------------------------------------------------
* Global Variables and Buffers
*---------------------------------------------------------------------------*/

// Customize your working buffer/struct here to what you want.
struct workBuf{
  std::string *data = NULL;
  int mode = 0;
  int size = 0;
};


/*-----------------------------------------------------------------------------
* Global Function Declarations
*---------------------------------------------------------------------------*/
class WorkerThread // Choose Name
{

private:
  	class Private;
  	Private *_pimpl;

public:
	int doWork(char *buf, int size, int mode);
	void *setWorkFunction(void *(*cb)(workBuf *_work));

	WorkerThread();
	WorkerThread(int Timer_base, long Init_time, int Interval_base, long Interval_time);	// Define the interval the worker thread checks for new work
	~WorkerThread();
};




#endif /* SRC_WORKERTHREAD_WORKERTHREAD_H_ */

