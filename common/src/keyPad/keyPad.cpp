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
*
*
*  	XC4602 pin     -> BeagleBone Black Pin
*  	VCC - VDD_3V3B -> pin P9_03 or P9_04
*  	GND - DGND     -> pin P9_01 or P9_02
* 	SCL - GPIO1_16 -> pin P9_15
* 	SD0 - GPIO1_28 -> pin P9_12
*
*
*/





/*-----------------------------------------------------------------------------
* Included Files
*---------------------------------------------------------------------------*/


#include "keyPad.h"

#include <hw/inout.h>      // for in32() and out32();
#include <sys/mman.h>      // for mmap_device_io();
#include <sys/neutrino.h>  // for ThreadCtl( _NTO_TCTL_IO_PRIV , NULL)
#include <sched.h>
#include <sys/procmgr.h>
#include <String>

/*-----------------------------------------------------------------------------
* Definitions
*---------------------------------------------------------------------------*/


#define AM335X_CONTROL_MODULE_BASE   (uint64_t) 0x44E10000
#define AM335X_CONTROL_MODULE_SIZE   (size_t)   0x00001448
#define AM335X_GPIO_SIZE             (uint64_t) 0x00001000
#define AM335X_GPIO1_BASE            (size_t)   0x4804C000

#define LED0          (1<<21)   // GPIO1_21
#define LED1          (1<<22)   // GPIO1_22
#define LED2          (1<<23)   // GPIO1_23
#define LED3          (1<<24)   // GPIO1_24

#define SD0 (1<<28)  // SD0 is connected to GPIO1_28
#define SCL (1<<16)  // SCL is connected to GPIO1_16


#define GPIO_OE        0x134
#define GPIO_DATAIN    0x138
#define GPIO_DATAOUT   0x13C

#define GPIO_IRQSTATUS_SET_1 0x38   // enable interrupt generation
#define GPIO_IRQWAKEN_1      0x48   // Wakeup Enable for Interrupt Line
#define GPIO_FALLINGDETECT   0x14C  // set falling edge trigger
#define GPIO_CLEARDATAOUT    0x190  // clear data out Register
#define GPIO_IRQSTATUS_1     0x30   // clear any prior IRQs

#define GPIO1_IRQ 99  // TRG page 465 list the IRQs for the am335x


#define P9_12_pinConfig 0x878 //  conf_gpmc_ben1 (TRM pp 1364) for GPIO1_28,  P9_12

// GPMC_A1_Configuration
#define PIN_MODE_0   0x00
#define PIN_MODE_1   0x01
#define PIN_MODE_2   0x02
#define PIN_MODE_3   0x03
#define PIN_MODE_4   0x04
#define PIN_MODE_5   0x05
#define PIN_MODE_6   0x06
#define PIN_MODE_7   0x07

// PIN MUX Configuration strut values  (page 1420 from TRM)
#define PU_ENABLE    0x00
#define PU_DISABLE   0x01
#define PU_PULL_UP   0x01
#define PU_PULL_DOWN 0x00
#define RECV_ENABLE  0x01
#define RECV_DISABLE 0x00
#define SLEW_FAST    0x00
#define SLEW_SLOW    0x01


typedef union _CONF_MODULE_PIN_STRUCT   // See TRM Page 1420
{
  unsigned int d32;
  struct {    // name: field size
           unsigned int conf_mmode : 3;       // LSB
           unsigned int conf_puden : 1;
           unsigned int conf_putypesel : 1;
           unsigned int conf_rxactive : 1;
           unsigned int conf_slewctrl : 1;
           unsigned int conf_res_1 : 13;      // reserved
           unsigned int conf_res_2 : 12;      // reserved MSB
         } b;
} _CONF_MODULE_PIN;


// Mutex workerMutex isnt declared everywhere
// This can cause compile errors
#define Lock() { 									\
		do { 										\
			if (workerMutex != NULL) 				\
			{ 										\
				pthread_mutex_lock(workerMutex); 	\
			} 										\
		} while(0); 								\
	}


#define Unlock() { 									\
		do { 										\
			if (workerMutex != NULL) 				\
			{										\
				pthread_mutex_unlock(workerMutex);	\
			}										\
		} while(0); 								\
	}


#define GetkA(m, _kA, __kA) { 						\
		do {	 									\
			if (m != NULL) 							\
			{ 										\
				pthread_mutex_lock(m); 				\
				_kA = __kA; 						\
				pthread_mutex_unlock(m); 			\
			} 										\
		} while(0); 								\
	}

/*-----------------------------------------------------------------------------
* Local non-member Function Declarations
*---------------------------------------------------------------------------*/

void *mainWorkThread(void *appData);
const struct sigevent* Inthandler( void* area, int id );




/*-----------------------------------------------------------------------------
* Local Function Implementation
*---------------------------------------------------------------------------*/



/* ----------------------------------------------------	*
 *	@getMsgQueId keyPad constructer:		   			*
 *	@breif:  											*
 *	@return:                                 			*
 * ---------------------------------------------------	*/
