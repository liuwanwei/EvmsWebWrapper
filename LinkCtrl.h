/* -- $Id: LinkCtrl.h,v 1.12 2004/04/30 03:20:22 qianjinqi Exp $ -- */

#ifndef _LINKCTRL_H
#define _LINKCTRL_H    1

typedef int (* PktHandler)(int sock_fd, char * oneframe, int len);

extern int Link_Ctrl(int __socket_fd);

#endif /* LinkCtrl.h */
