
/* -- $Id: RecvOneFrame.cpp,v 1.12.2.2.4.3 2004/07/22 11:14:40 yuzhe Exp $ -- */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>

#include "RecvOneFrame.h"

#include "NetLog.h"
#include "Debuger.h"

#include "NetServer.h"
#include "CtrlLinkTbl.h"
#include "Protocol.h"


#ifdef DEBUG
static void
PrintPkt (char *p, int len)
{
	printf ("\n .... RECEIVE PACKET CONTENT : .... \n");

	int i;

	for (i = 0; i < len; i++, p++)
	{
		if (i != 0 && i % 8 == 0)
			printf ("\n");

		printf ("%02x ", (unsigned char) (*p));
	}
	printf ("\n");

	return;
}
#endif

/**********************************************************************
* recvFrame:
* packet saved in char* pAppFrame, 
* return :	length of packet
**********************************************************************/
int
RecvOneFrame (int s, char *pAppFrame)
{
	assert (s >= 0);
	assert (pAppFrame != NULL);

	char bufleft[MAX_FRAME_LEN];
	char *ptrstart = bufleft, *ptrend = bufleft;

	int found_item = 0;

	//-- 在连接表中查找表项，其中保存未处理的报文分片 --//
	int i;

	PCTRL_LINK_DESC pcld = CtrlLinkTbl;

	for (i = 0; i < MAX_CONN_NUM; i++, pcld++)
	{
		if (pcld->socket_fd != s)
			continue;

		if (pcld->unhandled_buf.len > 0)
		{
			memcpy (bufleft, pcld->unhandled_buf.buf,
					pcld->unhandled_buf.len);
			ptrend = ptrend + pcld->unhandled_buf.len;

			pthread_mutex_lock (&pcld->mutex);

			pcld->unhandled_buf.len = 0;

			pthread_mutex_unlock (&pcld->mutex);
		}

		found_item = 1;

		break;
	}

	if (found_item != 1)
	{

		TRACE (12, (" socket number is not found!\n "));

		return -1;
	}

	//-- 开始分析报文头 --//
	int haslen = 0;
	int framelen = 0;

	int buflen = ptrend - ptrstart;

	logRec (" bufleft, ptrstart, ptrend: %d, %d, %d\n", bufleft, ptrstart,
			ptrend);

	if (buflen >= 2)			//???FIXME
	{
		// get another packet length
		framelen = ((PPACKET_HDR) ptrstart)->len;
		haslen = 1;

		if (buflen >= framelen)
		{
			// 包括完整报文
			memcpy (pAppFrame, ptrstart, framelen);

			// 剩余数据拷回
			pthread_mutex_lock (&pcld->mutex);

			memcpy (pcld->unhandled_buf.buf, ptrstart + framelen,
					buflen - framelen);
			pcld->unhandled_buf.len = buflen - framelen;

			pthread_mutex_unlock (&pcld->mutex);

#ifdef DEBUG
			{
				PrintPkt (pAppFrame, framelen);
			}
#endif

			return framelen;
		}
	}

	int data_received;

	for (;;)
	{
		data_received =
			recv (s, ptrend, MAX_FRAME_LEN - (ptrend - bufleft), 0);

		if (data_received == -1)
		{
			perror ("recv");

			return -1;
		}

		//客户端关闭连接
		if (data_received == 0)
		{
			TRACE (6, ("recv zero\n"));

			return 0;
		}

		ptrend = ptrend + data_received;

		buflen = ptrend - ptrstart;
		if (haslen == 0 && buflen >= 2)
		{
			framelen = ((PPACKET_HDR) ptrstart)->len;
			haslen = 1;
		}

		if (haslen == 1 && framelen > MAX_FRAME_LEN)
		{
			TRACE (12,
				   (" !!!!!! Frame Len > MAX_FRAME_LEN , EXIT!!!!!! ...\n"));
			return -1;
		}

		if (buflen >= framelen)
		{
			break;
		}
	}

	// 得到完整报文
	memcpy (pAppFrame, ptrstart, framelen);

	// 剩余数据拷回
	pthread_mutex_lock (&pcld->mutex);

	memcpy (pcld->unhandled_buf.buf, ptrstart + framelen,
			buflen - framelen);
	pcld->unhandled_buf.len = buflen - framelen;

	pthread_mutex_unlock (&pcld->mutex);

#ifdef DEBUG
	{
		PrintPkt (pAppFrame, framelen);
	}
#endif

	return framelen;
}
