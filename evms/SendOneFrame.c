#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <string.h>

#include "SendOneFrame.h"
#include "../Protocol.h"

int
SendEvmsFrame (int sock_fd, unsigned short retcode, char *data,
			   int data_len, int sub_type)
{
	printf("### Enter evms/SendEvmsFrame ###\n");

	char one_frame[MAX_FRAME_LEN];

	if (-1 == sock_fd)
	{
		printf("sock_fd == -1\n");
		return -1;
	}


	PACKET_HDR hdr;

	hdr.len = sizeof (PACKET_HDR) + data_len;
	hdr.type = MSG_TYPE_EVMS;
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

	printf("### Leave evms/SendEvmsFrame ###\n");

	return hdr.len;
}
