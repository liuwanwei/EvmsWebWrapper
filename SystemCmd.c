#include "SystemCmd.h"
#include "Protocol.h"
#include "stdlib.h"
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/socket.h>

#include "Functions.h"

// shell scripts used here.
static const char * s_system_cmd_script = "/usr/sbin/sanager/System.sh";

// directory content file
static const char * list_dir_file = "/tmp/dir_content_list";

int ListDirectory(char *, int, char **, int *);
int CreateDirectory(char *, int);

int SystemCmd(int sock_fd, char * oneframe, int len)
{
	int ret = -1;
	char * msg_body;
	int msg_body_len;
	char * reply = NULL;
	int reply_len = 0;
	PPACKET_HDR hdr;
	
	hdr = (PPACKET_HDR)oneframe;
	msg_body = oneframe + PACKET_HDR_LEN;
	msg_body_len = len - PACKET_HDR_LEN;

	switch(hdr->subtype)
	{
		case SYSTEM_LS_DIR:
			ret = ListDirectory(msg_body, msg_body_len, &reply, &reply_len);
			break;
		case SYSTEM_CR_DIR:
			ret = CreateDirectory(msg_body, msg_body_len);
			break;
		default:
			break;
	}


	if (SendAppFrame(sock_fd, 
			MSG_TYPE_IB,
			((PACKET_HDR *) oneframe)->subtype,
			(unsigned short) ret, 
			reply, 
			reply_len) <= 0)
	{
		if(NULL != reply)
		{
			free(reply);
		}

		return -1;
	}

	if(NULL != reply)
	{
		free(reply);
	}

	return 0;
}

int ListDirectory(char * msg_body, int msg_body_len, char ** reply, int * reply_len)
{
	char names_buffer[NET_BUFFER_LEN + 1];
	int names_buffer_len = 0;
	int name_len = 0;
	int file_num = 0;
        int pid;

        pid = fork();

        if (pid < 0)
        {
                return  - 1;
        }
        else if (pid == 0)
        {
                char *argument[4];
                char command[64];
                extern char **environ;
                snprintf(command, 64, "%s GetDirContent %s", s_system_cmd_script, msg_body);
                argument[0] = "sh";
                argument[1] = "-c";
                argument[2] = command;
                argument[3] = NULL;

		printf("%s\n", command);

                execve("/bin/sh", argument, environ);
                exit(1);
        }
        else
        {
		int ret;
                int status;
		char line_buffer[256];
		FILE * file_p = NULL;

                waitpid(pid, &status, 0);
                ret = WEXITSTATUS(status);

		if(0 != ret)
		{
			printf("script function %s::ls-l-h() failed!\n", s_system_cmd_script);
			return 0;
		}

		if(0 != access(list_dir_file, F_OK))
		{
			return 0;
		}

		file_p = fopen(list_dir_file, "r+");
        	if (NULL == file_p)
        	{
                	return 0;
        	}

        	memset(names_buffer, 0, NET_BUFFER_LEN + 1);

        	/* 每一个while 循环处理一行 */
        	while (NULL != fgets(line_buffer, 256, file_p))
        	{
	
                	name_len = strlen(line_buffer) + 1;
                	if (1 == name_len)
                	{
                        	break;
                	}

			if(names_buffer_len + name_len >= NET_BUFFER_LEN)
			{
				printf("%s 's size if greater than %d\n", list_dir_file, NET_BUFFER_LEN);
				break;
			}
			
			memcpy(names_buffer + names_buffer_len, line_buffer, name_len);

			names_buffer_len += name_len;

			file_num ++;
        	}

		fclose(file_p);

		if(file_num > 0)
		{
			(* reply) = (char *) malloc(names_buffer_len);
			memcpy((*reply), names_buffer, names_buffer_len);
			(* reply_len) = names_buffer_len;
		}

		return file_num;
        }

	return 0;
}

int CreateDirectory(char * msg_body, int msg_body_len)
{
        int pid;

        pid = fork();

        if (pid < 0)
        {
                return  - 1;
        }
        else if (pid == 0)
        {
                char *argument[4];
                char command[64];
                extern char **environ;
                snprintf(command, 64, "%s CreateDirectory %s", s_system_cmd_script, msg_body);
                argument[0] = "sh";
                argument[1] = "-c";
                argument[2] = command;
                argument[3] = NULL;
                execve("/bin/sh", argument, environ);
                exit(1);
        }
        else
        {
                int status;

                waitpid(pid, &status, 0);
                return WEXITSTATUS(status);

        }

	return 0;
}


/*
int SendCommonFrame(int sock_fd, unsigned short retcode, char * data, int data_len, char main_type, char sub_type)
{
	printf("### Enter SendCommonFrame ###\n");

	char one_frame[MAX_FRAME_LEN];

	if (-1 == sock_fd)
	{
		printf("sock_fd == -1\n");
		return -1;
	}

	printf("len: %d\n", data_len);
	printf("data: %s\n", data);

	PACKET_HDR hdr;

	hdr.len = PACKET_HDR_LEN + data_len;
	hdr.type = main_type;
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

	printf("### Leave SendCommonFrame ###\n");

	return hdr.len;
}
*/
