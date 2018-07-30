
#ifndef NETWORK_SOCKET_H
#define NETWORK_SOCKET_H

#include "../baseCom/noncopyable.h"

//in <netinet/tcp.h>
struct tcp_info;

class InetAddress;

//wrapper of socket file descriptor.
//it closes the sockfd when destructs.
//it's thread safe, all operations are delegated to OS.
class Socket : public noncopyable
{
 public:
  explicit Socket(int sockfd) : sockfd_(sockfd)
  { }
  
};

#endif