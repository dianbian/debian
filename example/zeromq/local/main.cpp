#include "Logging.h"
#include "EventLoop.h"
#include "TcpServer.h"
#include "codec.h"

#include <memory>
#include <stdio.h>
#include <unistd.h>

bool g_tcpNoDelay = false;

void onConnection(const TcpConnectionPtr& conn)
{
  if (conn->connected())
  {
    conn->setTcpNoDelay(g_tcpNoDelay);
  }
}

void onStringMessage(LengthHeaderCodec* codec,
                     const TcpConnectionPtr& conn,
                     const std::string& message,
                     Timestamp)
{
  codec->send(conn.get(), message);
}


int main(int argc, char* argv[])
{
  if (argc > 1)
  {
    uint16_t port = static_cast<uint16_t>(atoi(argv[1]));
    g_tcpNoDelay = argc > 2 ? atoi(argv[2]) : 0;
    int threadCount = argc > 3 ? atoi(argv[3]) : 0;

    LOG_INFO << "pid = " << getpid() << ", listen port = " << port;
    // muduo::Logger::setLogLevel(muduo::Logger::WARN);
    EventLoop loop;
    InetAddress listenAddr(port);
    TcpServer server(&loop, listenAddr, "PingPong");
    LengthHeaderCodec codec(std::bind(onStringMessage, &codec, 
					std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

    server.setConnectionCallback(onConnection);
    server.setMessageCallback(bind(&LengthHeaderCodec::onMessage, &codec, 
					std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    if (threadCount > 1)
    {
      server.setThreadNum(threadCount);
    }

    server.start();

    loop.loop();
  }
  else
  {
    fprintf(stderr, "Usage: %s listen_port [tcp_no_delay [threads]]\n", argv[0]);
  }
}
