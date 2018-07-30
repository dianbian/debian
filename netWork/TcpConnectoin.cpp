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

