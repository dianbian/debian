
#include "../baseCom/Logging.h"

#include "Connector.h"
#include "Channel.h"
#include "EventLoop.h"
#include "SockersOps.h"

#include <errno.h>

const int Connector::kMaxRetryDelayMs;

Connector::Connector(EventLoop* loop, const InetAddress& serverAddr)
  : loop_(loop),
    serverAddr_(serverAddr),
    connect_(false),
    state_(kDisconnected),
    retryDelayMs_(kInitRetryDelayMs)
{
  
}