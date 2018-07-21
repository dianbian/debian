
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

void Channel::tie(const std::shared_ptr<void>& obj)
{
	tie_ = obj;
	tied_ = true;
}

void Channel::update()
{
	addedToLoop_ = true;
	loop_->updateChannel(this);
}

void Channel::remove()
{
	assert(isNoneEvent());
	addedToLoop_ = false;
	loop_->removeChannel(this);
}

void Channel::handleEvent(Timestamp receiveTime)
{
	std::shared_ptr<void> guard;
	if (tied_)
	{
		guard = tie_.lock();
		if (guard)
			handleEventWithGuard(receiveTime);
	}
	else
	{
		handleEventWithGuard(receiveTime);
	}
}

void Channel::handleEventWithGuard(Timestamp receiveTime)
{
	eventHandling_ = true;
	LOG_TRACE << reventsToString();
	if ((revents_ & POLLHUP) && !(revents_ & POLLIN))  //data can read
	{
		if (logHup_)
			LOG_WARN << "fd = " << fd_ << " Channel::handleEvent() POLLHUP";
		if (closeCallback_)
			closeCallback_();
	}
	if (revents_ & POLLNVAL)  //fd description is not opening
	{
		LOG_WARN << "fd = " << fd << " Channel::handleEvent() POLLNVAL";
	}
	if (revents_ & (POLLERR | POLLNVAL))  //error | invalid
	{
		if (errorCallback_)
			errorCallback_();
	}
	if (revents_ & (POLLIN | POLLERR | POLLHUP))  //happen in the same time
	{
		if (readCallback_)
			readCallback_(receiveTime);       //call back is powerful
	}
	if (revents_ & POLLOUT)  //data can write
	{
		if (writeCallback_)
			writeCallback_();
	}
	eventHandling_ = false;
}

std::string Channel::reventsToString() const
{
	return evensToString(fd, revents_);
}

std::string Channel::evensToString() const
{
	return evensToString(fd, events_);
}

std::string Channel::evensToString(int fd, int ev)
{
	std::ostringstream oss;
	oss << fd << ": ";
	if (ev & POLLIN)
		oss << "IN ";
	if (ev & POLLPRI)
		oss << "PRI ";
	if (ev & POLLOUT)
		oss << "OUT ";
	if (ev & POLLHUP)
		oss << "HUP ";
	if (ev & POLLRDHUP)
		oss << "RDHUP ";
	if (ev & POLLERR)
		oss << "ERR ";
	if (ev & POLLNVAL)
		oss << "NVAL ";
	
	return oss.str().c_str();
}