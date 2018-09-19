
#ifndef BASECOM_TIMERQUEUE_H
#define BASECOM_TIMERQUEUE_H


#include "Mutex.h"
#include "Timestamp.h"
#include "noncopyable.h"

#include "Callbacks.h"
#include "Channel.h"

#include <vector>
#include <set>

class EventLoop;
class Timer;
class TimerId;

//A best efforts timer queue.       一个好的计时器队列
//No guarantee that the callback will be on time.  不能保证回调会准时执行
class TimerQueue : public noncopyable
{
	public:
	  explicit TimerQueue(EventLoop* loop);
		~TimerQueue();
		
		//Schedules the callback to be run at given time,  安排在给定时间运行回调
		//repeats if interval > 0.0
		//Muset be thread safe. Usually be called from other threads.
		TimerId addTimer(const TimerCallback& cb, Timestamp when, double interval);
#ifdef __GXX_EXPERIMENTAL_CXX0X__
    TimerId addTimer(TimerCallback&& cb, Timestamp when, double interval);
#endif
    void cancel(TimerId timerId);
	
	private:
	  //FIXME: use unique_ptr<Timer> instead of raw pointer.    使用只能指针代替原生指针
		//This requires heterogeneous comparsion lookup (N3465) from c++14  这需要异构比较查找
		//so that we can find an T* in a set<unique_ptr<T>>
		typedef std::pair<Timestamp, Timer*> Entry;
		typedef std::set<Entry> TimerList;
		typedef std::pair<Timer*, int64_t> ActiveTimer;
		typedef std::set<ActiveTimer> ActiveTimerSet;
		
		void addTimerInLoop(Timer* timer);
		void cancelInLoop(TimerId timerId);
		//called when timerId alarms
		void handleRead();
		//move out all expired timers
		std::vector<Entry> getExpired(Timestamp now);
		void reset(const std::vector<Entry>& expired, Timestamp now);
		bool insert(Timer* timer);
		
		EventLoop* loop_;
		const int timerfd_;
		Channel timerfdChannel_;
		//Tiemr list sorted by expiration
		TimerList timers_;
		
		//for cancel()
		ActiveTimerSet activeTimers_;
		bool callingExpiredTimers_;   //atomic
		ActiveTimerSet cancelingTimers_;
};


#endif //BASECOM_TIMERQUEUE_H
