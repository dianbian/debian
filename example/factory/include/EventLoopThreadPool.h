
#ifndef NETWORK_EVENTLOOPTHREADPOOL_H
#define NETWORK_EVENTLOOPTHREADPOOL_H

#include "noncopyable.h"

#include <vector>
#include <functional>
#include <memory>

class EventLoop;
class EventLoopThread;

class EventLoopThreadPool : public noncopyable
{
 public:
  typedef std::function<void(EventLoop*)> ThreadInitCallback;
  
  EventLoopThreadPool();
  EventLoopThreadPool(EventLoop* baseLoop, const std::string& name);
  ~EventLoopThreadPool();
   
  void Init(EventLoop* baseLoop, int numThreads);
  void setThreadNum(int numThreads) { numThreads_ = numThreads; }
  void start(const ThreadInitCallback& cb = ThreadInitCallback());
  
  //valid after calling start(), round-robin.
  EventLoop* getNextLoop();
  
  //with the same hash code, it will always return the same EventLoop.
  EventLoop* getLoopForHash(size_t hashCode);
  
  std::vector<EventLoop*> getAllLoops();
  
  bool started() const { return started_; }
  const std::string& name() const { return name_; }
  
 private:
  EventLoop* baseLoop_;
  std::string name_;
  bool started_;
  int numThreads_;
  int next_;
  std::vector<std::shared_ptr<EventLoopThread> > threads_;
  std::vector<EventLoop*> loops_;
};

#endif