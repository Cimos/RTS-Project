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
*----------------------------------------------------------------------------*/


#include "keyPad.h"
#include "../gpio/boneGpio.h"
#include <hw/inout.h>      // for in32() and out32();
#include <sys/mman.h>      // for mmap_device_io();
#include <sys/neutrino.h>  // for ThreadCtl( _NTO_TCTL_IO_PRIV , NULL)
#include <sched.h>
#include <sys/procmgr.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
/*-----------------------------------------------------------------------------
* Definitions
*----------------------------------------------------------------------------*/
//#define DEBUGF printf("Debug->");print
#define DEBUGF//

#define AM335X_CONTROL_MODULE_BASE   (uint64_t) 0x44E10000
#define AM335X_CONTROL_MODULE_SIZE   (size_t)   0x00001448
#define AM335X_GPIO_SIZE             (uint64_t) 0x00001000
#define AM335X_GPIO1_BASE            (size_t)   0x4804C000


#define SD0 (1<<28)  // SD0 is connected to GPIO1_28		pin 12
#define SCL (1<<17)  // SCL is connected to GPIO1_17		pin 23


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

// Mutex workerMutex isnt declared everywhere
// This can cause compile errors
#define Unlock() { 									\
		do { 										\
			if (workerMutex != NULL) 				\
			{										\
				pthread_mutex_unlock(workerMutex);	\
			}										\
		} while(0); 								\
	}


#define GetkA(m, tmpkA, _kA) { 						\
		do {	 									\
			if (m != NULL) 							\
			{ 										\
				pthread_mutex_lock(m); 				\
				tmpkA = _kA; 						\
				pthread_mutex_unlock(m); 			\
			} 										\
		} while(0); 								\
	}


#define GetTheadRun(m, tmpTheadRun, _threadRun) { 	\
		do {	 									\
			if (m != NULL) 							\
			{ 										\
				pthread_mutex_lock(m); 				\
				tmpTheadRun = _threadRun; 			\
				pthread_mutex_unlock(m); 			\
			} 										\
		} while(0); 								\
	}

/*-----------------------------------------------------------------------------
* Global Variables and Buffers
*---------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------
* Local non-member Function Declarations
*----------------------------------------------------------------------------*/

const struct sigevent* Inthandler( void* area, int id );
uint32_t KeypadReadIObit(uintptr_t gpio_base, uint32_t BitsToRead);
char DecodeKeyValue(uint32_t word);
void strobe_SCL(uintptr_t gpio_port_add);
void delaySCL(void);




/*-----------------------------------------------------------------------------
* Local Function Implementation
*----------------------------------------------------------------------------*/



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
	
	// TODO: Add a default timeout
	//ISR_data.timeout.sec = 1;

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
void keyPad::start(pthread_attr_t *_threadAttr)
{
	// Do not need to lock because other thread isnt even running yet
	kA = true;

	if (_threadAttr != NULL)
	{ memcpy(workerThreadAttr, _threadAttr, sizeof(pthread_attr_t)); }


	// create and run thread
	ThreadCtl(_NTO_TCTL_IO, 0);
	pthread_create(workerThread, _threadAttr, mainWorkThread, (void*)this);
	usleep(1);

}



/* ----------------------------------------------------	*
 *	@getMsgQueId keyPad deconstructer:		    		*
 *	@breif:  											*
 *	@return:                         		        	*
 * ---------------------------------------------------	*/