keyPad::keyPad()
{
	kA = true;

	workerThread = (pthread_t *) malloc(sizeof(pthread_t));
	workerThreadAttr = (pthread_attr_t *) malloc(sizeof(pthread_attr_t));
	workerMutex = (pthread_mutex_t *) malloc(sizeof(pthread_mutex_t));

}



/* ----------------------------------------------------	*
 *	@getMsgQueId keyPad deconstructer:		    		*
 *	@breif:  											*
 *	@return:                                 			*
 * ---------------------------------------------------	*/
keyPad::~keyPad()
{
	Lock();
    kA = false;
    Unlock();


	pthread_join(*workerThread, NULL);

    delete(workerThread);
    delete(workerThreadAttr);
    delete(workerMutex);
}



/* ----------------------------------------------------	*
 *	@getMsgQueId keyPad deconstructer:		    		*
 *	@breif:  											*
 *	@return:                                 			*
 * ---------------------------------------------------	*/
void keyPad::start(pthread_attr_t *_threadAttr = NULL)
{
	// Do not need to lock because other thread isnt even running yet
	threadRun = true;

	if (_threadAttr != NULL)
	{ memcpy(workerThreadAttr, _threadAttr, sizeof(pthread_attr_t)); }

	// create and run thread
	pthread_create(workerThread, _threadAttr, mainWorkThread, NULL);
}



/* ----------------------------------------------------	*
 *	@getMsgQueId keyPad deconstructer:		    		*
 *	@breif:  											*
 *	@return:                         		        	*
 * ---------------------------------------------------	*/
void keyPad::stop()
{
	Lock();
	threadRun = false;
	Unlock();
}
  
	

/* ----------------------------------------------------	*
 *	@getMsgQueId registerCallback:						*
 *	@breif:	pass a callback function which gets 		*
 *			called and passes the key that was pressed	*
 *			void callBackFunction(char keyPressed)		*
 *														*
 *	@return: success or failure                       	*
 * ---------------------------------------------------	*/
bool keyPad::registerCallback(void (*_cb)(char))
{
	Lock();

	//struct _cbTable *tmpHead = NULL;

	if (cbTable == NULL)
	{
		cbTable = (_cbTable*)malloc(sizeof(_cbTable));

		// If malloc fails
		if (cbTable == NULL)
		{
			DEBUGF("Malloc failed to allocate memory:\n");
			Unlock();
			return false;  // return failure
		}

		// if a null pointer was given when calling the function
		if (_cb == NULL)
		{
			DEBUGF("The given cb was null:\n");
			Unlock();
			return false;  // return failure
		}

		DEBUGF("The cb was registered:\n");

		//cbTable->head = cbTable;
		cbTable->cb = _cb;

		Unlock();

		//return success
		return true;
	}

// Partial implementation of a link list for cb functions.
//	while(true) {
//		if (cbTable->next == NULL) {
//			cbTable = malloc(sizeof(_cbTable));
//			cbTable->next->head = cbTable;
//			cbTable->cb = _cb;
//		} else {
//			cbTable = cbTable->next;
//		}
//	}

	DEBUGF("A cb was already registered:\n");
	DEBUGF("Multiple cb table is not supported yet:\n");

	Unlock();

	// return failure
	return false;
}



/* ----------------------------------------------------	*
 *	@getMsgQueId deregisterCallback:					*
 *	@breif:  											*
 *	@return:         		                        	*
 * ---------------------------------------------------	*/
bool keyPad::deregisterCallback(void (*_cb)(char))
{
	Lock();

	if (cbTable != NULL)
	{
		// if a null pointer was given when calling the function
		if (_cb == NULL)
		{
			DEBUGF("The given cb was null:\n");
			Unlock();
			return false;  // return failure
		}

		// checking if the function given is the same as when registered
		if (_cb != cbTable->cb)
		{
			DEBUGF("The given cb didnt match the cb in the table:\n");
			Unlock();
			return false;  // return failure
		}

		// remove the function from the cb pointer
		cbTable->cb = NULL;

		DEBUGF("The cb was de-registered:\n");

		// free the memory
		free(cbTable);

		Unlock();

		// return success
		return true;
	}

	Unlock();

	// return failure
	return false;
}



/* ----------------------------------------------------	*
 *	@getMsgQueId KeypadReadIObit:				    	*
 *	@breif:  											*
 *	@return:         		                        	*
 * ---------------------------------------------------	*/
uint32_t keyPad::KeypadReadIObit(uintptr_t gpio_base, uint32_t BitsToRead)  {
   volatile uint32_t val = 0;
   val  = in32(gpio_base + GPIO_DATAIN);// value that is currently on the GPIO port

   val &= BitsToRead; // mask bit
   //val = val >> (BitsToRead % 2);
   //return val;
   if(val==BitsToRead)
	   return 1;
   else
	   return 0;
}



