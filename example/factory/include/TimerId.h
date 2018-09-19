
#ifndef BASECOM_TIMERID_H
#define BASECOM_TIMERID_H

#include "copyable.h"

#include <stdint.h>
#include <stdio.h>

class Timer;

//An opaque indentifier, for canceling Timer.
//blur Timer varibale, may for destructing someone.

class TimerId : public copyable
{
 public:
  TimerId() : timer_(NULL), sequence_(0)
	{
		
	}
    
	TimerId(Timer* timer, int64_t seq)
	    : timer_(timer), sequence_(seq)
	{
		
	}
	
	//default copy-actor, dtor(destructor) and assignmeng are ok
	
	friend class TimerQueue;
	
 private:
  Timer* timer_;
	int64_t sequence_;
};

#endif //BASECOM_TIMERID_H
