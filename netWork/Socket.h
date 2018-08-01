
#ifndef NETWORK_SOCKET_H
#define NETWORK_SOCKET_H

#include "../baseCom/noncopyable.h"

//in <netinet/tcp.h>
struct tcp_info;

class InetAddress;

//wrapper of socket file descriptor.     包装socket文件描述符
//it closes the sockfd when destructs.
//it's thread safe, all operations are delegated to OS. 安全，系统代理
class Socket : public noncopyable
{
 public:
  explicit Socket(int sockfd) : sockfd_(sockfd)
  { }
  ~Socket();
  
  int fd() const { return sockfd_; }
  //return true if success.
  bool getTcpInfo(struct info*) const;
  bool getTcpInfoString(char* buf, int len) const;
  
  //abort if address in use
  void bindAddress(const InetAddress& localaddr);
  //abort if address
  void listen();
  
  //on success, return a non-negative integer(非负) this is
  //a descriptor for the accepted socket, which has been set
  //to non-blocking and close-on-exec. *peeraddr is assigned.
  //on error, -1 is returned, and *peeraddr is untouched.
  int accept(InetAddress* peeraddr);
  
  void shutdownWrite();
  //enable/disable TCP_NODELAY (diable/enable Nagle's algorithm).
  void setTcpNoDelay(bool on);
  //enable/disable SO_REUSEADDR
  void setReuseAddr(bool on);
  //enable/disable SO_REUSEPORT
  void setReusePort(bool on);
  //enable/disable SO_KEEPALIVE
  void setKeepAlive(bool on);
 
 private:
  const int sockfd_;
};

#endif