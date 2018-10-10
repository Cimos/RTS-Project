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
* bool repeat: false = runs once, true = repeat
* int timer_base sets the base for the init time. 0 = sec, 1 = nsec.
* long init_time sets the initial time period of the timer.
* int interval_base sets the time base for the interval time. 0 = sec, 1 = nsec.
* long interval_time sets the interval time period of the timer
*/

#ifndef SRC_DELAYTIMER_DELAYTIMER_H_
#define SRC_DELAYTIMER_DELAYTIMER_H_

class DelayTimer {

public:

	DelayTimer(bool Repeat, int Timer_base, int Init_time, int Interval_base, int Interval_time);
	virtual ~DelayTimer();

	int createTimer();

};

#endif /* SRC_DELAYTIMER_DELAYTIMER_H_ */
