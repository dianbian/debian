
//internal header file.

#ifndef NETWORK_POLLER_H
#define NETWORK_POLLER_H

#include "../baseCom/noncopyable.h"
#include "../baseCom/Timestamp.h"
#include "EventLoop.h"

#include <map>
#include <vector>

//base class for I/O multiplexing
//this class doesn't own the Channel objects.
class Poller : public noncopyable
{
 public:
  typedef std::vector<Channel*> ChannelList;
	
  Poller(EventLoop* loop);
  virtual ~Poller();
	
  //polls the I/O events.
  //Must be called in the loop thread.
  virtual Timestamp poll(int timeoutMs, ChannelList* activeChanels) = 0;
  //Changes the interested I/O events.
  //Must be called in the loop thread.
  virtual void updateChannel(Channel* channel) = 0;
  //Remove the channel, when it destructs.
  //Must be called in the loop thread.
  virtual void removeChannel(Channel* channel) = 0;
	
  virtual bool hasChannel(Channel* channel) const;

  static Poller* newDefaultPoller(EventLoop* loop);
	
  void assertInLoopThread() const
  {
     ownerLoop_->assertInLoopThread();
  }
	
 protected:
  typedef std::map<int, Channel*> ChannelMap;
  ChannelMap channels_;
	
 private:
  EventLoop* ownerLoop_;
};

#endif //NETWORK_POLLER_H
