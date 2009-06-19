/* -- $Id: NetServer.h,v 1.12.2.3.4.3 2004/07/28 04:53:26 yuzhe Exp $ -- */

#ifndef _NETSERVER_H
#define _NETSERVER_H    1

#define MAX_CONN_NUM                8           // ֧�ֵ����ͻ�����
#define MAX_USER_NUM                24          // ����û�����

#define MAX_USRNAME_LEN             16          // ����û�������
#define MAX_PASSWD_LEN              16          // ������볤��
#define MAX_FILENAME_LEN           128          // ����ļ�·��������

/*
#define MAX_FRAME_LEN             4096          // ����ĳ��ȣ�socket��
                                                // ̫���˰�,Ҳ����·��MTU��,
                                                // ���������TCP���Ķ��ط�
*/

#define HANDSHAKE_PERIOD            10          // �����������ֱ��ķ������ڣ�30�룩
#define MAX_NOTINFORM_CNT            5          // �������δ�������ֱ��Ĵ�����5�Σ�

#define MAX_TIMEOUT                 40          // �������ӳ�ʱʱ�䣨4�룩��
                                                // ��ʱ�ԵȲ�����ȷ�ͻ������ӣ�
                                                // ֱ�ӹرո��������Ӽ����˿ڡ�
#define TCP_MODE      		  0x00
#define UDP_MODE      		  0x01
#define MCAST_MODE    		  0x02

#define MAX_NUM_PTZ_PROTOCOL 	  32

#ifdef DEBUG
#define FIFO_TEST_PATH	"/tmp/NetServerFIFO"
#endif

//-- ȫ�ֱ���ؽṹ�� --//
#include <pthread.h>

// FIXME using namespace ::std;//"::" mains global

typedef struct __GLOBAL_STATE {
  pthread_t listen_thread_id;	          // LISTEN�߳�ID,���������߳�
  
  pthread_t join_thread_id;		  // JOIN�߳�ID
  unsigned int stop_join_thread;          // ֹͣJOIN�߳�flag
  
  pthread_t timer_thread_id;		  // ��ʱ�߳�ID
  
  #ifdef DEBUG
  pthread_t test_thread_id;	  	  // �����߳�ID
  #endif
  
} GLOBAL_STATE, *PGLOBAL_STATE;

//-- ȫ�ֱ������� --//
extern GLOBAL_STATE GlobalState;			      // �߳�״̬

//-- �������� --//
extern int StartNetServer();
extern int StopNetServer(void);

#endif /* NetServer.h */
