/*************************************************************************
	> File Name: TcpConnectoin.cpp
	> Mail: 986573837@qq.com 
	> Created Time: Sat 14 Jul 2018 10:39:03 AM CST
 ************************************************************************/

#include "../BaseCom/Logging.h"
#include "../BaseCom/WeakCallback.h"

#include "TcpConnectoin.h"
#include "Channel.h"
#include "EventLoop.h"
#include "Socket.h"
#include "SocketsOps.h"

#include <functional>

#include <errno.h>
using namespace std;

void defaultConnectionCallback(const TcpConnectoinPtr& conn)
{
  LOG_TRACE << conn->localAddress().toIpPort() << " -> "
            << conn->peerAddress().toInPort() << " is "
            << (conn->connected() ? "UP" : "DOWN");
  //do not call nonn->forceClose()
}

void defaultMessageCallback(const TcpConnectoinPtr&, Buffer* buf, Timestamp)
{
  buf->retrieveAll();
}

TcpConnectoin::TcpConnectoin(EventLoop* loop,
                            const string& nameArg,
                            int sockfd,
                            const InetAddress& localAddr,
                            const InetAddress& peerAddress)
                          : loop_(CHECK_NOTNULL(loop)),
                            name_(nameArg),
                            state_(kConnecting),
                            reading_(true),
                            socket_(new Socket(sockfd)),
                            channel_(new Channel(loop, sockfd)),
                            localAddr_(localAddr),
                            peerAddress_(peerAddress),
                            highWaterMark_(64 * 1024 * 1024)
{
  channel_->setReadCallback(std::bind(&TcpConnectoin::handleRead, this, _1));
  channel_->setWriteCallback(std::bind(&TcpConnectoin::handleWrite, this));
  channel_->setCloseCallback(std::bind(&TcpConnectoin::handleClose, this));
  channel_->setErrorCallback(std::bind(&TcpConnectoin::handleError, this));
  LOG_DEBUG << "TcpConnectoin::ctor[" << name_ << "] at << this << " fd= " << sockfd;
  socket_->setKeepAlive(true);
}

TcpConnectoin::~TcpConnectoin()
{
  LOG_DEBUG << "TcpConnectoin::dtor[" << name_ << " ] at " << this 
            << " fd= " << channel_->fd() << " state= " << stateToString();
  assert(state_ == kDisconnected);
}

bool TcpConnectoin::getTcpInfo(struct tcp_info* tcpi) const
{
  return socket_->getTcpInfo(tcpi);
}

string TcpConnectoin::getTcpInfoString() const
{
  char buf[1024];
  buf[0] = '\0';
  socket_->getTcpInfoString(buf, sizeof buf);
  return buf;
}

void TcpConnectoin::send(const void* data, int len)
{
  send(StringPiece(static_cast<const char*>(data), len));
}

void TcpConnectoin::send(const StringPiece& message)
{
  if (state_ == kConnected)
  {
    if (loop_->isInLoopThread())
    {
      sendInLoop(message);
    }
    else
    {
      loop_->runInLoop(std::bind(&TcpConnectoin::sendInLoop, this, message.as_string()));
    }
  }
}
//efficiency ?
void TcpConnectoin::send(Buffer* buf)
{
  if (state_ == kConnected)
  {
    if (loop_->isInLoopThread())
    {
      sendInLoop(buf->peek(), buf->readableBytes());
      buf->retrieveAll();
    }
    else
    {
      loop_->runInLoop(std::bind(&TcpConnectoin::sendInLoop, this, buf->retrieveAllAsString()));
    }
  }
}

void TcpConnectoin::sendInLoop(const StringPiece& message)
{
  sendInLoop(message.data(), message.size());
}
void TcpConnectoin::sendInLoop(const void* data, size_t len)
{
  loop_->assertInLoopThread();
  ssize_t nwrote = 0;
  size_t remaining = len;
  bool faultError = false;
  if (state_ == kDisconnected)
  {
    LOG_WARN << "disconnected, give up writing.";
    return;
  }
  //if no thing in output queue, try writing directly
  if (!channel_->isWriting() && outputBuffer_.readableBytes() == 0)
  { //outputBuffer_ 类内全局变量, 用来保存全部数据, 分批次发送
    nwrote = netsocket::write(channel_->fd(), data, len);
    if (nwrote >= 0)
    {
      remaining = len - nwrote;
      if (remaining == 0 && writeCompleteCallback_)
      {
        loop_->queueInLoop(std::bind(writeCompleteCallback_, shared_from_this()));
      }
    }
    else
    {
      nwrote = 0;
      if (errno != EWOULDBLOCK)
      {
        LOG_SYSERR << ""TcpConnectoin::sendInLoop;
        if (errno == EPIPE || errno == ECONNRESET)
        {
          faultError = true;
        }
      }
    }
  }
  
  assert(remaining <= len);
  if (!faultError && remaining > 0)
  {
    size_t oldLen = outputBuffer_.readableBytes();
    if (oldLen + remaining >= highWaterMark_ && oldLen < highWaterMark_ && highWaterMarkCallback_)
    {
      loop_->queueInLoop(std::bind(highWaterMarkCallback_, shared_from_this(), oldLen + remaining));
    }
    outputBuffer_.append(static_cast<const char*>(data) + nwrote, remaining);
    if (!channel_->isWriting())
    {
      channel_->enableWriting();
    }
  }
}

