#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "IscsiCmd.h"
#include "Debuger.h"

// Iscsi shell script path
const char * s_iscsi_script = "/usr/sbin/sanager/Iscsi.pl";
const char * s_iscsi_add_target_func    = "AddTarget";
const char * s_iscsi_del_target_func    = "DelTarget";
const char * s_iscsi_get_all_targets    = "GetAllTargets";
const char * s_iscsi_target_access_ctrl = "TargetAccessCtrl";

static const char * s_return_value_file = "/tmp/return_value_iscsi";

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

#define MAX_SHELL_CMD_LEN	256
int CallShell(const char * shell_script_name, const char * shell_func_name, char * param_list)
{
        int pid;
	int cmd_len = 0;

	if(NULL == shell_script_name
	|| NULL == shell_func_name)
	{
		return -1;
	}

	cmd_len = strlen(shell_script_name) + strlen(shell_func_name);
	if(NULL != param_list)
	{
		cmd_len += strlen(param_list);
	}

	if(cmd_len >= MAX_SHELL_CMD_LEN - 2)
	{
		printf("Shell command is two long to stored in stack memory\n");
		return -1;
	}

        pid = fork();

        if (pid < 0)
        {
                return  - 1;
        }
        else if (pid == 0)
        {
                char * argument[4];
                // char command[MAX_SHELL_CMD_LEN];
                extern char **environ;

                argument[0] = (char *)shell_script_name;
                argument[1] = (char *)shell_func_name;
                argument[2] = param_list;
		argument[3] = NULL;
                execve("/usr/bin/perl", argument, environ);
                exit(1);
        }
        else
        {
                int status;
                waitpid(pid, &status, 0);
                return WEXITSTATUS(status);
        }
}


int AddIscsiTarget(char * msg_body, int msg_body_len)
{
	if(NULL == msg_body
	|| msg_body_len <= 0)
	{
		return -1;
	}

	return CallShell(s_iscsi_script, s_iscsi_add_target_func, msg_body);
}
int DelIscsiTarget(char * msg_body, int msg_body_len)
{
	if(NULL == msg_body
	|| msg_body_len <= 0)
	{
		return -1;
	}

	return CallShell(s_iscsi_script, s_iscsi_del_target_func, msg_body);
}

int GetAllIscsiTargets(char ** reply, int * reply_len)
{
	if(0 != CallShell(s_iscsi_script, s_iscsi_get_all_targets, NULL))
	{
		return -1;
	}

	// Get results from /tmp/file
	FILE * file = NULL;
	file = fopen(s_return_value_file, "r+");

	if(NULL != file)
	{
		fclose(file);
	}
	
	return 0;
}
int IscsiTargetAccessCtrl(char * msg_body, int msg_body_len)
{
	if(NULL == msg_body
	|| msg_body_len <= 0)
	{
		return -1;
	}

	return CallShell(s_iscsi_script, s_iscsi_target_access_ctrl, msg_body);
}
