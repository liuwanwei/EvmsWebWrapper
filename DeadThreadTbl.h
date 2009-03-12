/* -- $Id: DeadThreadTbl.h,v 1.7 2004/04/30 03:20:20 qianjinqi Exp $ -- */

#ifndef _DEAD_THREAD_TBL_H_
#define _DEAD_THREAD_TBL_H_	1

#include <pthread.h>

#include "NetServer.h"

// �����̱߳�
typedef struct {
  pthread_t tid_tbl[MAX_CONN_NUM]; // �����߳�ID�б�
  pthread_mutex_t mutex;		 // �������
} DEAD_THREAD_TBL, PDEAD_THREAD_TBL; //FIXME PDEAD_THREAD_TBL ǰ��Ӧ�ü�"*"��,����û���õ���

extern DEAD_THREAD_TBL DeadThreadTbl;	// �����߳��б�

extern int InitDeadThreadTbl();
extern int DeinitDeadThreadTbl();

#ifdef DEBUG
extern int TestDeadThreadTbl();
#endif

extern int AddToDeadThreadTbl();

#endif /* DeadThreadTbl.h */
