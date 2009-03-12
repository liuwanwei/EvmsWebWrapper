#ifndef __SOCKET_MID_H
#define __SOCKET_MID_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

extern int socket_mid(int domain, int type, int protocol);
extern int listen_mid(int sockfd, int backlog);
extern int send_mid(int sockfd, void *pMsg, size_t len, int flags);
extern int recv_mid(int sockfd, void *buf, size_t len, int flags);
extern int bind_mid(int sockfd, struct sockaddr *addr, socklen_t addrlen);
extern int accept_mid(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
extern int getpeername_mid(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
extern int close_mid(int  sockfd);

#endif
