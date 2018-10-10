/******************************************************************************
*
* RTS Traffic Light Project 2018
*
*****************************************************************************/
/**
* @file     threadTemplate.h
* @brief    RTS Traffic Light Project
* @author   Michael Stekla	s3545953
* 			Simon Maddison	s3493550
* 			Shawn Buschmann	s3478646
*
*/



/*-----------------------------------------------------------------------------
* Included Files
*---------------------------------------------------------------------------*/
#include "threadTemplate.h"
#include <pthread.h>
#include <unistd.h>
#include "../DelayTimer/DelayTimer.h"

/*-----------------------------------------------------------------------------
* Definitions
*---------------------------------------------------------------------------*/

#define RING_BUFFER_SIZE 12


// Use to lock mutex
#define Lock(_Mutex) {		 											\
	do { 																\
		pthread_mutex_lock(&_Mutex); 									\
	} while(0); 														\
}

// Use to unlock mutex
#define Unlock(_Mutex) { 												\
	do { 																\
		pthread_mutex_unlock(&_Mutex);									\
	} while(0); 														\
}


#define INC_READ_RINGBUFFER_INDEX(rBuf) {	 							\
    do { 																\
	  Lock(rBuf.index_mtx);												\
      rBuf.readIndex++; 												\
      rBuf.readIndex=rBuf.readIndex%RING_BUFFER_SIZE; 					\
	  Unlock(rBuf.index_mtx);											\
    } while (0); 														\
}

#define INC_WRITE_RINGBUFFER_INDEX(rBuf) {		 						\
    do { 																\
      Lock(rBuf.index_mtx);												\
	  rBuf.writeIndex++; 												\
      rBuf.writeIndex=rBuf.writeIndex%RING_BUFFER_SIZE; 				\
	  Unlock(rBuf.index_mtx);											\
    } while (0);														\
}

#define GET_RINGBUFFER_INDEX(rBuf,rIndex,wIndex) { 						\
    do { 																\
      Lock(rBuf.index_mtx);												\
      rIndex = rBuf.readIndex;	 										\
      wIndex = rBuf.writeIndex; 										\
	  Unlock(rBuf.index_mtx);											\
    } while (0);														\
}

#define INC_WRITE_AND_GET_RINGBUFFER_INDEX(rBuf, rIndex, wIndex) {		\
    do { 																\
      Lock(rBuf.index_mtx);												\
      rBuf.writeIndex++; 												\
      rBuf.writeIndex = rBuf.writeIndex%RING_BUFFER_SIZE; 				\
      rIndex = rBuf.readIndex; 											\
      wIndex = rBuf.writeIndex; 										\
	  Unlock(rBuf.index_mtx);											\
    } while (0); 														\
}

#define GET_WORKER_THREAD_KEEPALIVE(_this,keepAlive) {		 			\
    do { 																\
	  Lock(_this->keepAliveWork_mtx);									\
      keepAlive = _this->keepWorkThreadAlive; 							\
	  Unlock(_this->keepAliveWork_mtx);									\
    } while (0); 														\
}

#define GET_WORKER_THREAD_KEEPALIVE_AND_RINGBUFFER_INDEX(_this, keepAlive,rBuf,rIndex,wIndex) { \
	do { 																\
	  Lock(_this->keepAliveWork_mtx);									\
	  keepAlive = _this->keepWorkThreadAlive; 							\
	  Unlock(_this->keepAliveWork_mtx);									\
	  Lock(rBuf.index_mtx);												\
	  rIndex = rBuf.readIndex; 											\
	  wIndex = rBuf.writeIndex; 										\
	  Unlock(rBuf.index_mtx);											\
	} while (0);														\
}


/*-----------------------------------------------------------------------------
* Global Variables
*---------------------------------------------------------------------------*/

struct _self
{
public:
	pthread_mutex_t index_mtx = PTHREAD_MUTEX_INITIALIZER;  // needs to be set to PTHREAD_MUTEX_INITIALIZER;
	workBuf work[RING_BUFFER_SIZE] = {};
	int readIndex = 0;
	int writeIndex = 0;
};

/*-----------------------------------------------------------------------------
* Global Function Declarations
*---------------------------------------------------------------------------*/
DelayTimer threadSleep(false, 1, 1000000, 0, 0);

//------------------------------------------------------------------------------
// Public Declarations
//------------------------------------------------------------------------------

class WorkerThread::Private
{
public:

	_self rBuf;

	bool keepWorkThreadAlive = true;
	pthread_mutex_t keepAliveWork_mtx = PTHREAD_MUTEX_INITIALIZER;  // needs to be set to PTHREAD_MUTEX_INITIALIZER;
	pthread_mutex_t work_mtx = PTHREAD_MUTEX_INITIALIZER;  // needs to be set to PTHREAD_MUTEX_INITIALIZER;
	pthread_t *workerThread = NULL;

	Private();
	~Private();

	// Main Worker Thread
	void mainWorkThread(void *appData);

	int initWorkBuf(workBuf *buf2init, int size);
	int deleteWorkBuf(workBuf *buf2delete);
	int copy2workBuf(workBuf *dest, char *src, int size, int mode);

