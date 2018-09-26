#ifndef BASECOM_ASYNCLOGGING_H
#define BASECOM_ASYNCLOGGING_H

//#include "BlockingQueue.h"
//#include "BoundedBlockingQueue.h"
#include "CountDownLatch.h"
#include "Mutex.h"
#include "Thread.h"
#include "LogStream.h"
#include "noncopyable.h"

#include <memory>
#include <vector>
#include <string>

class AsyncLogging : public noncopyable
{
 public:
  AsyncLogging(const std::string& basename,
               off_t rollSize,
               int flushInterval = 3);

  ~AsyncLogging()
  {
    if (running_)
    {
      stop();
    }
  }

  void append(const char* logline, int len);

  void start()
  {
    running_ = true;
    thread_.start();
    latch_.wait();
  }

  void stop()
  {
    running_ = false;
    cond_.notify();
    thread_.join();
  }

 private:

  // declare but not define, prevent compiler-synthesized functions
  AsyncLogging(const AsyncLogging&);  // ptr_container
  void operator=(const AsyncLogging&);  // ptr_container

  void threadFunc();

  typedef detail::FixedBuffer<detail::kLargeBuffer> Buffer;
  typedef std::shared_ptr<Buffer> BufferPtr;
  typedef std::vector<BufferPtr> BufferVector;

  const int flushInterval_;
  bool running_;
  std::string basename_;
  off_t rollSize_;
  Thread thread_;
  CountDownLatch latch_;
  MutexLock mutex_;
  Condition cond_;
  BufferPtr currentBuffer_;
  BufferPtr nextBuffer_;
  BufferVector buffers_;
};

#endif  // BASECOM_ASYNCLOGGING_H
