#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "Protocol.h"
#include "PrintObject.h"
#include "evms/ObjectStructs.h"

void ShowUsage(const char *prog)
{
    printf("usage : %s -t [evms][ib][iscsi] command\n", prog);
}

int Connect(const char *server_ip, unsigned short port);
void DisConnect(int sock_fd);
int SendEvmsCmd(int sock_fd, char *cmd, int cmd_len);
int SendIBCmd(int sock_fd, char *cmd, int cmd_len);

int main(int argc, char *argv[])
{
    if (argc < 4) {
	ShowUsage(argv[0]);
	return 0;
    }

    char cmd[128];
    int cmd_len = 0;
    int msg_type = MSG_TYPE_UNKNOWN;
    int begin_get_command = 0;
    int parameter_index;

    memset(cmd, '\0', 128);

    for (parameter_index = 1; parameter_index < argc; parameter_index++) {
	if (begin_get_command >= 1) {
	    if (begin_get_command > 1) {
		cmd[cmd_len] = ' ';
		cmd_len++;
	    }

	    strcat(cmd + cmd_len, argv[parameter_index]);
	    cmd_len += strlen(argv[parameter_index]);

	    begin_get_command++;
	    continue;
	}

	if (0 == strcmp("-t", argv[parameter_index])) {
	    parameter_index++;
	    if (0 == strcmp("evms", argv[parameter_index])) {
		msg_type = MSG_TYPE_EVMS;
	    } else if (0 == strcmp("ib", argv[parameter_index])) {
		msg_type = MSG_TYPE_IB;
	    } else if (0 == strcmp("iscsi", argv[parameter_index])) {
		msg_type = MSG_TYPE_ISCSI;
	    } else {
		ShowUsage(argv[0]);
		return 0;
	    }

	    begin_get_command = 1;
	}
    }

    int sock_fd = -1;

    sock_fd = Connect("127.0.0.1", 6018);
    if (-1 == sock_fd) {
	return -1;
    }

    switch (msg_type) {
    case MSG_TYPE_EVMS:
	SendEvmsCmd(sock_fd, cmd, cmd_len);
	break;
    case MSG_TYPE_IB:
	SendIBCmd(sock_fd, cmd, cmd_len);
	break;
    default:
	break;
    }

    DisConnect(sock_fd);

    return 0;
}

int Connect(const char *server_ip, unsigned short port)
{
    int sock_fd;
    struct sockaddr_in server;

    if (-1 == (sock_fd = socket(PF_INET, SOCK_STREAM, 0))) {
	perror("socket");
	return -1;
    }

    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    inet_pton(AF_INET, server_ip, &server.sin_addr);

    if (-1 ==
	connect(sock_fd, (struct sockaddr *) &server, sizeof(server))) {
	perror("connect");
	return -1;
    }

    return sock_fd;
}

void DisConnect(int sock_fd)
{
    close(sock_fd);
}



/*
 * GetServerResponse()
 *
 * This function will be called each time after we sent one frame to server .
 * The data response from server may not be as ideal as frame by frame , 
 * There are three conditons:
 * 1, one frame on one recv() ;
 * 2, several frames on one recv();
 * 3, frame fragment on one recv(), for example: one 5161 bytes long frame, may
 * be cutted into one 386 bytes fragment and another 4772 bytes fragment
 * So, we need to check the buffer, and get frames one by one.
 */
int g_left_len = 0;
char g_one_frame[MAX_FRAME_LEN];

