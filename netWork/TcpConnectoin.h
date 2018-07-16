/*************************************************************************
	> File Name: TcpConnectoin.h
	> Author: bian
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
};

#endif //NETWORK_TCPCONNECTION_H
