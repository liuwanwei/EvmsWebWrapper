#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <stdio.h>

#include "SocketMid.h"

#ifdef SUBNET

#include "Socket.h"

extern int _socket (int domain, int type, int protocol);
extern int _listen (int sockfd, int backlog);
extern int _send (int sockfd, const void *pMsg, size_t len, int flags);
extern int _recv (int sockfd, void *buf, size_t len, int flags);
extern int _bind (int sockfd, const struct sockaddr *addr,
				  socklen_t addrlen);
extern int _accept (int sockfd, const struct sockaddr *addr,
					socklen_t * addrlen);
extern int _getpeername (int sockfd, struct sockaddr *addr,
						 socklen_t * addrlen);
extern int _close (int sockfd);
#endif

int
socket_mid (int domain, int type, int protocol)
{
#ifdef SUBNET
	printf ("__subnet__\n");
	return _socket (domain, type, protocol);
#else
	printf ("__normal__\n");
	return socket (domain, type, protocol);
#endif
}

int
listen_mid (int sockfd, int backlog)
{
#ifdef SUBNET
	return _listen (sockfd, backlog);
#else
	return listen (sockfd, backlog);
#endif
}

int
send_mid (int sockfd, void *pMsg, size_t len, int flags)
{
#ifdef SUBNET
	return _send (sockfd, pMsg, len, flags);
#else
	return send (sockfd, pMsg, len, flags);
#endif
}

int
recv_mid (int sockfd, void *buf, size_t len, int flags)
{
#ifdef SUBNET
	return _recv (sockfd, buf, len, flags);
#else
	return recv (sockfd, buf, len, flags);
#endif
}

int
bind_mid (int sockfd, struct sockaddr *addr, socklen_t addrlen)
{
#ifdef SUBNET
	return _bind (sockfd, addr, addrlen);
#else
	return bind (sockfd, addr, addrlen);
#endif
}

int
accept_mid (int sockfd, struct sockaddr *addr, socklen_t * addrlen)
{
#ifdef SUBNET
	return _accept (sockfd, addr, addrlen);
#else
	return accept (sockfd, addr, addrlen);
#endif
}

int
getpeername_mid (int sockfd, struct sockaddr *addr, socklen_t * addrlen)
{
#ifdef SUBNET
	return _getpeername (sockfd, addr, addrlen);
#else
	return getpeername (sockfd, addr, addrlen);
#endif
}

int
close_mid (int sockfd)
{
#ifdef SUBNET
	return _close (sockfd);
#else
	return close (sockfd);
#endif
}
