
#ifndef NETWORK_ACCEPTOR_H
#define NETWORK_ACCEPTOR_H

#include "noncopyable.h"

#include "Channel.h"
#include "Socket.h"

#include <functional>

class EventLoop;
class InetAddress;

//acceptor of incoming TCP connections.
class Acceptor : public noncopyable
{
 public:
	 typedef std::function<void (int sockfd, const InetAddress&)> NewConnetionCallback;

	 Acceptor(EventLoop* loop, const InetAddress& listenAddr, bool reuseport);
	 ~Acceptor();

	 void setNewConnectionCallback(const NewConnetionCallback& cb)
	 { newConnectionCallback_ = cb; }

	 bool listenning() const { return listenning_; }
	 void listen();

 private:
	 void handleRead();

	 EventLoop* loop_;
	 Socket acceptSocket_;
	 Channel acceptChannel_;
	 NewConnetionCallback newConnectionCallback_;
	 bool listenning_;
	 int idleFd_;
};

#endif
