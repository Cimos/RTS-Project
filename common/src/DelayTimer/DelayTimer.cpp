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
*/

/*-----------------------------------------------------------------------------
* Included Files
*---------------------------------------------------------------------------*/
#include "../DelayTimer/DelayTimer.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/netmgr.h>
#include <sys/neutrino.h>
#include <iostream>
#include <errno.h>


/*-----------------------------------------------------------------------------
* Definitions
*---------------------------------------------------------------------------*/

#define MY_PULSE_CODE   _PULSE_CODE_MINAVAIL

typedef union
{
	struct _pulse   pulse;
	// your other message structures would go here too
} my_message_t;

/*-----------------------------------------------------------------------------
* Global Variables and Buffers
*---------------------------------------------------------------------------*/

bool					repeat;
int 					timer_base;
long 					init_time;
int 					interval_base;
long 					interval_time;
struct sigevent         event;
struct itimerspec       itime;
timer_t                 timer_id;
int                     chid;
int                     rcvid;
my_message_t            msg;

/* ----------------------------------------------------	*
 *	@getMsgQueId DelayTimer constructor:		    	*
 *	@breif:  											*
 *	@return:                                 			*
 * ---------------------------------------------------	*/

DelayTimer::DelayTimer(bool Repeat, int Timer_base, int Init_time, int Interval_base, int Interval_time) {
	timer_base = Timer_base;
	init_time = Init_time;
	interval_base = Interval_base;
	interval_time = Interval_time;
	repeat = Repeat;
}

/* ----------------------------------------------------	*
 *	@getMsgQueId DelayTimer deconstructer:		    	*
 *	@breif:  											*
 *	@return:                                 			*
 * ---------------------------------------------------	*/
DelayTimer::~DelayTimer() {
	// TODO Auto-generated destructor stub
}

/* ----------------------------------------------------	*
 *	@getMsgQueId DelayTimer timerfunction:		    	*
 *	@breif:  											*
 *	@return:                                 			*
 * ---------------------------------------------------	*/
int DelayTimer::createTimer(){
	chid = ChannelCreate(0); // Create a communications channel

	event.sigev_notify = SIGEV_PULSE;

	// create a connection back to ourselves for the timer to send the pulse on
	event.sigev_coid = ConnectAttach(ND_LOCAL_NODE, 0, chid, _NTO_SIDE_CHANNEL, 0);
	if (event.sigev_coid == -1){
		std::cout << "Error: couldnt connect to self!" << std::endl;
		perror(NULL);
		exit(EXIT_FAILURE);
	}
	//event.sigev_priority = getprio(0);
	event.sigev_code = MY_PULSE_CODE;

	// create the timer, binding it to the event
	if (timer_create(CLOCK_REALTIME, &event, &timer_id) == -1){
		std::cout << "Error: couldn't create a timer!" << std::endl;
		perror(NULL);
		exit(EXIT_FAILURE);
	}

	// setup the timer - one shot
	if (timer_base == 0){
		itime.it_value.tv_sec = init_time;
	}
	else if (timer_base == 1){
		itime.it_value.tv_nsec = init_time;
	}
	else{
		std::cout << "Error: incorrect timer base. Use: 0 = sec, 1 = nsec!" << std::endl;
	}


	//timer interval - repeat
	if (repeat == true){
		if (interval_base == 0){
			itime.it_interval.tv_sec = interval_time;
		}
		else if (interval_base == 1){
			itime.it_interval.tv_nsec = interval_time;
		}
		else{
			std::cout << "Error: incorrect interval base. Use: 0 = sec, 1 = nsec!" << std::endl;
		}
	}

	// and start the timer!
	timer_settime(timer_id, 0, &itime, NULL);

	rcvid = MsgReceive(chid, &msg, sizeof(msg), NULL);

	return rcvid;
}

