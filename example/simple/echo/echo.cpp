
#include "../../../baseCom/Logging.h"

#include "echo.h"

EchoServer::EchoServer(EventLoop* loop, const InetAddress& listenAdrr)
	: server_(loop, listenAddr, "EchoServer")
{
	server_.setConnectionCallback(std::bind(&EchoServer::onConnection, this, _1));
	server_.setMessageCallback(std::bind(&EchoServer::onMessage, this, _1, _2, _3));
}

void EchoServer::start()
{
	server_.start();
}


void EchoServer::onConnection(const TcpConnectionPtr& conn)
{
	LOG_INFO << "EchoServer - " << conn->peerAddr().toIpPort() << " ->"
		<< conn->localAddress().toIpPort() << " is "
		<< (conn->connected() ? "UP" : "DOWN");
}

void EchoServer::onMessage(TcpConnectionPtr& conn, Buffer* buf, Timestamp time)
{
	string msg(buf->retrieveAllAsString());
	LOG_INFO << conn->name() << " echo " << msg.size() << " bytes, "
		<< "data received at " << time.toString();
	conn->send(msg);
}