void keyPad::stop()
{
	Lock();
	kA = false;
	Unlock();

	pthread_join(*workerThread, NULL);

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

	//struct _cbTable *tmpHead = NULL;

	if (cbTable == NULL)
	{
		// Only need to lock variables when we actualy use them
		// No need to lock the variable when just read
		Lock();
		cbTable = (_cbTable*)malloc(sizeof(_cbTable));
		Unlock();

		// If malloc fails
		if (cbTable == NULL)
		{
			//DEBUGF("Malloc failed to allocate memory:\n");
			return false;  // return failure
		}

		// if a null pointer was given when calling the function
		if (_cb == NULL)
		{
			//DEBUGF("The given cb was null:\n");
			return false;  // return failure
		}

		//DEBUGF("The cb was registered:\n");

		//cbTable->head = cbTable;
		Lock();
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

	//DEBUGF("A cb was already registered:\n");
	//DEBUGF("Multiple cb table is not supported yet:\n");


	// return failure
	return false;
}


/* ----------------------------------------------------	*
 *	@getMsgQueId getCallback:						*
 *	@breif:	pass a callback function which gets 		*
 *			called and passes the key that was pressed	*
 *			void callBackFunction(char keyPressed)		*
 *														*
 *	@return: success or failure                       	*
 * ---------------------------------------------------	*/
_cbTable* keyPad::getCallback(void)
{

	//struct _cbTable *tmpHead = NULL;

	if (cbTable != NULL)
	{
		return cbTable;
	}

	return NULL;
}



/* ----------------------------------------------------	*
 *	@getMsgQueId deregisterCallback:					*
 *	@breif:  											*
 *	@return:         		                        	*
 * ---------------------------------------------------	*/
bool keyPad::deregisterCallback(void (*_cb)(char))
{

	if (cbTable != NULL)
	{

		// if a null pointer was given when calling the function
		if (_cb == NULL)
		{
			//DEBUGF("The given cb was null:\n");
			return false;  // return failure
		}

		// checking if the function given is the same as when registered
		if (_cb != cbTable->cb)
		{
			//DEBUGF("The given cb didnt match the cb in the table:\n");
			return false;  // return failure
		}

		Lock();
		cbTable->cb = NULL;	// remove the function from the cb pointer
		//DEBUGF("The cb was de-registered:\n");
		free(cbTable);	// free the memory
		Unlock();

		// return success
		return true;
	}

	// return failure
	return false;
}






/*-----------------------------------------------------------------------------
* Local non-member Function Implementation
*----------------------------------------------------------------------------*/



/* ----------------------------------------------------	*
 *	@getMsgQueId KeypadReadIObit:				    	*
 *	@breif:  											*
 *	@return:         		                        	*
 * ---------------------------------------------------	*/
uint32_t KeypadReadIObit(uintptr_t gpio_base, uint32_t BitsToRead)  {
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
char DecodeKeyValue(uint32_t word)
{
	char ret = 0;
	switch(word)
	{
		case 0x01:
			//DEBUGF("Key 1 pressed:\n");
			ret = '1';
			break;
		case 0x02:
			//DEBUGF("Key 2 pressed:\n");
			ret = '2';
			break;
		case 0x04:
			//DEBUGF("Key 3 pressed:\n");
			ret = '3';
			break;
		case 0x08:
			//DEBUGF("Key 4 pressed:\n");
			ret = '4';
			break;
		case 0x10:
			//DEBUGF("Key 5 pressed:\n");
			ret = '5';
			break;
		case 0x20:
			//DEBUGF("Key 6 pressed:\n");
			ret = '6';
			break;
		case 0x40:
			//DEBUGF("Key 7 pressed:\n");
			ret = '7';
			break;
		case 0x80:
			//DEBUGF("Key 8 pressed:\n");
			ret = '8';
			break;
		case 0x100:
			//DEBUGF("Key 9 pressed:\n");
			ret = '9';
			break;
		case 0x200:
			//DEBUGF("Key 10 pressed:\n");
			ret = 'A';
			break;
		case 0x400:
			//DEBUGF("Key 11 pressed:\n");
			ret = 'B';
			break;
		case 0x800:
			//DEBUGF("Key 12 pressed:\n");
			ret = 'C';
			break;
		case 0x1000:
			//DEBUGF("Key 13 pressed:\n");
			ret = 'D';
			break;
		case 0x2000:
			//DEBUGF("Key 14 pressed:\n");
			ret = 'E';
			break;
		case 0x4000:
			//DEBUGF("Key 15 pressed:\n");
			ret = 'F';
			break;
		case 0x8000:
			//DEBUGF("Key 16 pressed:\n");
			ret = 'G';
			break;
		case 0x00:  // key release event (do nothing)
		default:
			//DEBUGF("Could not decode key press:\n");
			ret = 0;
	}
	return ret;
}


/* ----------------------------------------------------	*
 *	@getMsgQueId strobe_SCL:					    	*
 *	@breif:  											*
 *	@return:         		                        	*
 * ---------------------------------------------------	*/
void strobe_SCL(uintptr_t gpio_port_add) {
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
 *	@breif: Small delay used to get timing 				*
 			correct for BBB								*
 *	@return:         		                        	*
 * ---------------------------------------------------	*/
void delaySCL(void)  {// Small delay used to get timing correct for BBB
  volatile int i, a;
  for(i=0;i<0x1F;i++) // 0x1F results in a delay that sets F_SCL to ~480 kHz
  {   // i*1 is faster than i+1 (i+1 results in F_SCL ~454 kHz, whereas i*1 is the same as a=i)
     a = i;
  }
  // usleep(1);  //why doesn't this work? Ans: Results in a period of 4ms as
  // fastest time, which is 250Hz (This is to slow for the TTP229 chip as it
  // requires F_SCL to be between 1 kHz and 512 kHz)
}



/* ----------------------------------------------------	*
 *	@getMsgQueId Inthandler:					    	*
 *	@breif:  											*
 *	@return:         		                        	*
 * ---------------------------------------------------	*/
 ISR_data ISR_area_data;

const struct sigevent* Inthandler( void* area, int id )
{
	// 	"Do not call any functions in ISR that call kernerl - including printf()
	//struct sigevent *pevent = (struct sigevent *) area;
	ISR_data *p_ISR_data = (ISR_data *) area;

	InterruptMask(GPIO1_IRQ, id);  // Disable all hardware interrupt

	// must do this in the ISR  (else stack over flow and system will crash
	out32(p_ISR_data->gpio1_base + GPIO_IRQSTATUS_1, SD0); //clear IRQ

	// do this to tell us how many times this handler gets called
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
void* keyPad::mainWorkThread(void *appData)
{
	_cbTable *cbTable = NULL;
	keyPad *KeyPad = (keyPad *) appData;
	bool tempkA = true;
	bool tempRun = true;
	int i = 0;
    int id = 0; // Attach interrupt Event to IRQ for GPIO1B  (upper 16 bits of port)
	char key = 0;


    volatile uint32_t val = 0;
	volatile uint32_t word = 0;
	uint64_t timeoutSetTime = 1000000000;
	uint64_t timeoutRemTime = 0;
	struct sigevent timerEvent;


	if (KeyPad != NULL)
	{
		cbTable = KeyPad->getCallback();
	}


	uintptr_t control_module = mmap_device_io(AM335X_CONTROL_MODULE_SIZE,AM335X_CONTROL_MODULE_BASE);
	uintptr_t gpio1_base = mmap_device_io(AM335X_GPIO_SIZE, AM335X_GPIO1_BASE);


	// initalise the global stuct
	ISR_area_data.count_thread = 0;
	ISR_area_data.gpio1_base = gpio1_base;


	memset(&ISR_area_data.pevent, 0, sizeof(ISR_area_data.pevent));
	SIGEV_INTR_INIT (&ISR_area_data.pevent);
	ISR_area_data.pevent.sigev_notify = SIGEV_INTR;  // Setup for external interrupt




	if( (control_module)&&(gpio1_base) )
	{
		ThreadCtl( _NTO_TCTL_IO_PRIV , NULL);// Request I/O privileges;


		// set DDR for LEDs to output and GPIO_28 to input
		val = in32(gpio1_base + GPIO_OE); // read in current setup for GPIO1 port
		val |= 1<<28;                     // set IO_BIT_28 high (1=input, 0=output)
		out32(gpio1_base + GPIO_OE, val); // write value to input enable for data pins
		val &= ~(LED0|LED1|LED2|LED3);    // write value to output enable
		out32(gpio1_base + GPIO_OE, val); // write value to output enable for LED pins

		val = in32(gpio1_base + GPIO_OE);
		val &= ~SCL;                      // 0 for output
		out32(gpio1_base + GPIO_OE, val); // write value to output enable for data pins


		val = in32(gpio1_base + GPIO_DATAOUT);
		val |= SCL;              // Set Clock Line High as per TTP229-BSF datasheet
		out32(gpio1_base + GPIO_DATAOUT, val); // for 16-Key active-Low timing diagram

		in32s((void*)&val, 1, control_module + P9_12_pinConfig );
		//DEBUGF("Original pinmux configuration for GPIO1_28 = %#010x\n", val);

		// set up pin mux for the pins we are going to use  (see page 1354 of TRM)
		volatile _CONF_MODULE_PIN pinConfigGPMC; // Pin configuration strut
		pinConfigGPMC.d32 = 0;
		// Pin MUX register default setup for input (GPIO input, disable pull up/down - Mode 7)
		pinConfigGPMC.b.conf_slewctrl = SLEW_SLOW;    // Select between faster or slower slew rate
		pinConfigGPMC.b.conf_rxactive = RECV_ENABLE;  // Input enable value for the PAD
		pinConfigGPMC.b.conf_putypesel= PU_PULL_UP;   // Pad pullup/pulldown type selection
		pinConfigGPMC.b.conf_puden = PU_ENABLE;       // Pad pullup/pulldown enable
		pinConfigGPMC.b.conf_mmode = PIN_MODE_7;      // Pad functional signal mux select 0 - 7

		// Write to PinMux registers for the GPIO1_28
		out32(control_module + P9_12_pinConfig, pinConfigGPMC.d32);
		in32s((void*)&val, 1, control_module + P9_12_pinConfig);   // Read it back
		//DEBUGF("New configuration register for GPIO1_28 = %#010x\n", val);

		// Setup IRQ for SD0 pin ( see TRM page 4871 for register list)
		out32(gpio1_base + GPIO_IRQSTATUS_SET_1, SD0);// Write 1 to GPIO_IRQSTATUS_SET_1
		out32(gpio1_base + GPIO_IRQWAKEN_1, SD0);    // Write 1 to GPIO_IRQWAKEN_1
		out32(gpio1_base + GPIO_FALLINGDETECT, SD0);    // set falling edge
		out32(gpio1_base + GPIO_CLEARDATAOUT, SD0);     // clear GPIO_CLEARDATAOUT
		out32(gpio1_base + GPIO_IRQSTATUS_1, SD0);      // clear any prior IRQs


	    int errvalue = 0;
	    errno = EOK;

	    struct sigevent event; // fill in "event" structure
	    memset(&event, 0, sizeof(event));
	    event.sigev_notify= SIGEV_INTR;  // Setup for external interrupt

	    id = InterruptAttachEvent(GPIO1_IRQ, &event, _NTO_INTR_FLAGS_TRK_MSK);// Main code starts here
	    errvalue = errno;

	    //DEBUGF( "The error generated was %d\n", errvalue );
	    //DEBUGF( "That means: %s\n", strerror( errvalue ) );

	    InterruptUnmask(GPIO1_IRQ, id);  // Enable a hardware interrupt
	}




	do
	{
	
	// Get Keep Alive variable
	GetkA(KeyPad->workerMutex, tempkA, KeyPad->kA);


	// TODO: Check if re-assigning their values are nedded
	SIGEV_INTR_INIT(&timerEvent);
	timeoutSetTime = 1000000000;   // One seconds in nano-secs
	timeoutRemTime = 0;

	// Timeout
   	TimerTimeout_r(CLOCK_REALTIME, _NTO_TIMEOUT_INTR, &timerEvent, &timeoutSetTime, &timeoutRemTime);
	InterruptWait(0, NULL);
	InterruptDisable();

	//  confirm that SD0 is still low (that is a valid Key press event has occurred)
	val = KeypadReadIObit(gpio1_base, SD0);  // read SD0 (means data is ready)

	if(val == 0)  // start reading key value form the keypad
	{
		word = 0;  // clear word variable

		delaySCL(); // wait a short period of time before reading the data Tw  (10 us)

		for(i=0;i<16;i++)           // get data from SD0 (16 bits)
		{
		strobe_SCL(gpio1_base);  // strobe the SCL line so we can read in data bit

		val = KeypadReadIObit(gpio1_base, SD0); // read in data bit
		val = ~val & 0x01;                      // invert bit and mask out everything but the LSB
		DEBUGF("keyPad->val[%u]=%u, ",i, val);
		word = word | (val<<i);  // add data bit to word in unique position (build word up bit by bit)
		}
		DEBUGF("word=%u\n",word);
		key = DecodeKeyValue(word);
		if (cbTable->cb != NULL)
		{
			if (key != 0)
				cbTable->cb(key);
		}
	}

		out32(gpio1_base + GPIO_IRQSTATUS_1, SD0); //clear IRQ
		InterruptUnmask(GPIO1_IRQ, id);
		InterruptEnable();

	} while(tempkA);


	munmap_device_io(gpio1_base, AM335X_GPIO_SIZE);
    munmap_device_io(control_module, AM335X_CONTROL_MODULE_SIZE);

    DEBUGF("keyPad->Thread Dead\n");
	return 0; // What to return?
}



//
//int i = 0;
//
//for(;;)   // for loop that correctly decodes key press
//{
//	InterruptWait( 0, NULL );   // block this thread until an interrupt occurs
//	InterruptDisable();
//
//	//  confirm that SD0 is still low (that is a valid Key press event has occurred)
//	val = KeypadReadIObit(gpio1_base, SD0);  // read SD0 (means data is ready)
//	volatile uint32_t word = 0;
//
//	if(val == 0)  // start reading key value form the keypad
//	{
//		word = 0;  // clear word variable
//
//		delaySCL(); // wait a short period of time before reading the data Tw  (10 us)
//
//		for(i=0;i<16;i++)           // get data from SD0 (16 bits)
//		{
//			strobe_SCL(gpio1_base);  // strobe the SCL line so we can read in data bit
//
//			val = KeypadReadIObit(gpio1_base, SD0); // read in data bit
//			val = ~val & 0x01;                      // invert bit and mask out everything but the LSB
			//DEBUGF("keyPad->val[%u]=%u, ",i, val);
//			word = word | (val<<i);  // add data bit to word in unique position (build word up bit by bit)
//		}
//		//DEBUGF("keyPad->word=%u\n",word);
//		DecodeKeyValue(word);
//	}
//	out32(gpio1_base + GPIO_IRQSTATUS_1, SD0); //clear IRQ
//	InterruptUnmask(GPIO1_IRQ, id);
//	InterruptEnable();
//}


