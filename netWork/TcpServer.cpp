
#include "../../baseCom/Logging.h"

#include "TcpServer.h"
#include "Acceptor.h"
#include "EventLoop.h"
#include "EventLoopThreadPool.h"
#include "SocketsOps.h"

#include <functional>

#include <stdio.h>

TcpServer::TcpServer(EventLoop* loop, const InetAddress& listenAddr,
                    const std::string& nameArg, Option option)
  : loop_(CHECK_NOTNULL(loop)),
    ipPort_(listenAddr.toIpPort()),
    name_(nameArg),
    acceptor_(new Acceptor(loop, listenAddr, option == kReusePort)),
    threadPool_(new EventLoopThreadPool(loop, name_),
    connectionCallback_(defaultConnectionCallback),
    messageCallback_(defaultMessageCallback),
    nextConnId_(1),
{
  acceptor_->setNewConnectionCallback(std::bind(&TcpServer::newConnection, this, _1, _2));
}

TcpServer::~TcpServer()
{
  loop_->assertInLoopThread();
  LOG_TRACE << "TcpServer::~TcpServer [ " << name_ << "] destructing";
  
  for (auto it = connections_.begin(); it != connections_.end(); ++it)
  {
    TcpConnectionPtr conn(it->second);
    it->second.reset();
    conn->getLoop->runInLoop(std::bin(&TcpConnection::connectDestroyed, conn));
  }
}

void TcpServer::setThreadNum(int numThreads)
{
  assert(0 <= numThreads);
  threadPool_->setThreadNum(numThreads);
}

void TcpServer::start()
{
  if (starte_.getAndSet(1) == 0)
  {
    threadPool_->start(threadInitCallback_);
    
    assert(!acceptor_->listenning());
    loop_->runInLoop(std::bind(&Acceptor::listen, get_pointer(acceptor_)));
  }
}

void TcpServer::newConnection(int sockfd, const InetAddress& peerAddr)
{
  loop_->assertInLoopThread();
  EventLoop* ioLoop = threadPool_->getNextLoop();
  char buf[64];
  snprintf(buf, sizeof buf, "-%s#%d", ipPort_.c_str(), nextConnId_);
  ++nextConnId_;
  std::string connName = name_ + buf;
  
  LOG_INFO << "TcpServer::newConnection [" << name_ << "] - new connection [" << connName
           << "] from " << peerAddr.toIpPort();
  InetAddress localAddr(netsockets::getLocalAddr(sockfd));
  //poll with zero timeout to double confirm the new connection
  //use make_shared if necessary
  TcpConnectionPtr conn(new TcpConnection(ioLoop, connName, sockfd, localAddr, peerAddr));
  connections_[connName] = conn;
  conn->setConnectionCallback(connectionCallback_);
  conn->setMessageCallback(messageCallback_);
  conn->setWriteCompleteCallback(writeCompleteCallback_);
  conn->serCloseCallback(std::bind(&TcpServer::removeConnection, this, _1));  //unsafe ?
  ioLoop->runInLoop(std::bind(&TcpServer::connectEstablished, conn));
}

void TcpServer::removeConnection(const TcpConnectionPtr& conn)
{
  loop_->runInLoop(std::bind(&TcpServer::removeConnectionInLoop, this ,conn)); //unsafe ?
}

void TcpServer::removeConnectionInLoop(const TcpConnectionPtr& conn)
{
  loop_->assertInLoopThread();
  LOG_INFO << "TcpServer::removeConnectionInLoop [" << name_
           << "] - connection" << conn->name();
           
  size_t n = connections_.erase(conn->name());
  (void)n;
  assert(n == 1);
  EventLoop* ioLoop = conn->getLoop();
  ioLoop->queueInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
}