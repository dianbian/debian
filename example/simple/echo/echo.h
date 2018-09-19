#include "InetAddress.h"
#include "TcpServer.h"
#include "Timestamp.h"

class EventLoop;
class Buffer;

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
