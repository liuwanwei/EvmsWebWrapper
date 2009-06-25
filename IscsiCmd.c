#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "IscsiCmd.h"
#include "Debuger.h"

// If we need the error message, every function need reply and reply_len param
int AddIscsiTarget(char * msg_body, int msg_body_len);
int DelIscsiTarget(char * msg_body, int msg_body_len);
int GetAllIscsiTargets(char ** reply, int * reply_len);
int IscsiTargetAccessCtrl(char * msg_body, int msg_body_len);

int SendIscsiFrame(int, unsigned short , char * , int , int );

int
IscsiCmd (int sock_fd, char *oneframe, int len)
{
	TRACE (10, ("### Enter IBCmd ###"));

	int ret = -1;
	PPACKET_HDR hdr;
	char * msg_body;
	int msg_body_len;
	char * reply = NULL;
	int reply_len = 0;

	hdr = (PPACKET_HDR)oneframe;

	msg_body = oneframe + PACKET_HDR_LEN;
	msg_body_len = len - PACKET_HDR_LEN;

	switch(hdr->subtype)
	{
	case ISCSI_ADD_TARGET:
		ret = AddIscsiTarget(msg_body, msg_body_len);
		break;
	case ISCSI_DEL_TARGET:
		ret = DelIscsiTarget(msg_body, msg_body_len);
		break;
	case ISCSI_GET_ALL_TARGETS:
		ret = GetAllIscsiTargets(&reply, &reply_len);
		break;
	case ISCSI_TARGET_ACCESS_CTRL:
		ret = IscsiTargetAccessCtrl(msg_body, msg_body_len);
		break;
	default:
		break;
	}

	TRACE (10, ("### Leave EvmsCmd ###"));

	if (SendIscsiFrame
		(sock_fd, (unsigned short) ret, reply, reply_len,
		 ((PACKET_HDR *) oneframe)->subtype) <= 0)
	{
		if(NULL != reply)
		{
			free(reply);
		}
		
		return -1;
	}

        /* 切记不要忘了释放动态分配的内存*/
	if(NULL != reply)
	{
		free(reply);
	}

	return 0;
}

int SendIscsiFrame(int sock_fd, unsigned short retcode, char * data, int data_len, int sub_type)
{
	printf("### Enter SendIscsiFrame###\n");

	char one_frame[MAX_FRAME_LEN];

	if (-1 == sock_fd)
	{
		printf("sock_fd == -1\n");
		return -1;
	}


	PACKET_HDR hdr;

	hdr.len = PACKET_HDR_LEN + data_len;
	hdr.type = MSG_TYPE_ISCSI;
	hdr.subtype = sub_type;
	hdr.retcode = retcode;

	memcpy (one_frame, &hdr, PACKET_HDR_LEN);
	memcpy (one_frame + PACKET_HDR_LEN, data, data_len);

	// now we change data_len's real meaning
	data_len = PACKET_HDR_LEN + data_len;

	if (data_len != my_send (sock_fd, one_frame, data_len))
	{
		perror ("send");
		return -1;
	}

	printf("### Leave SendIscsiFrame ###\n");

	return hdr.len;
}


int AddIscsiTarget(char * msg_body, int msg_body_len)
{
	return 0;
}
int DelIscsiTarget(char * msg_body, int msg_body_len)
{
	return 0;
}

int GetAllIscsiTargets(char ** reply, int * reply_len)
{
	return 0;
}
int IscsiTargetAccessCtrl(char * msg_body, int msg_body_len)
{
	return 0;
}