	void *(*cb)(workBuf *_work) = NULL;

	// Credit were credit is due: https://stackoverflow.com/questions/38224532/pthread-create-invalid-use-of-non-static-member-function
    static void* main_wrapper(void* object);

private:
};


//------------------------------------------------------------------------------
// Public Implementation
//------------------------------------------------------------------------------

WorkerThread::WorkerThread()
{
	_pimpl = new Private();
}

/**
* Deconstructor
*/
WorkerThread::~WorkerThread()
{
	delete _pimpl;
}

// If you want to change the type of data passed to the thread you would need to start here and workBuf struct.
int WorkerThread::doWork(char *buf, int size, int mode)
{
	int rIndex = 0, wIndex = 0;
	int timeOut = 5;

	// Get read/write index of the
	GET_RINGBUFFER_INDEX(_pimpl->rBuf, rIndex, wIndex);

	// If write index is about to write over read index.. wait for max 5 milliseconds.
	// If still not, then return error
	while ((rIndex == (wIndex + 1))
		&& (timeOut != 0))
	{
		// std::this_thread::sleep_for(std::chrono::milliseconds(1));
		GET_RINGBUFFER_INDEX(_pimpl->rBuf, rIndex, wIndex);
		timeOut--;
	}

	//init work buf
	_pimpl->initWorkBuf(&_pimpl->rBuf.work[wIndex], size);

	//transfer data to work buf
	_pimpl->copy2workBuf(&_pimpl->rBuf.work[wIndex], buf, size, mode);

	//notify thread of new work
	INC_WRITE_RINGBUFFER_INDEX(_pimpl->rBuf);

	// return success
	return 0;
}

void *WorkerThread::setWorkFunction(void *(*_cb)(workBuf *_work))
{
	if (_cb != NULL)
	{
		_pimpl->cb = _cb;
		return ((void*) _cb);
	}
	else
	{
		return NULL;
	}
}

//------------------------------------------------------------------------------
// Private Implementation
//------------------------------------------------------------------------------
WorkerThread::Private::Private()
{
	workerThread = new pthread_t;

	pthread_create(workerThread, NULL, main_wrapper, (void*)this);
	
	//Allow scheduler to start main_wrapper
	usleep(1);
}

WorkerThread::Private::~Private()
{
	Lock(keepAliveWork_mtx);
	keepWorkThreadAlive = false;
	Unlock(keepAliveWork_mtx);

	pthread_join(*workerThread, NULL);

	delete workerThread;
}

void* WorkerThread::Private::main_wrapper(void* object)
{
	// Credit were credit is due: https://stackoverflow.com/questions/38224532/pthread-create-invalid-use-of-non-static-member-function
	reinterpret_cast<Private*>(object)->mainWorkThread(NULL);
	return 0;
}

void WorkerThread::Private::mainWorkThread(void *appData)
{
	int rIndex = 0, wIndex = 0;
	bool kA = false;
	//bool successful = 0;

	while (true)
	{
		GET_WORKER_THREAD_KEEPALIVE(this, kA);
		if (!kA) { return; }

		GET_RINGBUFFER_INDEX(rBuf, rIndex, wIndex);

		while (rIndex == wIndex)
		{
			GET_WORKER_THREAD_KEEPALIVE_AND_RINGBUFFER_INDEX(this, kA, rBuf, rIndex, wIndex);
			if (!kA) { return; }

			//TODO: Add in chrono sleep here
			// std::this_thread::sleep_for(std::chrono::milliseconds(1));
			threadSleep.createTimer();	//sleep for 1 millisecond

		}

		if (rBuf.work[rIndex].data == NULL)
		{
			INC_READ_RINGBUFFER_INDEX(rBuf);
			continue;
		}

		// Actual work being done here.
		Lock(rBuf.index_mtx);
		if (cb != NULL) {
			cb(rBuf.work);
		}
		Unlock(rBuf.index_mtx);


		// delete work buff
		deleteWorkBuf(&rBuf.work[rIndex]);

		// inc ring buffer
		INC_READ_RINGBUFFER_INDEX(rBuf);
	}
}

int WorkerThread::Private::initWorkBuf(workBuf *buf2init, int size)
{
	int ret = 0;
	try
	{
		buf2init->data = new std::string;
		buf2init->size = size;
		ret++;
	}
	catch (const std::exception& e)
	{
		ret = -1;
	}
	return ret;
}

int WorkerThread::Private::deleteWorkBuf(workBuf *buf2delete)
{
	int ret = 0;
	if (buf2delete->data == NULL)
	{
		return -1;
	}
	try
	{
		delete buf2delete->data;
		buf2delete->data = NULL;
		buf2delete->size = 0;
		ret++;
	}
	catch (const std::exception& e)
	{
		ret = -1;
	}
	return ret;
}

int WorkerThread::Private::copy2workBuf(workBuf *dest, char *src, int size, int mode)
{
	int ret = 0;
	try
	{
		dest->data->assign(src,&src[size]);
		dest->mode = mode;
	}
	catch (const std::exception& e)
	{
		ret = -1;
	}
	return ret;
}


/*
* END OF FILE
* NOTHING BEYOND THIS POINT *
*/