int GetServerResponse(int sock_fd, int response_msg_type)
{
    int msg_len = 0;
    int first_frame_len = 0;
    char *header;
    PPACKET_HDR hdr;
    PACKET_HDR send_frame;

    msg_len = g_left_len;

    // Main loop, get one data buffer after each loop
    while (1) {

	// we must receive at least PACKET_HDR_LEN bytes
	if (msg_len < PACKET_HDR_LEN) {
	    msg_len += recv(sock_fd,
			    g_one_frame + msg_len, MAX_FRAME_LEN - msg_len,
			    0);

	    if (msg_len < PACKET_HDR_LEN) {
		continue;
	    }
	}

	header = g_one_frame;
	first_frame_len = ((PPACKET_HDR) header)->len;

	// we must receive at least a full frame
	if (msg_len < first_frame_len) {
	    continue;
	}

	g_left_len = msg_len;

	// Buffer handler, get one frame each loop.
	while (g_left_len > 0) {
	    hdr = (PPACKET_HDR) header;

	    if (hdr->retcode != RETCODE_OK) {
		perror("command");
		return -1;
	    }

	    if (hdr->type == MSG_TYPE_EVMS) {
		if (hdr->subtype == EVMS_PROMPT_MSG) {
		    /* This message will only be display to user */
		    printf("%s\n", header + PACKET_HDR_LEN);
		} else if (hdr->subtype == EVMS_OBJECT_INFO) {
		    printf("Get EVMS_OBJECT_INFO type packet\n");

		    char *buffer;
		    int obj_len;
		    my_object_info_t obj;

		    buffer = header + PACKET_HDR_LEN;
		    obj_len = hdr->len - PACKET_HDR_LEN;
		    MY_UNPACK_EVMS_OBJECT(buffer, obj_len, &obj);

		    PrintObject(&obj);

		    MY_FREE_BASIC_INFO_T(obj.basic);
		    MY_FREE_ADVANCE_INFO_T(obj.advance);
		} else if (hdr->subtype == EVMS_NEED_USER_INPUT) {
		    /* 
		     * This message means that the server 
		     * is waiting for a response.
		     * Here, I fatch user input from "fgets".
		     * In web module, user input should be 
		     * fatched from visiable controls like 
		     * text edit or labels.
		     */
		    char choice[10];

		    /* Get the user input */
		    while (1) {
			fgets(choice, 10, stdin);

			if (strlen(choice) >= 10) {
			    perror("must at least 1 and at most 9 length");
			    continue;
			}

			break;
		    }

		    /* Send back user input to server */
		    send_frame.len = PACKET_HDR_LEN + strlen(choice) + 1;
		    send_frame.type = MSG_TYPE_EVMS;
		    send_frame.subtype = EVMS_NEED_USER_INPUT;
		    send_frame.retcode = RETCODE_OK;
		    send(sock_fd, &send_frame, PACKET_HDR_LEN, 0);
		    send(sock_fd, choice, strlen(choice) + 1, 0);
		}

		else if (((PPACKET_HDR) header)->subtype == EVMS_INIT_MSG) {
		    /* 
		     * This message will be sent from client(web) 
		     * to server(NetServer) once, 
		     * Then we can send EVMS_COMMAND_MSG to server.
		     */
		    printf("init ok\n");

		    if (EVMS_INIT_MSG == response_msg_type) {
			goto got_current_response;
		    }
		} else if (((PPACKET_HDR) header)->subtype ==
			   EVMS_DEINIT_MSG) {
		    printf("deinit ok\n");

		    if (EVMS_DEINIT_MSG == response_msg_type) {
			goto got_current_response;
		    }
		} else if (((PPACKET_HDR) header)->subtype ==
			   EVMS_COMMAND_MSG) {
		    /*
		     * Normal evms command, such as "q:v", 
		     * or "c:r, MDRaid1RegMgr={ver1_super ..." .
		     */
		    printf("command ok\n");

		    if (EVMS_COMMAND_MSG == response_msg_type) {
			goto got_current_response;
		    }
		} else {
		    printf("!!! error : unknown subtype %d !!!\n",
			   ((PPACKET_HDR) header)->subtype);
		    return -1;
		}


		/* if buffer isn't finished, jump to the next frame */
		g_left_len -= ((PPACKET_HDR) header)->len;
		header += ((PPACKET_HDR) header)->len;
	    } else if (hdr->type == MSG_TYPE_IB) {
		    switch(hdr->subtype)
		    {
			    /*
			    case IB_ADD_GROUP:
			    case IB_DEL_GROUP:
			    case IB_ADD_DEVICE_TO_GROUP:
			    case IB_DEL_DEVICE_FROM_GROUP:
			    case IB_ADD_IB_PORT_TO_GROUP:
			    case IB_DEL_IB_PORT_FROM_GROUP:
			    */
			    case IB_GET_ALL_GROUPS:
			    case IB_GET_DEVICES_FROM_GROUP:
			    case IB_GET_ALL_IB_PORTS:
			    case IB_GET_IB_PORTS_FROM_GROUP:
				    printf("Objects List Begin: \n");
				    printf("%s\n", g_one_frame + PACKET_HDR_LEN);
				    printf("Objects List End: \n");
				    break;
			    default:
				    break;
		    }

		    printf("IB command Success!");
	    }

	}

	msg_len = 0;
    }

  got_current_response:

    g_left_len -= ((PPACKET_HDR) header)->len;
    header += ((PPACKET_HDR) header)->len;

    if (g_left_len > 0) {
	if (g_left_len > MAX_FRAME_LEN / 2) {
	    printf("!!! left bytes is too long to reserve !!!");
	    return -1;
	}

	memcpy(g_one_frame, header, g_left_len);
    }

    return 0;
}

