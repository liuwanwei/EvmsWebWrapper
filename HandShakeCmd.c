
/* -- $Id: HandShakeCmd.cpp,v 1.12.2.1.4.3 2004/07/22 11:14:37 yuzhe Exp $ -- */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "HandShakeCmd.h"
#include "NetLog.h"
#include "Debuger.h"
#include "CtrlLinkTbl.h"
#include "Protocol.h"

int
HandShakeCmd (int sock_fd, char *pPacket, int len) 
{
	TRACE (12, (" #### GOT HANDSHAKE CMD #### \n"));
	int i;

	PCTRL_LINK_DESC pcld = CtrlLinkTbl;
	for (i = 0; i < MAX_CONN_NUM; i++, pcld++)
	{
		if (pcld->socket_fd != sock_fd)
			continue;
		pthread_mutex_lock (&pcld->mutex);
		pcld->notinformcnt = 0;
		pthread_mutex_unlock (&pcld->mutex);

		// 报文头结构，只需要将发来的包原封不动的扔回去就结束了。
		PACKET_HDR rspPkt;
		memcpy (&rspPkt, pPacket, len);

		/*
		rspPkt.subtype = HANDSHAKE_REQ;
		rspPkt.retcode = 0x00;
		*/

		if (send (sock_fd, &rspPkt, sizeof (rspPkt), 0) == -1)
		{
			perror ("send_mid");
		}

		break;
	}

	TRACE (12, (" #### FINISHED HANDSHAKE #### \n"));
	return 0;
}

