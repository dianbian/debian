
#ifndef NETWORK_EVENTLOOP_H
#define NETWORK_EVENTLOOP_H

#include "../baseCom/Mutex.h"
#include "../baseCom/CurrentThread.h"
#include "../baseCom/Timestamp.h"
#include "../baseCom/noncopyable.h"

#include "Callbacks.h"
#include "TimerId.h"

#include <functional>
#include <memory>

class Channel;
class Poller;
class TimeQueue;

//Reactor, at most one per thread.
//This is an interface class, do not expose too much details.
class EventLoop : public noncopyable
{
  public:
    typedef functional<void()> Functor;
	
	EventLoop();
	~EventLoop();   //force out-line dtor, for scoped_str members.
	
	//Loops forever.
	//Must be called in the same thread as creation of the object.
	void loop();
	
	//Quits loop, this is not 100% thread safe, if you call through a raw pointer,
	//better to call through shared_ptr<EventLoop> for 100% safety.
	void quit();
	
	//Time when poll returns, usually means data arrival.   轮询返回的时间，通常意味着数据到达
	Timestamp pollReturnTime() const { return pollReturnTime_; }
	
	int64_t iteration() const { return iteration_; }
	
	//Runs callback immediately in the loop thread.    在循环线程中立即运行回调
	//It wakes up the loop, and run the cb.            它唤醒循环并运行CB
	//If in the same loop thread, cb is run within the function. 如果在同一个循环线程中，CB在函数内运行
	//Safe to call from other threads.                           安全地从其他线程调用
	void runInLoop(const Functor& cb);
	//Queuses callback in the loop thread.          循环线程中的队列回调
	//Runs after finish pooling.                    完成池后运行
	//Safe to call from other threads.              安全地从其他线程调用
	void queueInLoop(const Functor& cb);
	size_t queueSize() const;

#ifdef __GXX_EXPERIMENTAL_CXX0X__
    void runInLoop(Functor&& cb);
	void queueInLoop(Functor&& cb);
#endif
    
	//Timers
	//Runs callback at 'time'.
	//Safe to call from other threads.
	TimeId runAt(const Timestamp& time, const TimerCallback& cb);
	//Runs callback after delay seconds.
	//Safe to call from other threads.
	TimeId runAfter(double delay, const TimerCallback& cb);
	//Runs callback every interval seconds.
	//Safe to call from other threads.
	TimeId runEvery(double interval, const TimerCallback& cb);
	//Cancels the Timer.
	//Safe to call from other threads.
	void cancel(TimeId timerId);

#ifdef __GXX_EXPERIMENTAL_CXX0X__
    TimeId runAt(const Timestamp& time, TimerCallback&& cb);
	TimeId runAfter(double delay, TimerCallback&& cb);
	TimeId runEvery(double interval, TimerCallback&& cb);
#endif

    //internal usage
	void wakeup();
	void updateChannel(Channel* channel);
};

#endif //NETWORK_EVENTLOOP_H