int SendEvmsCmd(int sock_fd, char *cmd, int cmd_len)
{
    if (-1 == sock_fd) {
	return -1;
    }

    PACKET_HDR hdr;

    // Send evms initialization packet
    hdr.len = PACKET_HDR_LEN;
    hdr.type = MSG_TYPE_EVMS;

    hdr.subtype = EVMS_INIT_MSG;

    if (PACKET_HDR_LEN != send(sock_fd, &hdr, PACKET_HDR_LEN, 0)) {
	perror("init");
	return -1;
    }

    GetServerResponse(sock_fd, EVMS_INIT_MSG);


    // Send evms command packet
    hdr.len = PACKET_HDR_LEN + cmd_len;
    hdr.subtype = EVMS_COMMAND_MSG;

    if (PACKET_HDR_LEN != send(sock_fd, &hdr, PACKET_HDR_LEN, 0)) {
	perror("send header");
	return -1;
    }

    if (cmd_len != send(sock_fd, cmd, cmd_len, 0)) {
	perror("command");
	return -1;
    }

    GetServerResponse(sock_fd, EVMS_COMMAND_MSG);

    hdr.len = PACKET_HDR_LEN;
    hdr.subtype = EVMS_DEINIT_MSG;

    if (PACKET_HDR_LEN != send(sock_fd, &hdr, PACKET_HDR_LEN, 0)) {
	perror("deinit");
	return -1;
    }

    GetServerResponse(sock_fd, EVMS_DEINIT_MSG);

    return hdr.len;
}

int SendIBCmd(int sock_fd, char *cmd, int cmd_len)
{
    if (-1 == sock_fd) {
	return -1;
    }

    if (NULL == cmd) {
	return -1;
    }

    char *ib_cmd_param = NULL;
    IB_RELATED_OPERATIONS ib_cmd_type = 0;

    if (!strcmp(cmd, "add_group")) {
	ib_cmd_type = IB_ADD_GROUP;
	cmd += 9;
    } else if (!strcmp(cmd, "del_group")) {
	ib_cmd_type = IB_DEL_GROUP;
	cmd += 9;
    } else if (!strcmp(cmd, "get_all_groups")) {
	ib_cmd_type = IB_GET_ALL_GROUPS;
	cmd += 14;
    } else if (!strcmp(cmd, "add_gdev")) {
	ib_cmd_type = IB_ADD_DEVICE_TO_GROUP;
	cmd += 8;
    } else if (!strcmp(cmd, "del_gdev")) {
	ib_cmd_type = IB_DEL_DEVICE_FROM_GROUP;
	cmd += 8;
    } else if (!strcmp(cmd, "get_gdevs")) {
	ib_cmd_type = IB_GET_DEVICES_FROM_GROUP;
	cmd += 9;
    } else if (!strcmp(cmd, "get_all_ports")) {
	ib_cmd_type = IB_GET_ALL_IB_PORTS;
	cmd += 12;
    } else if (!strcmp(cmd, "add_gport")) {
	ib_cmd_type = IB_ADD_IB_PORT_TO_GROUP;
	cmd += 9;
    } else if (!strcmp(cmd, "del_gport")) {
	ib_cmd_type = IB_DEL_IB_PORT_FROM_GROUP;
	cmd += 9;
    } else if (!strcmp(cmd, "get_gports")) {
	ib_cmd_type = IB_GET_IB_PORTS_FROM_GROUP;
	cmd += 10;
    }

    ib_cmd_param = cmd;
    while (*ib_cmd_param == ' ') {
	ib_cmd_param++;
    }

    /*
     * From now on, we begin send out messages
     */
    int param_len;
    PACKET_HDR hdr;

    // Send evms command packet

    param_len = strlen(ib_cmd_param);

    if(ib_cmd_type == IB_DEL_GROUP
    || ib_cmd_type == IB_ADD_GROUP
    || ib_cmd_type == IB_ADD_DEVICE_TO_GROUP
    || ib_cmd_type == IB_DEL_DEVICE_FROM_GROUP
    || ib_cmd_type == IB_GET_DEVICES_FROM_GROUP
    || ib_cmd_type == IB_ADD_IB_PORT_TO_GROUP
    || ib_cmd_type == IB_DEL_IB_PORT_FROM_GROUP
    || ib_cmd_type == IB_GET_IB_PORTS_FROM_GROUP)
    {
	    if(0 == param_len)
	    {
	    	printf("Invalid ib command!\n");
	    	return -1;
	    }
    }

    hdr.len = PACKET_HDR_LEN + param_len;
    hdr.type = MSG_TYPE_IB; 
    hdr.subtype = ib_cmd_type;

    if (PACKET_HDR_LEN != send(sock_fd, &hdr, PACKET_HDR_LEN, 0)) {
	printf("send ib command header error !\n");
	return -1;
    }

    if (cmd_len != send(sock_fd, ib_cmd_param, param_len, 0)) {
	printf("ib parameter error !\n");
	return -1;
    }

    GetServerResponse(sock_fd, ib_cmd_type);

    return hdr.len;
}