/* ----------------------------------------------------	*
 *	@getMsgQueId DecodeKeyValue:				    	*
 *	@breif:  											*
 *	@return:         		                        	*
 * ---------------------------------------------------	*/
void keyPad::DecodeKeyValue(uint32_t word)
{
	switch(word)
	{
		case 0x01:
			DEBUGF("Key  1 pressed:\n");
			break;
		case 0x02:
			DEBUGF("Key  2 pressed:\n");
			break;
		case 0x04:
			DEBUGF("Key  3 pressed:\n");
			break;
		case 0x08:
			DEBUGF("Key  4 pressed:\n");
			break;
		case 0x10:
			DEBUGF("Key  5 pressed:\n");
			break;
		case 0x20:
			DEBUGF("Key  6 pressed:\n");
			break;
		case 0x40:
			DEBUGF("Key  7 pressed:\n");
			break;
		case 0x80:
			DEBUGF("Key  8 pressed:\n");
			break;
		case 0x100:
			DEBUGF("Key  9 pressed:\n");
			break;
		case 0x200:
			DEBUGF("Key 10 pressed:\n");
			break;
		case 0x400:
			DEBUGF("Key 11 pressed:\n");
			break;
		case 0x800:
			DEBUGF("Key 12 pressed:\n");
			break;
		case 0x1000:
			DEBUGF("Key 13 pressed:\n");
			break;
		case 0x2000:
			DEBUGF("Key 14 pressed:\n");
			break;
		case 0x4000:
			DEBUGF("Key 15 pressed:\n");
			break;
		case 0x8000:
			DEBUGF("Key 16 pressed:\n");
			// TODO: is this needed?
			//usleep(1); // do this so we only fire once
			break;
		case 0x00:  // key release event (do nothing)
			break;
		default:
			DEBUGF("Could not decode key press:\n");
	}
}



/* ----------------------------------------------------	*
 *	@getMsgQueId strobe_SCL:					    	*
 *	@breif:  											*
 *	@return:         		                        	*
 * ---------------------------------------------------	*/
void keyPad::strobe_SCL(uintptr_t gpio_port_add) {
   uint32_t PortData;
   PortData = in32(gpio_port_add + GPIO_DATAOUT);// value that is currently on the GPIO port
   PortData &= ~(SCL);
   out32(gpio_port_add + GPIO_DATAOUT, PortData);// Clock low
   delaySCL();

   PortData  = in32(gpio_port_add + GPIO_DATAOUT);// get port value
   PortData |= SCL;// Clock high
   out32(gpio_port_add + GPIO_DATAOUT, PortData);
   delaySCL();
}



/* ----------------------------------------------------	*
 *	@getMsgQueId delaySCL:						    	*
 *	@breif:  											*
 *	@return:         		                        	*
 * ---------------------------------------------------	*/
void keyPad::delaySCL()  {// Small delay used to get timing correct for BBB
  volatile int i, a;
  for(i=0;i<0x1F;i++) // 0x1F results in a delay that sets F_SCL to ~480 kHz
  {   // i*1 is faster than i+1 (i+1 results in F_SCL ~454 kHz, whereas i*1 is the same as a=i)
     a = i;
  }
  // usleep(1);  //why doesn't this work? Ans: Results in a period of 4ms as
  // fastest time, which is 250Hz (This is to slow for the TTP229 chip as it
  // requires F_SCL to be between 1 kHz and 512 kHz)
}








/*-----------------------------------------------------------------------------
* Local non-member Function Implementation
*---------------------------------------------------------------------------*/


/* ----------------------------------------------------	*
 *	@getMsgQueId Inthandler:					    	*
 *	@breif:  											*
 *	@return:         		                        	*
 * ---------------------------------------------------	*/
const struct sigevent* Inthandler( void* area, int id )
{
	// Do not call any functions in ISR that call kernerl - including printf()
	ISR_data *p_ISR_data = (ISR_data *) area;

	InterruptMask(GPIO1_IRQ, id);  // Disable all hardware interrupt

	//clear IRQ or stack over flow and system will crash
	out32(p_ISR_data->gpio1_base + GPIO_IRQSTATUS_1, SD0);

	// counter for isr
	p_ISR_data->count_thread++;

	// got IRQ.
	// work out what it came from

    InterruptUnmask(GPIO1_IRQ, id);  // Enable a hardware interrupt

    // return a pointer to an event structure (preinitialized
    // by main) that contains SIGEV_INTR as its notification type.
    // This causes the InterruptWait in "int_thread" to unblock.
	return (&p_ISR_data->pevent);
}


/* ----------------------------------------------------	*
 *	@getMsgQueId mainWorkThread:						*
 *	@breif:  											*
 *	@return:         		                        	*
 * ---------------------------------------------------	*/
void* mainWorkThread(void *appData)
{

	bool tempkA = false;

	do
	{

		//GetkA(workerMutex,tempkA, kA);

	} while(tempkA);
    // creates interrupt for data ready from i2c keyboard...


    // wait while we wait for data, then call any callbacks that were registered.


	return 0; // What to return?
}





