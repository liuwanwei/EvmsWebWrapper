/* -- $Id: DeadThreadTbl.h,v 1.7 2004/04/30 03:20:20 qianjinqi Exp $ -- */

#ifndef _DEAD_THREAD_TBL_H_
#define _DEAD_THREAD_TBL_H_	1

#include <pthread.h>

#include "NetServer.h"

// 死亡线程表
typedef struct {
  pthread_t tid_tbl[MAX_CONN_NUM]; // 死亡线程ID列表
  pthread_mutex_t mutex;		 // 互斥变量
} DEAD_THREAD_TBL, PDEAD_THREAD_TBL; //FIXME PDEAD_THREAD_TBL 前面应该加"*"吧,还好没有用到它

extern DEAD_THREAD_TBL DeadThreadTbl;	// 死亡线程列表

extern int InitDeadThreadTbl();
extern int DeinitDeadThreadTbl();

#ifdef DEBUG
extern int TestDeadThreadTbl();
#endif

extern int AddToDeadThreadTbl();

#endif /* DeadThreadTbl.h */
