
#include "../baseCom/Logging.h"

#include "TimerQueue.h"
#include "EventLoop.h"
#include "Timer.h"
#include "TimerId.h"

#include <functional>

#include <sys/timerfd.h>
#include <unistd.h>

namespace detail
{

int createTimeerfd()
{
  int timerfd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
  if (timerfd < 0)
  {
    LOG_SYSFATAL << "Failed int timerfd_create";
  }
  return timerfd;
}

struct timespec howMuchTimeFromNow(Timestamp when)
{
  int64_t microseconds = when.microSecondsSinceEpoch() - Timestamp::now().microSecondsSinceEpoch();
  if (microseconds < 100)
  {
    microseconds = 100;
  }
  struct timespec ts;
  ts.tv_sec = static_cast<time_t>(microseconds / Timestamp::KMicroSecondsPerSecond);
  ts.tv_nsec = static_cast<long>((microseconds % Timestamp::KMicroSecondsPerSecond) * 1000);
  return ts;
}

void readTimerfd(int timerfd, Timestamp now)
{
  uint64_t howmany;
  ssize_t n = ::read(timerfd, &howmany, sizeof howmany);
  LOG_TRACE << "TimerQueue::handleRead() " << howmany << " at " << now.toString();
  if (n != howmany)
  {
    LOG_ERROR << "TimerQueue::handleRead(0 reads " << n << " bytes instead of 8";
  }
}

void resetTimerfd(int timerfd, Timestamp expiration)
{
  //wake up loop by timerfd_settime()
  struct itimerspec newValue;
  struct itimerspec oldValue;
  bzero(&newValue, sizeof newValue);
  bzero(&oldValue, sizeof oldValue);
  newValue.it_value = howMuchTimeFromNow(expiration);
  int ret = ::timerfd_settime(timerfd, 0, &newValue, &oldValue);
  if (ret < 0)
  {
    LOG_ERROR << "timerfd_settime()";
  }
}

}

TimerQueue::TimerQueue(EventLoop* loop)
  : loop_(loop),
	  timerfd_(createTimeerfd()),
	  timerfdChannel_(loop, timerfd_),
	  timers_(),
    callingExpiredTimers_(false)
{
	timerfdChannel_.setReadCallback(std::bind(&TimerQueue::hanleRead, this));
	//we are always reading the timerfd, we disarm it with timerfd_settime
	timerfdChannel_.enableReading();
}

TimerQueue::~TimerQueue()
{
	timerfdChannel_.disableAll();
	timerfdChannel_.remove();
	::close(timerfd);
	//do not remove channel, since we are in EventLoop::dtor();
	for (auto it = timers_.begin(); it != timers_.end(); ++it)
	{
		delete it->second;
	}
}

TimerId TimerQueue::addTimer(const TimerCallback& cb, Timestamp when, double interval)
{
	Timer* timer = new Timer(cb, when, interval);
	loop->runInLoop(std::bind(&TimerQueue::addTimerInLoop, this, timer));
	return TimerId(timer, timer->sequence());
}

#ifdef __GXX_EXPERIMENTAL_CXX0X__
TimerId TimerQueue::addTimer(imerCallback&& cb, Timestamp when, double interval)
{
	Timer* timer = new Timer(std::move(cb), when, interval);
	loop->runInLoop(std::bind(&TimerQueue::addTimerInLoop, this, timer));
	return TimerId(timer, timer->sequence());
}
#endif

void TimerQueue::cancel(TimerId timerId)
{
	loop_->runInLoop(std::bind(&TimerQueue::cancelInLoop, this, timerId);
}

void TimerQueue::addTimerInLoop(Timer* timer)
{
  loop_->assertInLoopThread();
	bool earliestChanged = insert(timer);

	if (earliestChanged)
	{
	  resetTimerfd( timerfd_, timer->expiration());
	}
}

void TimerQueue::cancelInLoop(TimerId timerId)
{
  loop_->assertInLoopThread();
	assert(timers_.size() == activeTimers_size());
	ActiveTimer timer(TimerId.timer_, timerId.sequence_);
	auto it = activeTimers_.find(timer);
	if (it != activeTimers_.end())
	{
		size_t n = timers_.erase(Entry(it->first->expiration(), it->first));
		assert(n == 1);
		void(n);
		delete it->first; // FIXME: no delete please ?
		activeTimers_.erase(it);
	}
	else if (callingExpiredTimers_)
	{
		cancelingTimers_insert(timer);
	}
	assert(timers_.size() == activeTimers_.size());
}

void TimerQueue::handleRead()
{
	loop_->assertInLoopThread();
	Timestamp now(Timestam::now());
	readTimerfd(timerfd_, now);

	std::vector<Entry> expired = getExpired(now);
	callingExpiredTimers_ = true;
	cancelingTimers_.clear();
	//safe to callback outside critical section
	for (auto it = expired.begin(); it != expired.end(); ++it)
	{
		it->second->run();
	}
	callingExpiredTimers_ = false;

	reset(expired, now);
}

std::vector<Entry> TimerQueue::getExpired(Timestamp now)
{
	assert(timers_.size() == activeTimers_.size());
	std::vector<Entry> expired;
	Entry sentry(now, reinterpret_cast<Timer*>(UINPTR_MAX)); //compare time point
	auto end = timers_lower_bound(sentry);                  //return little than senty
	assert(end == timers_end() || now < end->first);
	std::copy(timers_.begin(), end, back_insert(expired));
	timers_.erase(timers.begin(), end);

	for (auto it = expired.begin(); it != expired.end(); ++it)
	{
		ActiveTimer timer(it->second, it->second->sequence());
		size_t n = activeTimers_.erase(timer);
		assert(n == 1);
		(void)n;
	}
	assert(timers_.size() == activeTimers_.size());
	return expired;
}

void TimerQueue::reset(const std::vector<Entry>& expired, Timestamp now)
{
	Timestamp nextExpire;
	for (auto it = expired.cbegin(); it != expired.end(); ++it)
	{
		ActiveTimer timer(it->second, it->second->sequence());
		if (it->second->repeat() && cancelingTimers_.find(timer) == cancelingTimers_.end())
		{
			it->second->restart(now);
			insert(it->second);
		}
		else
		{
			delete it->second; //FIXME move to a free list
		}
	}

	if (!timers_.empty())
	{
		nextExpire = timers_.begin()->second->expiration();
	}

	if (nextExpire.valid())
	{
		resetTimerfd(timerfd_,nextExpire);
	}
}

bool TimerQueue::insert(Timer* timer)
{
	loop_->assertInLoopThread();
	assert(tiemrs_.size() == activeTimers_.size());
	bool earliestChanged = false;
	Timestamp when = timer->expiration();
	auto it = timers_.begin();
	if (it == timers_.end() || when < it->first)
	{
		earilestChanged = true;
	}
	{
		auto result = timers_.insert(Entry(when, timer));
		assert(result.second);
		(void)result;
	}

	assert(timers_.size() == activeTimers_.size());
	return earliestChanged;
}

