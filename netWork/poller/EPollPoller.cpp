
#include "../../baseCom/Logging.h"
#include "../../baseCom/Types.h"

#include "../Channel.h"
#include "EPollPoller.h"

#include <type_traits>

#include <assert.h>
#include <errno.h>
#include <poll.h>
#include <sys/epoll.h>
#include <unistd.h>

//on linux, the constants of poll(2) and epoll(4)
//are expected to be the same.
static_assert(EPOLLIN == POLLIN, "pollin");
static_assert(EPOLLPRI == POLLPRI, "pollpri");
static_assert(EPOLLOUT == POLLOUT, "pollout");
static_assert(EPOLLRDHUP == POLLRDHUP, "pollrdhup");
static_assert(EPOLLERR == POLLERR, "pollerr");
static_assert(EPOLLHUP == POLLHUP, "pollhup");

namespace
{
const int kNew = -1;
const int kAdded = 1;
const int kDeleted = 2;
}

EPollPoller::EPollPoller(EventLoop* loop) 
  : Poller(loop), 
    epollfd_(::epoll_create1(EPOLL_CLOEXEC)),
    events_(kInitEventListSize)
{
  if (epollfd_ < 0)
  {
    LOG_SYSFATAL << "EPollPoller::EPollPoller";
  }
}

EPollPoller::~EPollPoller()
{
  ::close(epollfd_);
}

Timestamp EPollPoller::poll(int timeoutMs, ChannelList* activeChannels)
{
  LOG_TRACE << "fd total count " << channels_.size(); //base class var
  int numEvents = ::epoll_wait(epollfd_,
                              &*events_.begin(),
                              static_cast<int>(events_.size()),
                              timeoutMs);
  int savedErrno = errno;
  Timestamp now(Timestamp::now());
  if (numEvents > 0)
  {
    LOG_TRACE << numEvents << " events happened";
    fillActiveChannels(numEvents, activeChannels);
    if (implicit_cast<size_t>(numEvents) == events_.size())
    {
      events_.resize(events_.size() * 2);
    }
  }
  else if (numEvents == 0)
  {
    LOG_TRACE << "nothing happened";
  }
  else
  {
    //errno happens, log uncommon ones
    if (savedErrno != EINTR)
    {
      errno = savedErrno;
      LOG_SYSERR << "EPollPoller::poll()";
    }
  }
  return now;
}
