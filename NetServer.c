
/* -- $Id: NetServer.cpp,v 1.12.2.2.4.9 2004/08/03 06:08:32 yuzhe Exp $ -- */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#include <unistd.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <pthread.h>

#include "SysConfExt.h"

#include "NetLog.h"
#include "Debuger.h"

#include "NetServer.h"
#include "CtrlLinkTbl.h"
#include "DeadThreadTbl.h"
#include "LinkCtrl.h"


/*****************
 *  全局变量定义
 *****************/
GLOBAL_STATE GlobalState;		// 线程状态

unsigned int USER_AUTH_PORT = 6018;
int CurPort;

static void
Timer_Thread_Cleanup (void *pinfo)
{
	TRACE (12,
		   (" ####### Processing Timer_Thread Cleanup Start! ######### \n"));

	TRACE (12,
		   (" ####### Processing Timer_Thread Cleanup Finished! ######### \n"));

	TRACE (14, (" Timer thread exit!\n"));

	return;
}

static void *
Timer_Thread (void *pinfo)
{
	int i;

	pthread_cleanup_push (Timer_Thread_Cleanup, NULL);
	for (;;)
	{
		//置位“已超时的Link控制线程”的停止位
		PCTRL_LINK_DESC pcld = CtrlLinkTbl;

		for (i = 0; i < MAX_CONN_NUM; i++, pcld++)
		{
			if (pcld->socket_fd == 0)
				continue;

			pthread_mutex_lock (&pcld->mutex);

			pcld->notinformcnt++;

			TRACE (12, (" Timer Count = %d\n ", pcld->notinformcnt));

			//未收到握手报文时间大于32分钟(30秒 * 64),结束控制线程
			if (pcld->notinformcnt > MAX_NOTINFORM_CNT)
			{
				pthread_cancel (pcld->ctrl_tid);
			}

			pthread_mutex_unlock (&pcld->mutex);
		}

		//10
		sleep (HANDSHAKE_PERIOD);
	}

	pthread_cleanup_pop (1);

	return (void *) 0;
}

/*Join_Thread()
 *PURPOSE
 *通过pthread_join涵数回收死亡线程,具体原理还不太明白 
 *ARGUMENTS
 *void *pinfo        - no use
 */
static void *
Join_Thread (void *pinfo)
{
	int i;

	while (GlobalState.stop_join_thread != 1)
	{

		for (i = 0; i < MAX_CONN_NUM; i++)
		{
			pthread_mutex_lock (&DeadThreadTbl.mutex);

			if (DeadThreadTbl.tid_tbl[i] != 0)
			{
				TRACE (5,
					   (" Join thread will reclaim the tid : %ld\n ",
						DeadThreadTbl.tid_tbl[i]));

				pthread_join (DeadThreadTbl.tid_tbl[i], NULL);
				DeadThreadTbl.tid_tbl[i] = 0;

				TRACE (5, (" Join thread has reclaimed it\n "));
			}

			pthread_mutex_unlock (&DeadThreadTbl.mutex);
		}

		//checking every 0.5 second
		usleep (500);
	}

	TRACE (14, (" Thread that joins dead_threads exit!\n"));

	return (void *) 0;
}

static void
Server_Thread_Cleanup (void *pinfo)
{
	TRACE (12,
		   (" ####### Processing NET_SERVER Cleanup Start! ######### \n"));

	//停止所有LINK_CTRL线程
	int i;
	PCTRL_LINK_DESC pcld = CtrlLinkTbl;

	for (i = 0; i < MAX_CONN_NUM; i++, pcld++)
	{
		if (pcld->ctrl_tid == 0)
			continue;

		pthread_cancel (pcld->ctrl_tid);
	}

	//关闭Listen端口
	int sock_fd = *(int *) pinfo;

	if (close (sock_fd) == -1)
	{
		perror ("close");
	}

	//修改Global变量
	// GlobalState.listen_thread_id = 0;

	TRACE (12,
		   (" ####### Processing NET_SERVER Cleanup Finished! ######### \n"));

	return;
}

