#include "echo.h"

#include "../../../baseCom/Logging.h"
#include "../../../netWork/EventLoop.h"

#include <unistd.h>

int main()
{
	LOG_INFO << "pid = " << getpid();
	EventLoop loop;
	InetAddress listenAddr(2018);
	EchoServer server(&loop, listenAddr);
	server.start();
	loop.loop();
}
