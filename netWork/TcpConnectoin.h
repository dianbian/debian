/*************************************************************************
	> File Name: TcpConnectoin.h
  > Mail: 986573837@qq.com 
	> Created Time: Sat 14 Jul 2018 10:39:22 AM CST
 ************************************************************************/

//this is a public head file, it must only include public header files.

#ifndef NETWORK_TCPCONNECTION_H
#define NETWORK_TCPCONNECTION_H

#include "../baseCom/StringPiece.h"
#include "../baseCom/Atomic.h"
#include "../baseCom/noncopyable.h"

#include "Callbacks.h"
#include "Buffer.h"
#include "InetAddress"

#include <memory>

struct tcp_info;

class Channel;
class EventLoop;
class Socket;

//Tcp connection, for both client and server usage.
//This is an interface class, so do not expose too much details.
class TcpConnectoin : public noncopyable, std::enable_shared_from_this<TcpConnectoin>
{
	//constructs a TcpConnectoin with a connected sockfd
	//user should not create this object. (don‘t new)
	TcpConnectoin(EventLoop* loop, const std::string& name, int sockfd, 
	            const InetAddress& localAddr, const InetAddress& peerAddr);
				
	~TcpConnectoin();
  
  EventLoop* getLoop() const { return loop_; }
  const std::string& name() const { return name_; }
  cosnt InetAddress& localAddress() const { return localAddr_; }
  const InetAddress& peerAddress() const { return peerAddr_; }
  bool connected() const { return state_ == kConnected; }
  bool disconnected() const { return state_ == kDisconnected_; }
  bool getTcpInfo(struct tcp_info*) const; //return true if success.
  std::string getTcpInfoString() const;
  
  void send(const void* message, int len);
  void send(const StringPiece& message);
  void send(Buffer* message);     //swap data
  void shutdown();   //not thread safe, noe simultaneous calling
  void forceClose();
  void forceCloseWithDelay(double seconds);
  void setTcpNoDelay(bool on);
  
  void startRead();  //reading or not
  void stopRead();
  bool isReading() const { return reading_; }  //not thread safe
  
  void setContext(cosnt any& context) { context_ = context; }
  const any& getContext() cosnt { return context_; }
  
  void setConnectionCallback(const ConnectionCallback& cb)
  { connectionCallback_ = cb; }
  void setMessageCallback(const MessageCallback& cb)
  { messageCallback_ = cb; }
  void setWriteCompleteCallback(const WriteCompleteCallback cb)
  { writeCompleteCallback_ = cb; }
  void setHighWaterMarkCallback(const HighWaterCallback& cb, size_t highWaterMark)
  { highWaterMarkCallback_ = cb; highWaterMark_ = highWaterMark; }
  //internal use only
  void setCloseCallback(const CloseCallback& cb) { closeCallback_ = cb; }
  //advanced interface
  Buffer* inputBuffer() { return &inputBuffer_; }
  Buffer* outputBuffer() { return &outputBuffer_; }
  //called when TcpServer accepts a new connection, should by called only once
  void connectEstablished();
  //called when TcpServer has removed me from its map, should ba called only once
  void connectDestroyed();
 
 private:
  enum StateE { kDisconnected, kConnecting, kConnected, kDisconnecting };
  void handleRead(Timestamp receiveTime);
  void handleWrite();
  void handleClose();
  void handleError();
  void sendInLoop(const StringPiece& message);
  void sendInLoop(const void* message, size_t len);
  void shutdownInLoop();
  void forceCloseInLoop();
  void setState(StateE s) { state_ = s; }
  const char* stateToString() const;
  void startReadInLoop();
  void stopReadInLoop();
  
  EventLoop* loop_;
  const std::string name_;
  StateE state_;   //use atomic variable
  bool reading_;
  //don't expose thos classes to client.
  std::unique<Socket> socket_;
  std::unique<Channel> channel_;
  const InetAddress localAddr_;
  const InetAddress peerAddr_;
  ConnectionCallback connectionCallback_;
  MessageCallback messageCallback_;
  WriteCompleteCallback writeCompleteCallback_;
  HighWaterCallback highWaterMarkCallback_;
  CloseCallback closeCallback_;
  size_t highWaterMark_;
  Buffer inputBuffer_;
  Buffer outputBuffer_;  //use list<Buffer> as output buffer.
  any context_;
};

typedef std::shared_ptr<TcpConnectoin> TcpConnectoinPtr;

#endif //NETWORK_TCPCONNECTION_H
