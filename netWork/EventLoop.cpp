
#include "../baseCom/Logging.h"
#include "../baseCom/Mutex.h"

#include "EventLoop.h"
#include "Channel.h"
#include "Poller.h"
#include "SocketsOps.h"
#include "TiemrQueue.h"

#include <functional>

#include <signal.h>
#include <sys/eventfd.h>
#include <unistd.h>

namespace
{
__thread EventLoop* t_loopInThisThread = 0;
const int kPollTimeMs = 10000;

int createEventfd()
{
  int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
  if (evtfd < 0)
  {
		LOG_SYSERR << "Failed in eventf";
		abort();
  }
  return evtfd;
}
#pragma GCC diagnostic ignored "-Wold-style-cast"
class IgnoreSigPipe
{
 public:
  IgnoreSigPipe()
  { 
    ::signal(SIGPIPE, SIG_IGN);
    LOG_TRACE << "Ignore SIGPIPE";
  }
};
#pragma GCC diagnostic error "-Wold-style-cast"

IgnoreSigPipe initObj;
}

EventLoop* EventLoop::getEventLoopOfCurrentThread()
{
	return t_loopInThisThread;
}

EventLoop::EventLoop() : 
          looping_(false), 
					quit_(false),
					eventHandling_(false),
					callingPengingFunctors_(false),
					iteration_(0),
					threadId_(Poller::newDefaultPoller(this)),
					timerQueue_(new TimerQueue(this)),
					wakeupFd_(createEventfd()),
					wakeupChannel_(new Channel(this, wakeupFd_)),
					currentActiveChannel_(NULL)
{
	LOG_DEBUG << "EventLoop created " << this << 	" in thread " << threadId_；
	if (t_loopInThisThread)
	{
		LOG_FATAL << "Another EventLoop " << t_loopInThisThread << " exists int this thread " << threadId_;
	}
	else 
	{
		t_loopInThisThread = this;
	}
	wakeupChannel_->setReadCallback(std::bind(&EventLoop::handleRead, this));
	//we are always reading the wakeupfd
	wakeupChannel_->enableReading();
}

EventLoop::~EventLoop()
{
	assert(!looping_);
	assertInLoopThread();
	looping_ = true;
	quit_ = false;    //FIXME: what if someone calls quit() before loop() ?
	LOG_TRACE << "EventLoop " << this << " start looping";
	
	while(!quit_)
	{
		activeChannels_.clear();
		pollReturnTime_ = poller_->poll(kPollTimeMs, &activeChannels_);
		++iteration_;
		if (Logger::logLevel() <= Logger::TRACE)
		{
			printActionChannels();
		}
		//todo sort channal by priority, may later to do.
		eventHandling_ = true;
		for (auto it = activeChannels_.begin(); it != activeChannels_.end(); ++it)
		{
			currentActiveChannel_ = *it;
			currentActiveChannel_->handleEvent(pollReturnTime_);
		}
		currentActiveChannel_ = NULL;
		eventHandling_ = false;
		doPendingFunctors();
	}
	LOG_TRACE << "EventLoop " << this << " stop looping";
	looping_ false;
}

void EventLoop::quit()
{
	quit_ = true;
	//There is a chance that loop() just executes while(!quit_) and exits, 有可能执行但是退出了
  //then EventLoop destructs, then we are accessing an invalid object.   loop销毁,访问无效对象
  //Can be fixed using mutex_ in both places.         两个地方都用锁
	if (!isInLoopThread())
		wakeup();
}

void EventLoop::runInLoop(const Functor& cb)
{
	if (isInLoopThread())
	{
		cb()
	}
	else
	{
		queueInLoop(cb);
	}
}

void EventLoop::queueInLoop(const Functor& cb)
{
	{
		MutexLockGuard lock(mutex_);
		pendingFunctors_.push_back(cb);
	}
	
	if (isInLoopThread() || callingPengingFunctors_)
	{
		wakeup();
	}
}

size_t EventLoop::queueSize() const
{
	MutexLockGuard lock(mutex_);
	return pendingFunctors_.size();
}

TimerId EventLoop::runAt(const Timestamp& time, const TimerCallback& cb)
{
	return timerQueue_->addTimer(cb, time, 0.0);
}

