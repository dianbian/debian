
#include "../baseCom/Logging.h"

#include "TimerQueue.h"
#include "EventLoop.h"
#include "Timer.h"
#include "TimerId.h"

#include <functional>

#include <sys/timerfd.h>
#include <unistd.h>

name detail
{

int createTimeerfd()
{
  int timerfd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
  if (timer < 0)
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

}