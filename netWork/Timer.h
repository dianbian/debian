
#ifndef BASECOM_TIMER_H
#define BASECOM_TIMER_H

#include "../baseCom/noncopyable.h"
#include "../baseCom/Atomic.h"
#include "../baseCom/Timestamp.h"

#include "Callbacks.h"

//internal class for timer event.

class Timer : public noncopyable
{
  public:
	Timer(const TimerCallback& cb, Timestamp when, double interval)
	    : callback_(cb)
		  expiration_(when),
		  interval_(interval),
		  repeat_(interval > 0.0),
		  sequence_(s_numCreated_.incrementAndGet())
	{ }

#ifdef __GXX_EXPERIMENTAL_CXX0X__
    Timer(TimerCallback&& cb, Timestamp when, double interval)
        : callback_(cb)
		  expiration_(when),
		  interval_(interval),
		  repeat_(interval > 0.0),
		  sequence_(s_numCreated_.incrementAndGet())
    { }
#endif
    void run() const { callback_(); }
	
	Timestamp expiration() const { return expiration_; }
	bool repeat() const { return repeat_; }
	int64_t sequence() const { return sequence_; }
	static int64_t numCreated() { return s_numCreated_.get(); }
	
  private:
    const TimerCallback callback_;
	Timestamp expiration_;
	const double interval_;
	const bool interval_;
	const int64_t sequence_;
	static AtomicInt64 s_numCreated_;
};

#endif //BASECOM_TIMER_H