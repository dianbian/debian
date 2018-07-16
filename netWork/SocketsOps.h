//This is internal head file, you can not include this.

#ifndef NETWORK_SOCKETSOPS_H
#define NETWORK_SOCKETSOPS_H

#include <arpa/inet.h>

namespace netsockets
{
	//creates a non-blocking socket file descriptor,
	//abort if any error.
	int createNonblockingOrDie(sa_family family);
	
	int connect(int sockfd, const struct sockaddr* addr);
	void bindOrDie(int sockfd, const struct sockaddr* addr);
	void listenOrDie(int sockfd);
	int accept(int sockfd, struct sockaddr_in6* addr);
	ssize_t read(int sockfd, void* buf, size_t count);
	ssize_t readv(int sockfd, const struct iovec* iov, int iovcnt);
	ssize_t write(int sockfd, const void* buf, size_t count);
	void close(int sockfd);
	void shutdownWrite(int sockfd);
	
	void toInPort(char* buf, size_t size, const struct sockaddr* addr);
	void toIp(char* buf, size_t size, const struct sockaddr* addr);
	
	void fromIpPort(const char* ip, uint16_t port, const sockaddr* addr);
	void fromIpPort(const char* ip, uint16_t port, const sockaddr_in6* addr);
	
	int getSocketError(int sockfd);
	
	const struct sockaddr* sockaddr_cast(const struct sockaddr_in* addr);
	const struct sockaddr* sockaddr_cast(const struct sockaddr_in6* addr);
	struct sockaddr* sockaddr_cast(struct sockaddr_in6* addr);
	const struct sockaddr_in* sockaddr_in_cast(const struct sockaddr* addr);
	const struct sockaddr_in6* sockaddr_in6_cast(const struct sockaddr* addr);
	
	struct sockaddr_in6 getLocaladdr(int sockfd);
	struct sockaddr_in6 getPeerAddr(int sockfd);
	bool isSelfConnect(int sockfd);
	
}

#endif //NETWORK_SOCKETSOPS_H