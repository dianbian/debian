#include "../../../netWork/EventLoop.h" 
#include "../../../netWork/InetAddress.h"
//#include "../../../netWork/TcpServer.h"

#include "../../../baseCom/Timestamp.h"

class EchoServer
{
 public:
	EchoServer(EventLoop * loop, const InetAddress& listenAddr);
	
	void start();   //调用server_.start()
	
 private:
  void onConnection(const TcpConnectionPtr& conn);
	
	void onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp time);
	
	TcpServer server_;
};
