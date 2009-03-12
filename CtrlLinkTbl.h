/* -- $Id: CtrlLinkTbl.h,v 1.12 2004/04/30 03:20:20 qianjinqi Exp $ -- */

#ifndef _CTRL_LINK_TBL_H
#define _CTRL_LINK_TBL_H 1

#include "Protocol.h"
#include "NetServer.h"

// ������������
typedef struct {
  unsigned long ipaddr;                  // �ͻ���IP
  char username[MAX_USRNAME_LEN];        // �û���
  int socket_fd;                         // socket��
  pthread_t ctrl_tid;                    // �����̺߳�
  pthread_mutex_t mutex;                 // �������
  int notinformcnt;                      // δ�յ������֡����ĵĴ�����ÿ��30�룩
  
  struct {
  	char buf[MAX_FRAME_LEN];
  	int len;
  } unhandled_buf;			 // ������δ��������
  
  int head;             		 // �����߳�ͷ����±��
  int tail;      			 // �����߳�β����±��
} CTRL_LINK_DESC, *PCTRL_LINK_DESC;

extern CTRL_LINK_DESC CtrlLinkTbl[MAX_CONN_NUM];              // ��������������

extern int InitCtrlLinkTbl();
extern int DeinitCtrlLinkTbl();

extern PCTRL_LINK_DESC GetCtrlLink(int __sock_fd);
extern int GetLinkUser(int __sock_fd, char* __username, int __len);

extern int DelFromLinkTbl(int sock_fd);

#ifdef DEBUG
extern int TestCtrlLinkTbl();
#endif

#endif /* CtrlLinkTbl.h */