static void *
Server_Thread (void *pinfo)
{
	//-- 初始化全局表格 --//
	InitDeadThreadTbl ();		//初始化死亡线程组织结构表

	InitCtrlLinkTbl ();			//初始化控制连接组织结构表,该结构描述了每个连接的信息

	//--  回收“死亡线程”启动 --//
	TRACE (14, ("Begin to thread that joins dead_threads!\n"));

	if (pthread_create
		(&GlobalState.join_thread_id, NULL, Join_Thread, NULL) != 0)
	{
		TRACE (6, ("Timer thread start Err!\n"));
	}

	//--  定时器线程启动 --//
	TRACE (14, ("Begin to timer thread!\n"));

	if (pthread_create
		(&GlobalState.timer_thread_id, NULL, Timer_Thread, NULL) != 0)
	{
		TRACE (6, ("Timer thread start Err!\n"));
	}

	//FIXME FIXME  FIXME FIXME

	//-- 准备网络连接 --//
	int listen_fd;

	if ((listen_fd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
	{
		perror ("socket");
		logRec (" socket err,errno:%d,id:%d\n", errno, getpid ());
		return (void *) 0;
	}

	TRACE (14, ("Succeed to create socket!\n"));

	struct sockaddr_in this_addr;
	struct sockaddr_in client_addr;

	this_addr.sin_family = AF_INET;
	this_addr.sin_port = htons (USER_AUTH_PORT);
	this_addr.sin_addr.s_addr = INADDR_ANY;
	memset (&(this_addr.sin_zero), '\0', 8);

	int reuse = 1;
	setsockopt (listen_fd, SOL_SOCKET, SO_REUSEADDR, (const void *) &reuse,
				sizeof (int));

	if (bind
		(listen_fd, (struct sockaddr *) &this_addr,
		 sizeof (struct sockaddr)) == -1)
	{
		perror ("bind");
		return (void *) 0;
	}

	TRACE (14, ("Succeed to bind!\n"));

	if (listen (listen_fd, MAX_CONN_NUM) == -1)
	{
		perror ("listen");
		return (void *) 0;
	}

	TRACE (14, ("Succeed to listen!\n"));

	//PUSH & POP
	pthread_cleanup_push (Server_Thread_Cleanup, &listen_fd);

	//-- 开始接受客户端连接请求 --//
	int link_ctrl_fd = 0;

	while (1)
	{
		TRACE (14, ("wait for client connection!\n"));

		socklen_t sin_size = sizeof (client_addr);

		link_ctrl_fd =
			accept (listen_fd, (struct sockaddr *) &client_addr,
					&sin_size);

		if (link_ctrl_fd == -1)
		{
			TRACE (0, ("!!! accept failed . !!! \n"));
			perror ("accept");
			break;
		}

		TRACE (14,
			   ("Begin to accept to client, _accept return %d !\n",
				listen_fd));

		//create with detach status defaultly
		pthread_t link_ctrl_tid;

		pthread_create (&link_ctrl_tid, NULL,
						(void *(*)(void *)) Link_Ctrl,
						(void *) (unsigned long long )link_ctrl_fd);
	}

	/*pthread_cleanup_pop
	 *PURPOSE
	 *execute cleanup function routine, or pop up 1 stack element only
	 *ARGUMENTS
	 *0         - pop up 1 stack element only
	 *1         - pop up 1 stack element and execute function toutine as pthread_cleanup_push
	 */
	pthread_cleanup_pop (1);

	return (void *) 0;
}

int
StartNetServer (int debug_level)
{
	printf (" # NetServer is starting... # \n ");

	/*
	CDebuger *pd = CDebuger::Instance();
	pd->SetDebugLevel(debug_level);
	printf("   --> Runing in debug level %d \n", pd->GetDebugLevel() );
	*/

	if (GlobalState.listen_thread_id != 0)
	{
		printf (" Net Server is already runing \n");
	}

	//初始化GlobalState变量, '\0' 用printf输出时跟"(int)0"无差别, 但跟'0'不同
	memset (&GlobalState, '\0', sizeof (GLOBAL_STATE));

	CurPort = USER_AUTH_PORT + 1;

	//"(void * (*)(void*))", 将Server_Thread函数转换,实际上无意义,是这样吗?
	if (pthread_create
		(&GlobalState.listen_thread_id, NULL,
		 (void *(*)(void *)) Server_Thread, NULL) != 0)
	{
		perror ("pthread_create");
		return -1;
	}

	return 0;
}

int
StopNetServer (void)
{
	//停止所有LINK_CTRL线程
	int i;

	PCTRL_LINK_DESC pcld = CtrlLinkTbl;

	for (i = 0; i < MAX_CONN_NUM; i++, pcld++)
	{
		if (pcld->ctrl_tid == 0)
			continue;

		//停止线程
		pthread_cancel (pcld->ctrl_tid);
		pthread_join (pcld->ctrl_tid, NULL);
	}

	//停止TIMER线程
	pthread_cancel (GlobalState.timer_thread_id);

	//停止JOIN线程
	GlobalState.stop_join_thread = 1;

	//停止LISTEN线程
	pthread_cancel (GlobalState.listen_thread_id);
	sleep (1);

	//JOIN线程
	TRACE (12, ("+++++ Terminating Listen Thread ++++++++\n"));
	pthread_join (GlobalState.listen_thread_id, NULL);
	GlobalState.listen_thread_id = 0;

	TRACE (12, ("+++++ Terminating Timer Thread ++++++++\n"));
	pthread_join (GlobalState.timer_thread_id, NULL);
	GlobalState.timer_thread_id = 0;

	TRACE (12, ("+++++ Terminating Join Thread ++++++++\n"));
	pthread_join (GlobalState.join_thread_id, NULL);
	GlobalState.join_thread_id = 0;
	GlobalState.stop_join_thread = 0;

	//全局结构清除
	// FIXME CDebuger *pd = CDebuger::Instance();
	// pd->SetDebugLevel(0);

	//恢复全局变量
	memset (&GlobalState, '\0', sizeof (GLOBAL_STATE));

	DeinitDeadThreadTbl ();

	DeinitCtrlLinkTbl ();

	printf ("\n\n # NetServer exits normally. # \n\n ");

	return 0;
}
