
/* -- $Id: LinkCtrl.cpp,v 1.12.2.4.4.6 2004/08/13 00:56:27 yuzhe Exp $ -- */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

#include <sys/socket.h>

#include "NetLog.h"
#include "Debuger.h"

#include "LinkCtrl.h"

#include "RecvOneFrame.h"
#include "Protocol.h"

#include "CtrlLinkTbl.h"
#include "UnknownRequest.h"
#include "SysCheckInCmd.h"

#include "HandShakeCmd.h"
#include "EvmsCmd.h"
#include "InfinibandCmd.h"
#include "IscsiCmd.h"
#include "SystemCmd.h"

PktHandler g_PktHandler[MSG_TYPE_NUM] = {
    GetUnknowCmd,
    HandShakeCmd,
    EvmsCmd,
    InfinibandCmd,
    IscsiCmd,
    SystemCmd
};

static void Link_Ctrl_Cleanup(void *pinfo)
{
    TRACE(12,
	  (" ####### Processing LINK_CTRL Cleanup Start! ######### \n"));

    int sock_fd = *(int *) pinfo;

    //停止所有关联数据线程
    if (-1 == DelFromLinkTbl(sock_fd)) {
	TRACE(12, (" Delete sock_fd from link table failed! "));
    }

    if (close(sock_fd) == -1) {
	perror("close_mid");
    }

    TRACE(12,
	  (" ####### Processing NET_SERVER Cleanup Finished! ######### \n"));

    return;
}

void print_frame_details(char * frame, int len)
{
	int i = 0;

	printf("LEN   T  S  ODR   RET   CONTENT\n");

	for(i = 0; i < len; i ++)
	{
		printf("%02x ", frame[i]);
	}

	printf("\n");
}

/***************************************************************************************
* 客户控制连接处理函数，成功返回0，失败返回-1。
***************************************************************************************/
int Link_Ctrl(int socket_fd)
{
    pthread_cleanup_push(Link_Ctrl_Cleanup, &socket_fd);

    int len = 0;

    TRACE(12, ("Enter Link_Ctrl()!\n"));

    if (SysCheckInCmd(socket_fd) != 0) {
	close(socket_fd);
	return 0;
    }
    //开始处理应用报文。
    PktHandler func;
    char one_frame[MAX_FRAME_LEN];

    while ((len = RecvOneFrame(socket_fd, one_frame)) > 0) {
	TRACE(10,
	      ("Receive a command! : %d ------- length : %d\n",
	       ((PPACKET_HDR) one_frame)->type, len));

	if (((PPACKET_HDR) one_frame)->type > 0
	    && ((PPACKET_HDR) one_frame)->type <= MSG_TYPE_NUM) {

	    print_frame_details(one_frame, ((PPACKET_HDR) one_frame)->len);

	    func = g_PktHandler[((PPACKET_HDR) one_frame)->type];
	    if (NULL != func) {
		func(socket_fd, (char *) one_frame,
		     ((PPACKET_HDR) one_frame)->len);
	    }
	} else {
	    TRACE(10, ("Error Message Type!\n"));
	}
    }

    TRACE(2, ("Control channel is eixt!\n"));

    pthread_cleanup_pop(1);	/* Link_Ctrl_Cleanup(socket_fd) */

    return 0;
}
