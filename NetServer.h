/* -- $Id: NetServer.h,v 1.12.2.3.4.3 2004/07/28 04:53:26 yuzhe Exp $ -- */

#ifndef _NETSERVER_H
#define _NETSERVER_H    1

#define MAX_CONN_NUM                8           // 支持的最大客户端数
#define MAX_USER_NUM                24          // 最大用户个数

#define MAX_USRNAME_LEN             16          // 最大用户名长度
#define MAX_PASSWD_LEN              16          // 最大密码长度
#define MAX_FILENAME_LEN           128          // 最大文件路径名长度

/*
#define MAX_FRAME_LEN             4096          // 最大报文长度（socket）
                                                // 太大了吧,也许超出路径MTU了,
                                                // 会造成整个TCP报文段重发
*/

#define HANDSHAKE_PERIOD            10          // 控制连接握手报文发送周期（30秒）
#define MAX_NOTINFORM_CNT            5          // 最大允许未接收握手报文次数（5次）

#define MAX_TIMEOUT                 40          // 数据连接超时时间（4秒）：
                                                // 超时仍等不到正确客户端连接，
                                                // 直接关闭该数据连接监听端口。
#define TCP_MODE      		  0x00
#define UDP_MODE      		  0x01
#define MCAST_MODE    		  0x02

#define MAX_NUM_PTZ_PROTOCOL 	  32

#ifdef DEBUG
#define FIFO_TEST_PATH	"/tmp/NetServerFIFO"
#endif

//-- 全局表相关结构体 --//
#include <pthread.h>

// FIXME using namespace ::std;//"::" mains global

typedef struct __GLOBAL_STATE {
  pthread_t listen_thread_id;	          // LISTEN线程ID,主服务器线程
  
  pthread_t join_thread_id;		  // JOIN线程ID
  unsigned int stop_join_thread;          // 停止JOIN线程flag
  
  pthread_t timer_thread_id;		  // 定时线程ID
  
  #ifdef DEBUG
  pthread_t test_thread_id;	  	  // 测试线程ID
  #endif
  
} GLOBAL_STATE, *PGLOBAL_STATE;

//-- 全局变量声明 --//
extern GLOBAL_STATE GlobalState;			      // 线程状态

//-- 引出函数 --//
extern int StartNetServer();
extern int StopNetServer(void);

#endif /* NetServer.h */
