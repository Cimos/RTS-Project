/******************************************************************************
*
* RTS Traffic Light Project 2018
*
*****************************************************************************/
/**
* @file     DelayTimer.h
* @brief    RTS Traffic Light Project
* @author   Michael Stekla	s3545953
* 			Simon Maddison	s3493550
* 			Shawn Buschmann	s3478646
*
* Timer class that can be used as a delay.
* Call constructor with the follwoing variables:
* bool repeat: false = runs once, true = repeat
* int timer_base sets the base for the init time. 0 = sec, 1 = nsec.
* long init_time sets the initial time period of the timer.
* int interval_base sets the time base for the interval time. 0 = sec, 1 = nsec.
* long interval_time sets the interval time period of the timer

  //To use timer as 1 shot example for 1 sec:
  //Setup
  DelayTimer myTimer(false, 0 , 1, 0 , 0);
  //use timer where you want 1 sec delay and return a pulse.
  myTimer.createTimer();

  // note: if you want to return something other than a pulse,
           place into the 'my_message_t' struct below.
*/

#ifndef SRC_DELAYTIMER_DELAYTIMER_H_
#define SRC_DELAYTIMER_DELAYTIMER_H_



#include <sys/netmgr.h>
#include <sys/neutrino.h>
#include <unistd.h>

/*-----------------------------------------------------------------------------
* Definitions
*---------------------------------------------------------------------------*/

#define MY_PULSE_CODE   _PULSE_CODE_MINAVAIL


typedef union
{
	struct _pulse   pulse;
	// your other message structures would go here too
} my_message_t;



class DelayTimer 
{
private:

	bool					repeat = false;
	int 					timer_base = 0;
	long 					init_time = 0;
	int 					interval_base = 0;
	long 					interval_time = 0;
	struct sigevent         event = {};
	struct itimerspec       itime = {};
	timer_t                 timer_id = {};
	int                     chid = 0;
	int                     rcvid = 0;
	my_message_t            msg = {};


public:

	//DelayTimer() {}
	DelayTimer(bool Repeat, int Timer_base, long Init_time, int Interval_base, long Interval_time);
	virtual ~DelayTimer();

	int createTimer();

};

#endif /* SRC_DELAYTIMER_DELAYTIMER_H_ */
