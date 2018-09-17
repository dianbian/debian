
#include "../../../baseCom/Logging.h"

#include "../../../netWork/EventLoop.h"
#include "../../../netWork/Buffer.h"

#include "echo.h"

EchoServer::EchoServer(EventLoop* loop, const InetAddress& listenAddr)
	: server_(loop, listenAddr, "EchoServer")
{
	server_.setConnectionCallback(std::bind(&EchoServer::onConnection, this, std::placeholders::_1));
	server_.setMessageCallback(std::bind(&EchoServer::onMessage, this, std::placeholders::_1,
				std::placeholders::_2, std::placeholders::_3));
}

void EchoServer::start()
{
	printf("EchoServer::start\n");
	server_.start();
}


void EchoServer::onConnection(const TcpConnectionPtr& conn)
{
	LOG_INFO << "EchoServer - " << conn->peerAddress().toIpPort() << " ->"
		<< conn->localAddress().toIpPort() << " is "
		<< (conn->connected() ? "UP" : "DOWN");
}

void EchoServer::onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp time)
{
  std::string msg(buf->retrieveAllAsString());
	LOG_INFO << conn->name() << " echo " << msg.size() << " bytes, "
		<< "data received at " << time.toString();
	printf("收到消息%s\n", msg.c_str());
	conn->send(msg);
}
