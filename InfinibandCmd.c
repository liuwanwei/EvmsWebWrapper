#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "Debuger.h"
#include "InfinibandCmd.h"
#include "Infiniband.h"

int
InfinibandCmd (int sock_fd, char *oneframe, int len)
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

	switch (hdr->subtype)
	{
	case IB_ADD_GROUP:
		ret = AddGroup(msg_body, msg_body_len);
		break;
	case IB_DEL_GROUP:
		ret = DelGroup(msg_body, msg_body_len);
		break;
	case IB_GET_ALL_GROUPS:
		ret = GetAllGroups(&reply, &reply_len);
		break;
	case IB_ADD_DEVICE_TO_GROUP:
		ret = AddDeviceToGroup(msg_body, msg_body_len, &reply, &reply_len);
		break;
	case IB_DEL_DEVICE_FROM_GROUP:
		ret = DelDeviceFromGroup(msg_body, msg_body_len);
		break;
	case IB_GET_DEVICES_FROM_GROUP:
		ret = GetDevicesFromGroup(msg_body, msg_body_len, &reply, &reply_len);
		break;
	case IB_GET_ALL_IB_PORTS:
		ret = GetAllIBPorts(&reply, &reply_len);
		break;
	case IB_ADD_IB_PORT_TO_GROUP:
		ret = AddIBPortToGroup(msg_body, msg_body_len);
		break;
	case IB_DEL_IB_PORT_FROM_GROUP:
		ret = DelIBPortFromGroup(msg_body, msg_body_len);
		break;
	case IB_GET_IB_PORTS_FROM_GROUP:
		ret = GetIBPortsFromGroup(msg_body, msg_body_len, &reply, &reply_len);
	default:
		break;
	}


	TRACE (10, ("### Leave EvmsCmd ###"));

	if (SendIBFrame
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


int SendIBFrame(int sock_fd, unsigned short retcode, char * data, int data_len, int sub_type)
{
	printf("### Enter SendIBFrame ###\n");

	char one_frame[MAX_FRAME_LEN];

	if (-1 == sock_fd)
	{
		printf("sock_fd == -1\n");
		return -1;
	}


	PACKET_HDR hdr;

	hdr.len = PACKET_HDR_LEN + data_len;
	hdr.type = MSG_TYPE_IB;
	hdr.subtype = sub_type;
	hdr.retcode = retcode;

	memcpy (one_frame, &hdr, PACKET_HDR_LEN);
	memcpy (one_frame + PACKET_HDR_LEN, data, data_len);

	// now we change data_len's real meaning
	data_len = PACKET_HDR_LEN + data_len;

	if (data_len != send (sock_fd, one_frame, data_len, 0))
	{
		perror ("send");
		return -1;
	}

	printf("### Leave SendIBFrame ###\n");

	return hdr.len;
}
