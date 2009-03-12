/* -- $Id: CtrlLinkTbl.h,v 1.12 2004/04/30 03:20:20 qianjinqi Exp $ -- */

#ifndef _CTRL_LINK_TBL_H
#define _CTRL_LINK_TBL_H 1

#include "Protocol.h"
#include "NetServer.h"

// 控制链接描述
typedef struct {
  unsigned long ipaddr;                  // 客户端IP
  char username[MAX_USRNAME_LEN];        // 用户名
  int socket_fd;                         // socket号
  pthread_t ctrl_tid;                    // 控制线程号
  pthread_mutex_t mutex;                 // 互斥变量
  int notinformcnt;                      // 未收到“握手”报文的次数（每次30秒）
  
  struct {
  	char buf[MAX_FRAME_LEN];
  	int len;
  } unhandled_buf;			 // 缓冲区未处理数据
  
  int head;             		 // 数据线程头结点下标号
  int tail;      			 // 数据线程尾结点下标号
} CTRL_LINK_DESC, *PCTRL_LINK_DESC;

extern CTRL_LINK_DESC CtrlLinkTbl[MAX_CONN_NUM];              // 控制链接描述表

extern int InitCtrlLinkTbl();
extern int DeinitCtrlLinkTbl();

extern PCTRL_LINK_DESC GetCtrlLink(int __sock_fd);
extern int GetLinkUser(int __sock_fd, char* __username, int __len);

extern int DelFromLinkTbl(int sock_fd);

#ifdef DEBUG
extern int TestCtrlLinkTbl();
#endif

#endif /* CtrlLinkTbl.h */

