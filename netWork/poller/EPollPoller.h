
#ifndef NETWORK_EPOLLPOLLER_H
#define NETWORK_EPOLLPOLLER_H

#include "../Poller.h"

#include <vector>

struct epoll_event;

// IO Multiplexing with epoll(4).

class EPollPoller : public Poller
{
 public:
  EPollPoller(EventLoop* loop);
  virtual ~EPollPoller();
  
  virtual Timestamp poll(int timeoutMs, ChannelList* activeChannels);
  virtual void updateChannel(channel* channel);
  virtual void removeChannel(Channel* channel);
  
 private:
  static const int kInitEventListSize = 16;
  
  static const char* operationToString(int op);
  
  void fillActiveChannels(int numEvents, channelList* activeChannels) const;
  
  void update(int operation, Channel* channel);
  
  typedef std::vector<struct epoll_event> EventList;
  int epollfd_;
  EventList events_;
};

#endif