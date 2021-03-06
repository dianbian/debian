
#ifndef NETWORK_INETADDRESS_H
#define NETWORK_INETADDRESS_H

#include "../baseCom/copyable.h"
#include "../baseCom/StringPiece.h"

#include "../netWork/SocketsOps.h"

#include <netinet/in.h>

//EBO : derived empty class can optimize current class

//wrapper of sockaddr_in. (packaging properties)
//this is an POD interface class.
class InetAddress : public copyable
{
 public:
  //constructs an endpoint with given port number.
  //Mostly used in TcpSever listening.
  explicit InetAddress(uint16_t port = 0, bool loopbackOnly = false, bool ipv6 = false);
		
  //constructs an endpoint whit given ip and port
  //ip shoule be "1.2.3.4"
  InetAddress(StringArg ip, uint16_t port, bool ipv6 = false);
		
  //constructs an endpoint with given struct &c sockaddr_in.
  //Mostly used when accepting new connections
	explicit InetAddress(const struct sockaddr_in& addr) : addr_(addr)
  { }
		
	explicit InetAddress(const struct sockaddr_in6& addr) : addr6_(addr)
	{ }
		
	sa_family_t family() const { return addr_.sin_family; }
		
	std::string toIp() const;
	std::string toIpPort() const;
	uint16_t toPort() const;
		
	//default copy/assignment are ok
		
	const struct sockaddr* getSockAddr() const { return netsockets::sockaddr_cast(&addr6_); }
	void setSockAddrInet6(const struct sockaddr_in6& addr6) { addr6_ = addr6; }
		
	uint32_t ipNetEndian() const;
	uint16_t portNetEndian() const { return addr_.sin_port; }
		
	//reslove hostname to IP address, not changing port or sin_family.
	//return true on success, thread safe.
	static bool resolve(StringArg hostname, InetAddress* result);
		
 private:
	union
  {
		struct sockaddr_in addr_;
		struct sockaddr_in6 addr6_;
	};
};

#endif //NETWORK_INETADDRESS_H
