
#include "Logging.h"

#include "Connector.h"
#include "Channel.h"
#include "EventLoop.h"
#include "SocketsOps.h"

#include <errno.h>

const int Connector::kMaxRetryDelayMs;

Connector::Connector(EventLoop* loop, const InetAddress& serverAddr)
  : loop_(loop),
    serverAddr_(serverAddr),
    connect_(false),
    state_(kDisconnected),
    retryDelayMs_(kInitRetryDelayMs)
{
  LOG_DEBUG << "ctor[" << this << "]";
}

Connector::~Connector()
{        
  LOG_DEBUG << "dtor[" << this << "]";
	assert(!channel_);
}

void Connector::start()
{
  connect_ = true;
  loop_->runInLoop(std::bind(&Connector::startInLoop, this));  //unsafe ?
}

void Connector::startInLoop()
{
  loop_->assertInLoopThread();
  assert(state_ == kDisconnected);
  if (connect_)
  {
    connect();
  }
  else
  {
    LOG_DEBUG << "do not connect";
  }
}

void Connector::stop()
{
  connect_ = false;
  loop_->queueInLoop(std::bind(&Connector::stopInLoop, this)); //unsafe ?
}

void Connector::stopInLoop()
{
  loop_->assertInLoopThread();
  if (state_ == kConnecting)
  {
    setState(kDisconnected);
    int sockfd = removeAndResetChannel();
    retry(sockfd);
  }
}

void Connector::connect()
{
  int sockfd = netsockets::createNonblockingOrDie(serverAddr_.family());
  int ret = netsockets::connect(sockfd, serverAddr_.getSockAddr());
  int savedErrno = (ret == 0) ? 0 : errno;
  switch (savedErrno)
  {
    case 0:
    case EINPROGRESS:
    case EINTR:
    case EISCONN:
      connecting(sockfd);
      break;
    case EAGAIN:
    case EADDRINUSE:
    case ECONNREFUSED:
    case ENETUNREACH:
      retry(sockfd);
      break;
    case EACCES:
    case EPERM:
    case EAFNOSUPPORT:
    case EALREADY:
    case EBADF:
    case EFAULT:
    case ENOTSOCK:
      LOG_SYSERR << "connect error in Connector::startInLoop " << savedErrno;
      netsockets::close(sockfd);
      break;
    default:
      LOG_DEBUG << "Unexpected error in Connector::startInLoop " << savedErrno;
      netsockets::close(sockfd);
      break;
  }
}

void Connector::restart()
{
	loop_->assertInLoopThread();
  setState(kDisconnected);
  retryDelayMs_ = kInitRetryDelayMs;
  connect_ = true;
  startInLoop();
}

void Connector::connecting(int sockfd)
{
  setState(kConnecting);
  assert(!channel_);
  channel_.reset(new Channel(loop_, sockfd));
  channel_->setWriteCallback(std::bind(&Connector::handleWrite, this));
  channel_->setErrorCallback(std::bind(&Connector::handleError, this));
  channel_->enableWriting();
}

int Connector::removeAndResetChannel()
{
  channel_->disableAll();
  channel_->remove();
  int sockfd = channel_->fd();
  //can't reset channel here, because we are inside Channel::handleEvent
  loop_->queueInLoop(std::bind(&Connector::resetChannel, this));
	return sockfd;
}

void Connector::resetChannel()
{
  channel_.reset();
}

void Connector::handleWrite()
{
  LOG_TRACE << "Connector::handleWrite " << state_;
  
  if (state_ == kConnecting)
  {
    int sockfd = removeAndResetChannel();
    int err = netsockets::getSocketError(sockfd);
    if (err)
    {
      LOG_WARN << "Connector::handleWrite - SO_ERROR = "
               << err << " " << strerror_tl(err);
      retry(sockfd);
    }
    else if (netsockets::isSelfConnect(sockfd))
    {
      LOG_WARN << "Connector::handleWrite - Self connect";
      retry(sockfd);
    }
    else 
    {
      setState(kConnected);
      if (connect_)
      {
        newConnectionCallback_(sockfd);  
      }
      else
      {
        netsockets::close(sockfd);
      }
    }
  }
  else
  { //?
    assert(state_ == kDisconnected);
  }
}

void Connector::handleError()
{
  LOG_ERROR << "Connector::handleError state = " << state_;
  if (state_ == kConnecting)
  {
    int sockfd = removeAndResetChannel();
    int err = netsockets::getSocketError(sockfd);
    LOG_TRACE << "SO_ERROR = " << err << " " << strerror_tl(err);
    retry(sockfd);
  }
}

void Connector::retry(int sockfd)
{
  netsockets::close(sockfd);
  setState(kDisconnected);
  if (connect_)
  {
    LOG_INFO << "Connector::retry - Retry connecting to " << serverAddr_.toIpPort()
             << " in " << retryDelayMs_ << "milliseconds. "; 
    loop_->runAfter(retryDelayMs_ / 1000.0, std::bind(&Connector::startInLoop, shared_from_this()));
    retryDelayMs_ = std::min(retryDelayMs_ * 2, kMaxRetryDelayMs);
  }
  else
  {
    LOG_DEBUG << "do not connect";
  }
}
