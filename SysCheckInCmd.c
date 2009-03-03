
/* -- $Id: SysCheckInCmd.cpp,v 1.12.2.1.4.3 2004/07/22 11:14:41 yuzhe Exp $ -- */
	 
	
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <string.h>
	
#include <unistd.h>
#include <sys/types.h>
	
#include <sys/socket.h>
#include <netinet/in.h>
	
#include "SysCheckInCmd.h"
#include "SysConfExt.h"
	
#include "NetLog.h"
#include "Debuger.h"
	
#include "CtrlLinkTbl.h"
#include "Protocol.h"
	int
SysCheckInCmd (int sock_fd) 
{
	
		//-- 登记客户端连接 --//
	int i;
	int pos = -1;
	unsigned long ipaddr;
	struct sockaddr_in sockaddr;
	unsigned int salen = sizeof (sockaddr);

	PCTRL_LINK_DESC pcld;
	TRACE (12, (" #### Enter SYS_CHECKIN_REQ #### \n"));
	getpeername (sock_fd, (struct sockaddr *) &sockaddr, &salen);
	ipaddr = sockaddr.sin_addr.s_addr;
	pcld = CtrlLinkTbl;
	for (i = 0; i < MAX_CONN_NUM; i++, pcld++)
		
	{
		if (pcld->socket_fd != 0)
			
		{
			if (pcld->ipaddr == ipaddr)
				
			{
				
					// repeated link
					TRACE (6,
						   ("Repeated connection from same ip address\n"));
				goto FAIL;
			}
		}
		
		else if (-1 == pos)
			
		{
			pos = i;
		}
	}
	if (-1 == pos)
		
	{
		
			// Link Table is full
			TRACE (6, ("Link Ctrl Table is full!\n"));
		goto FAIL;
	}
	
	else
		
	{
		
			// 找到空表项
			pcld = CtrlLinkTbl;
		pcld += pos;
		pthread_mutex_lock (&pcld->mutex);
		
			// strncpy(pcld->username, username, MAX_USRNAME_LEN);
			pcld->socket_fd = sock_fd;
		pcld->ipaddr = ipaddr;
		
			/* pthread_self
			 * PURPOSE
			 * return identifier of current thread
			 */ 
			pcld->ctrl_tid = pthread_self ();
		pthread_mutex_unlock (&pcld->mutex);
	}
	return 0;
  FAIL:TRACE (12, (" #### FINISHED SYS_CHECKIN_REQ #### \n"));
	return -1;
}


