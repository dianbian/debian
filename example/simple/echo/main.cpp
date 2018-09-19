#include "echo.h"

#include "Logging.h"
#include "EventLoop.h"

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
