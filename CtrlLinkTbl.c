
/* -- $Id: CtrlLinkTbl.cpp,v 1.12.2.2.4.2 2004/07/22 11:14:36 yuzhe Exp $ -- */
	 
	
#include <stdio.h>
#include <string.h>
	
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
	
#include "Debuger.h"
#include "CtrlLinkTbl.h"
#include "SysConfExt.h"
	CTRL_LINK_DESC CtrlLinkTbl[MAX_CONN_NUM];	// øÿ÷∆¡¥Ω”√Ë ˆ±Ì
int
InitCtrlLinkTbl () 
{
	int i;

	PCTRL_LINK_DESC pcld = CtrlLinkTbl;
	for (i = 0; i < MAX_CONN_NUM; i++, pcld++)
		
	{
		pcld->ipaddr = 0;
		memset (&pcld->username, '\0', MAX_USRNAME_LEN);
		pcld->socket_fd = 0;
		pcld->ctrl_tid = 0;
		pthread_mutex_init (&pcld->mutex, NULL);
		pcld->notinformcnt = 0;
		pcld->head = -1;
		pcld->tail = -1;
	}
	return 0;
}
int
DeinitCtrlLinkTbl () 
{
	int i;

	PCTRL_LINK_DESC pcld = CtrlLinkTbl;
	for (i = 0; i < MAX_CONN_NUM; i++, pcld++)
		
	{
		pthread_mutex_destroy (&pcld->mutex);
		memset (pcld, '\0', sizeof (CTRL_LINK_DESC));
	}
	return 0;
}

PCTRL_LINK_DESC GetCtrlLink (int sock_fd) 
{
	int i;
	int isOk = 0;

	PCTRL_LINK_DESC pcld = CtrlLinkTbl;
	for (i = 0; i < MAX_CONN_NUM; i++, pcld++)
		
	{
		if (pcld->socket_fd == sock_fd)
			
		{
			isOk = 1;
			break;
		}
	}
	if (isOk == 1)
		return pcld;
	
	else
		return NULL;
}
int
GetLinkUser (int sock_fd, char *username, int len) 
{
	if (len < MAX_USRNAME_LEN)
		return -1;
	int i;
	int isOk = 0;

	PCTRL_LINK_DESC pcld = CtrlLinkTbl;
	for (i = 0; i < MAX_CONN_NUM; i++, pcld++)
		
	{
		if (pcld->socket_fd == sock_fd)
			
		{
			memcpy (username, pcld->username, MAX_USRNAME_LEN);
			username[MAX_USRNAME_LEN - 1] = '\0';
			isOk = 1;
			break;
		}
	}
	if (isOk == 1)
		return 0;
	
	else
		return -1;
}
int
DelFromLinkTbl (int sock_fd) 
{
	PCTRL_LINK_DESC pctrllink;
	pctrllink = GetCtrlLink (sock_fd);
	if (pctrllink == 0)
		return -1;
	int i;
	int isOk = 0;

	PCTRL_LINK_DESC pcld = CtrlLinkTbl;
	for (i = 0; i < MAX_CONN_NUM; i++, pcld++)
		
	{
		if (pcld != pctrllink)
			continue;
		pthread_mutex_lock (&pcld->mutex);
		pcld->ipaddr = 0;
		memset (&pcld->username, '\0', MAX_USRNAME_LEN);
		pcld->socket_fd = 0;
		pcld->ctrl_tid = 0;
		pcld->notinformcnt = 0;
		pcld->head = -1;
		pcld->tail = -1;
		pthread_mutex_unlock (&pcld->mutex);
		isOk = 1;
		break;
	}
	if (isOk == 1)
		return 0;
	
	else
		return -1;
}


#ifdef DEBUG
	int
TestCtrlLinkTbl () 
{
	printf
		(" #################### LINK_CTRL TABLE ################################ \n");
	printf
		("     ipaddr    username sock_fd ctrl_tid notinformcnt buflen head tail \n");
	printf
		(" ------------- -------- ------- -------- ------------ ------ ---- ---- \n");
	int i;

	PCTRL_LINK_DESC pcld = CtrlLinkTbl;
	for (i = 0; i < MAX_CONN_NUM; i++, pcld++)
		
	{
		printf (" %13s %8s %7d %8ld %12d %6d %4d %4d\n",
				 inet_ntoa (*(struct in_addr *) &pcld->ipaddr),
				 pcld->username, pcld->socket_fd, pcld->ctrl_tid,
				 pcld->notinformcnt, pcld->unhandled_buf.len, pcld->head,
				 pcld->tail);
	} printf ("\n\n");
	return 0;
}


#endif	/*  */