void TcpConnectoin::shutdown()
{
  if (state_ == kDisconnected)
  {
    setState(kDisconnecting);
    loop_->runInLoop(std::bind(&TcpConnectoin::shutdownInLoop, this));
  }
}

void TcpConnectoin::shutdownInLoop()
{
  loop_->assertInLoopThread();
  if (!channel_->isWriting())
  {
    socket_->shutdownWrite();
  }
}

void TcpConnectoin::forceClose()
{
  if (state_ == kConnected || state_ == kDisconnecting)
  {
    setState(kDisconnecting);
    loop_->queueInLoop(std::bind(&TcpConnectoin::forceCloseInLoop, shared_from_this()));
  }
}

void TcpConnectoin::forceCloseWithDelay(double seconds)
{
  if (state_ == kConnected || state_ == kDisconnecting)
  {
    setState(kDisconnecting);
    loop_->runAfter(seconds, makeWeakCallback(shared_from_this(), &TcpConnectoin::forceClose());
    //not forceCloseInLoop to avoid race condition
  }
}

void TcpConnectoin::forceCloseInLoop()
{
  loop_->assertInLoopThread();
  if (state_ == kConnected || state_ == kDisconnecting)
  {
    handleClose(); //as if we received 0 bytes in handleRead().
  }
}

const char* TcpConnectoin::stateToString() const
{
  switch (state_)
  {
    case kDisconnected:
      return "kDisconnected";
    case kConnecting:
      return "kConnecting";
    case kConnected:
      return "kConnected";
    case kDisconnecting:
      return "kDisconnecting";
    default:
      return "unknown state";
  }
}

void TcpConnectoin::setTcpNoDelay(bool on)
{
  socket_->setTcpNoDelay(on);
}

void TcpConnectoin::startRead()
{
  loop_->runInLoop(std::bind(&TcpConnectoin::startReadInLoop, this));
}

void TcpConnectoin::startReadInLoop()
{
  loop_->assertInLoopThread();
  if (!reading_ || !channel_->isReading())
  {
    channel_->enableReading();
    reading_ = true;
  }
}

void TcpConnectoin::stopRead()
{
  loop_->runInLoop(std::bind(&TcpConnectoin::stopReadInLoop, this));
}

void TcpConnectoin::stopReadInLoop()
{
  loop_->assertInLoopThread();
  if (reading_ || channel_->isReading())
  {
    channel_->disableReading();
    reading_ = false;
  }
}

void TcpConnectoin::connectEstablished()
{
  loop_->assertInLoopThread();
  assert(state_ == kConnecting);
  setState(kConnected);
  channel_->tie(shared_from_this());
  channel_->enableReading();
  
  connectionCallback_(shared_from_this());
}

void TcpConnectoin::connectDestroyed()
{
  loop->assertInLoopThread();
  if (state_ == kConnected)
  {
    setState(kConnected);
    channel_->disableAll();
    
    connectionCallback_(shared_from_this());
  }
  channel_->remove();
}

void TcpConnectoin::handleRead(Timestamp receiveTime)
{
  loop_->assertInLoopThread();
  int savedErrno = 0;
  ssize_t n = inputBuffer_.readFd(channel_->fd(), &savedErrno);
  if (n > 0)
  {
    messageCallback_(shared_from_this(), &inputBuffer_, receiveTime);
  }
  else if (n == 0)
  {
    handleClose();
  }
  else
  {
    errno = savedErrno;
    LOG_SYSERR << "TcpConnectoin::handleRead";
    handleError();
  }
}

void TcpConnectoin::handleWrite()
{
  loop_->assertInLoopThread();
  if (channel_->isWriting())
  {
    ssize_t n = netsocket::write(channel_->fd(), outputBuffer_.peek(), outputBuffer_.readableBytes());
    if (n > 0)
    {
      outputBuffer_.retrieve(n);
      if (outputBuffer_.readableBytes() == 0)
      {
        channel_->disableWriting();
        if (writeCompleteCallback_)
        {
          loop_->queueInLoop(std::bind(writeCompleteCallback_, shared_from_this()));
        }
        if (state_ == kDisconnecting)
        {
          shutdownInLoop();
        }
      }
    }
    else
    {
      LOG_SYSERR << "TcpConnectoin::handleWrite";
    }
  }
  else
  {
    LOG_TRACE << "Connection fd = " << channel_->fd() << " is down, no more writing";
  }
}

void TcpConnectoin::handleClose()
{
  loop_->assertInLoopThread();
  LOG_TRACE << "fd = " << channel_->fd() << " state = " << stateToString();
  assert(state_ == kConnected || state_ == kDisconnecting);
  //wo don't close fd, leave it to dtor, so we cam find leaks easily.
  setState(kDisconnected);
  channel_->disableAll();
  
  TcpConnectoinPtr guardThis(shared_from_this());
  connectionCallback_(guardThis);
  //must be the last line
  closeCallback_(guardThis);
}