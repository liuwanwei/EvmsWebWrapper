#include <stdio.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>

#include "Protocol.h"
#include "NetError.h"
#include "UnknownRequest.h"

int
GetUnknowCmd (int __sock_fd, char *__pPacket, int __len)
{
	PPACKET_HDR pPktHdr = (PPACKET_HDR) __pPacket;

	PACKET_HDR rspPkt;

	rspPkt.len = sizeof (rspPkt);
	rspPkt.type = pPktHdr->type;
	rspPkt.subtype = pPktHdr->subtype + 1;
	rspPkt.order = pPktHdr->order;
	rspPkt.retcode = ENET_UNKNOW_CMD;

	int ret;

	ret = my_send (__sock_fd, &rspPkt, sizeof (rspPkt));
	if (ret < 0)
	{
		perror ("send");
		fprintf (stderr, "!!! GetUnknowCmd send failed.\n");
	}

	return 0;
}
