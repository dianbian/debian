
#include "../baseCom/Logging.h"
#include "Channel.h"
#include "EventLoop.h"

#include <sstream>

#include <poll.h>

const int Channel::KNoneEvent = 0;
const int Channel::kReadEvent = POLLIN | POLLPRI;
const int Channel::kWritEvent = POLLOUT;

Channel::Channel(EventLoop* loop, int fd__) 
    : loop_(loop), 
	  fd_(fd__), 
	  events_(0),
      revents_(0), 
	  index_(-1), 
	  logHup_(true), 
	  tied_(false), 
	  eventHandling_(false), 
	  addedToLoop_(false)
{
	
}

Channel::~Channel()
{
	assert(!eventHandling_);
	assert(!addedToLoop_);
	if (loop_->isInLoopThread())
	{
		assert(!loop_->hasChannel(this));
	}
}