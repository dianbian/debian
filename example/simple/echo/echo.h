#include "netWork/TcpServer.h"

class EchoServer
{
  public:
	EchoServer(net::EventLoop * loop, const net::InetAddress& listenAddr);
	
	void start();   //调用server_.start()
	
  private:
    void onConnection(const net::TcpConnectionPtr& conn);
	
	void onMessage(const net::TcpConnectionPtr& conn， net::Buffer* buf, TimeStamp time);
	
	net::TcpServer server_;
};