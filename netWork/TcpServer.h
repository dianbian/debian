#ifndef NETWORK_TCPSERVER_H
#define NETWORK_TCPSERVER_H

#include "../baseCom/Atomic.h"
#include "../baseCom/Types.h"
#include "../baseCom/noncopyable.h"

#include "TcpConnectoin.h"

#include <map>
#include <functional>
#include <memory>

class Acceptor;
class EventLoop;
class EventLoopThreadPool;

//tcp servrer, supports single-threaded and thread-pool models.
//this is an interface class, so don't expose too much details.
class TcpServer : public noncopyable
{
 public:
	typedef std::function<void(EventLoop*)> ThreadInitCallback;
	enum Option
	{
		kNoReusePort,
		kReusePort
	};
	TcpServer(EventLoop* loop, const InetAddress& listenAddr, 
			      const std::string& nameArag, Option option = kNoReusePort);
	~TcpServer();

	const std::string& ipPort() const { return ipPort_; }
	const std::string& name() const { return name_; }
	EventLoop* getLoop() const { return loop_: }
  
	//set the number of threads for handling input.
	//always accepts new connection in loop's thread.
	//must be called before progress start
	//0 means all I/O in loop's thread, no thread will created, default value.
	//1 means all I/O in another thread.
	//N means a thread pool with N threads, new connections are assigned on a round-robin basis.
	void setThreadNum(int numThreads);
	void setThreadInitCallback(const ThreadInitCallback& cb)
	{ ThreadInitCallback_ = cb; }
	//valid after calling start()  why?
	std::shared_ptr<EventLoopThreadPool> threadPool()
	{ return threadPool_;	}

  //start the server if it is not listening.
	//it is harmless to call it multiple times, thread safe.
	void start();

	//set connection callback, not thread safe.
	void setConnectionCallback(const ConnectionCallback& cb)
	{ ConnectionCallback_ = cb; }
	//set message callback, not thread safe.
	void setMessageCallback(const MessageCallback& cb)
	{ messageCallback_ = cb; }
	//set write complte callback, not thread safe.
	void setWriteCompleteCallback(const WriteCompleteCallback& cb)
	{ writeCompleteCallback_ = cb; }

 private:
	//not thread safe, but in loop.
	void newConnection(int sockfd, const InetAddress& peerAddr);
	//thread safe.
	void removeConnection(const TcpConnectoinPtr& conn);
	//not thread safe, but in loop.
	void removeConnectionInLoop(const TcpConnectoinPtr& conn);

	typedef std::map<std::string, TcpConnectoinPtr> ConnectionMap;

	EventLoop* loop_;  //this is the acceptor loop
	const std::string inPort_;
	const std::string name_;
	std::unique_ptr<Acceptor> acceptor_; //avoid revealing acceptor
	std::shared_ptr<EventLoopThreadPool> threadPool_;
	ConnectionCallback connectionCallback_;
	MessageCallback messageCallback_;
	WriteCompleteCallback writeCompleteCallback_;
	ThreadInitCallback threadinitCallback_;
	AtomicInt32 started_;

	//always in loop thread.
	int nextConnId_;
	ConnectionMap connections_;
};

#endif
