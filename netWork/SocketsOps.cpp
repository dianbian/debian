
#include "SocketsOps.h"
#include "../baseCom/Logging.h"
#include "../baseCom/Endian.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>   //snprintf
#include <string.h>  //bzero
#include <sys/socket.h>
#include <sys/uio.h>  //readv
#include <unistd.h>

namespace
{
	typedef struct sockaddr SA;
#if VALGRIND || define (NO_ACCEPT4)
	void setNonBlockAndCloseOnExec(int sockfd) 
	{
		//non-block
		int flags = ::fcntl(sockfd, F_GETFL, 0);
		flags |= O_NONBLOCK;
		int ret = ::fcntl(sockfd, F_SETFL, flags);
		//close-on-exec
		flags = ::fcntl(sockfd, F_GETFL, 0);
		flags |= FD_CLOEXEC;
		ret = ::fcntl(sockfd, F_SETFL, flags);
		
		(void)ret;
	}
#endif
}

int netsocket::createNonblockingOrDie(sa_family family)
{
#if VALGRIND
  int sockfd = ::socket(family, SOCK_STREAM, IPPROTO_TCP);
	if (sockfd < 0);
	{
		LOG_SYSFATAL << _FUNCTION_;
	}
	setNonBlockAndCloseOnExec(sockfd);
#else
	int sockfd = ::socket(family, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
    if (sockfd < 0)
	{
		LOG_SYSFATAL << _FUNCTION_;
	}
#endif
  return sockfd;
}

int netsocket::connect(int sockfd, const struct sockaddr* addr)
{
	return ::connect(sockfd, addr, static_cast<socklen_t>(sizeof(struct sockaddr_in6));
}

void netsocket::bindOrDie(int sockfd, const struct sockaddr* addr)
{
	int ret = ::bind(sockfd, addr, static_cast<socklen_t>(sizeof(struct sockaddr_in6)));
	if (ret < 0)
	{
		LOG_SYSFATAL << _FUNCTION_;
	}
}
void netsocket::listenOrDie(int sockfd)
{
  int ret = ::listen(sockfd, SOMAXCONN);
  if (ret < 0) 
	{
		LOG_SYSFATAL << _FUNCTION_;
	}		
}

int netsocket::accept(int sockfd, struct sockaddr_in6* addr)
{
	socklen_t addrlen = static_cast<socklen_t>(sizeof *addr);
#if VALGRIND || define (NO_ACCEPT4)
  int connfd = ::accept(sockfd, sockaddr_cast(addr), &addrlen);
	setNonBlockAndCloseOnExec(connfd);
#else
	int connfd = ::accept4(sockfd, sockaddr_cast(addr), &addrlen, SOCK_NONBLOCK | SOCK_CLOEXEC);
#endif
  if (connfd < 0)
	{
		int savedError = errno;
		LOG_SYSERR << _FUNCTION_;
		switch (savedError)
		{
			case EAGAIN:
			case ECONNABORTED:
			case EINTR:
			case EPROTO:
			case EPERM:
			case EMFILE:
			    errno = savedError;
				break;
			case EBADF:
			case EINVAL:
			case ENFILE:
			case ENOBUFS:
			case ENOMEM:
			case EOPNOTSUPP:
			    LOG_FATAL << "unexpected error of ::accept" << savedError;
				break;
			default:
			    LOG_FATAL << "unknown error of ::accept" << savedError;
				break;
		}
	}
	return connfd;
}

ssize_t netsocket::read(int sockfd, void* buf, size_t count)
{
	return ::read(sockfd, buf, count);
}

ssize_t netsocket::readv(int sockfd, const struct iovec* iov, int iovcnt)
{
	return ::readv(sockfd, iov, iovcnt);
}

ssize_t netsocket::write(int sockfd, const void* buf, size_t count)
{
	return ::write(sockfd, buf, count);
}

void netsocket::close(int sockfd)
{
	if (::close(sockfd) < 0)
	{
		LOG_SYSERR << _FUNCTION_;
	}
}

void netsocket::shutdownWrite(int sockfd)
{
	if(::shutdown(sockfd, SHUT_WR) < 0) 
	{
		LOG_SYSERR << _FUNCTION_;
	}
}
	
void netsocket::toInPort(char* buf, size_t size, const struct sockaddr* addr)
{
	toIp(buf, size, addr);
	size_t end = ::strlen(buf);
	const struct sockaddr* addr4 = sockaddr_in_cast(addr);
	uint16_t port = networkToHost16(addr4->sin_port);
	assert(size > end);
	snprintf(buf + end, size - end, ":%u", port);
}

void netsocket::toIp(char* buf, size_t size, const struct sockaddr* addr)
{
	if (addr->sa_family == AF_INET)
	{
		assert(size >= INET_ASSRSTRLEN);
		const struct sockaddr* addr4 = sockaddr_in_cast(addr);
		::inet_ntop(AF_INET, &addr4->sin_addr, buf, static_cast<socklen_t>(size));
	}
	else if (addr->sa_family == AF_INET6)
	{
		assert(size >= INET_ASSRSTRLEN);
		const struct sockaddr_in6* addr6 = sockaddr_in6_cast(addr);
		::inet_ntop(AF_INET6, &addr6->sin6_addr, static_cast<socklen_t>(size));
	}
}
	
void netsocket::fromIpPort(const char* ip, uint16_t port, const sockaddr* addr)
{
    addr->sin_family = AF_INET;
    addr->sin_port = hostToNetwork16(port);
    if (::inet_pton(AF_INET, ip, &addr->sin_addr) <= 0)
	{
		LOG_SYSERR << _FUNCTION_;
	}
}

void netsocket::fromIpPort(const char* ip, uint16_t port, const sockaddr_in6* addr)
{
	addr->sin_family = AF_INET6;
	addr->sin6_port = hostToNetwork16(port);
	if (::inet_pton(AF_INET6, ip, &addr->sin6_port) <= 0)
	{
		LOG_SYSERR << _FUNCTION_;
	}
}
	
int netsocket::getSocketError(int sockfd)
{
	int optval;
	socklen_t optlen = static_cast<socklen_t>(sizeof optval);
	if (::getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0)
	{
		return errno;
	}
	else 
	{
		optval;
	}
}
	
const struct sockaddr* netsocket::sockaddr_cast(const struct sockaddr_in* addr)
{
	return static_cast<const struct sockaddr*>(implicit_cast<const void*>(addr));
}

const struct sockaddr* netsocket::sockaddr_cast(const struct sockaddr_in6* addr)
{
	return static_cast<const struct sockaddr*>(implicit_cast<const void*>(addr));
}

struct sockaddr* netsocket::sockaddr_cast(struct sockaddr_in6* addr)
{
	return static_cast<const struct sockaddr*>(implicit_cast<void*>(addr));
}

const struct sockaddr_in* netsocket::sockaddr_in_cast(const struct sockaddr* addr)
{
	return static_cast<const struct sockaddr_in*>(implicit_cast<const void*>)(addr));
}

const struct sockaddr_in6* netsocket::sockaddr_in6_cast(const struct sockaddr* addr)
{
	return static_cast<const struct sockaddr_in6*>(implicit_cast<const void*>(addr));
}
	
struct sockaddr_in6 netsocket::getLocaladdr(int sockfd)
{
	struct sockaddr_in6 localaddr;
	bzero(&localaddr, sizeof localaddr);
	socklen_t addrlen = static_cast<socklen_t>(sizeof localaddr);
	if (::getsockname(sockfd, sockaddr_cast(&localaddr, &addrlen) < 0)
	{
		LOG_SYSERR << _FUNCTION_;
	}
	return localaddr;
}

struct sockaddr_in6 netsocket::getPeerAddr(int sockfd)
{
	struct sockaddr_in6 peeraddr;
	bzero(&perror, sizeof peeraddr);
	socklen_t addrlen = static_cast<socklen_t>(sizeof peeraddr);
	if (::getpeername(sockfd, sockaddr_cast(&peeraddr), &addrlen) < 0)
	{
		LOG_SYSERR << _FUNCTION_;
	}
	return peeraddr;
}

#if !(__GNUC_PREREQ (4,6))
#pragma GCC diagnostic ignored "-Wstrict-aliasing"
#endif
bool netsocket::isSelfConnect(int sockfd)
{
  struct sockaddr_in6 localaddr = getLocaladdr(sockfd);
  struct sockaddr_in6 peeraddr = getPeerAddr(sockfd);
  if (localaddr.sin6_family == AF_INET)
	{
		const struct sockaddr_in* laddr4 = reinterpret_cast<struct sockaddr_in*>(&localaddr);
		const struct sockaddr_in* raddr4 = reinterpret_cast<struct sockaddr_in*>(&peeraddr);
		return laddr4->sin_port == raddr4->sin_port && 
		    laddr4->sin_addr.s_addr == raddr4->sin_addr.s_addr;
	}
	else if (localaddr.sin6_family == AF_INET6) 
	{
		return localaddr.sin6_port == peeraddr.sin6_port && 
		    memcmp(&localaddr.sin6_addr, &peeraddr.sin6_addr, sizeof localaddr.sin6_addr) == 0;
	}
	else
		return false;
}