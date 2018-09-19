#ifndef NETWORK_EVENTLOOPTHREAD_H
#define NETWORK_EVENTLOOPTHREAD_H

#include "../baseCom/Condition.h"
#include "../baseCom/Mutex.h"
#include "../baseCom/Thread.h"
#include "../baseCom/noncopyable.h"

#include <functional>

class EventLoop;

class EventLoopThread : public noncopyable
{
 public:
  typedef std::function<void(EventLoop*)> ThreadInitCallback;
  
  EventLoopThread(const ThreadInitCallback& cb = ThreadInitCallback(), 
                const std::string& name = std::string());
  
  ~EventLoopThread();
  EventLoop* startLoop();
  
 private:
  void threadFunc();
  
  EventLoop* loop_;
  bool exiting_;
  Thread thread_;
  MutexLock mutex_;
  Condition cond_;
  ThreadInitCallback callback_;
};

#endif
