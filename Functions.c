#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "Protocol.h"
#include "Functions.h"
        

int SendAppFrame(int sock_fd, 
		unsigned char type, 
		unsigned char sub_type, 
		unsigned short retcode, 
		char * data, 
		unsigned char data_len)
{
	printf("### Enter SendAppFrame ###\n");

	char one_frame[MAX_FRAME_LEN];

	if (-1 == sock_fd)
	{
		printf("sock_fd == -1\n");
		return -1;
	}


	PACKET_HDR hdr;

	hdr.len = PACKET_HDR_LEN + data_len;
	hdr.type = type;
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

	printf("### Leave SendAppFrame ###\n");

	return hdr.len;
}

#define MAX_SHELL_CMD_LEN       256
int CallShell(const char * shell_script_name, 
              const char * shell_func_name, 
              char * param_list)
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
                char command[MAX_SHELL_CMD_LEN];
                extern char **environ;

                if(NULL == param_list)
                {
                        snprintf(command, 
                                MAX_SHELL_CMD_LEN, 
                                "%s %s",
                                shell_script_name,
                                shell_func_name);
                }
                else
                {
                        snprintf(command, 
                                MAX_SHELL_CMD_LEN, 
                                "%s %s \"%s\"",
                                shell_script_name,
                                shell_func_name,
                                param_list);
                }

                argument[0] = "sh";
                argument[1] = "-c";
                argument[2] = command;
                argument[3] = NULL;
                printf("pl command: %s\n", command);
                execve("/bin/bash", argument, environ);
                exit(1);
        }
        else
        {
                int status;
                waitpid(pid, &status, 0);

                return WEXITSTATUS(status);
        }
}



int GetLinedupResultsFromFile(const char * filename, 
                              char ** reply, 
                              int * reply_len)
{
        if(NULL == filename
        || NULL == reply
        || NULL == reply_len)
        {
                return -1;
        }

        // Get results from /tmp/file
        FILE * file = NULL;
        char response[NET_BUFFER_LEN + 1];
        char line_buf[SHORT_BUFFER_LEN];
        char line_len = 0;
        int response_len = 0;
        
        file = fopen(filename, "r+");

        if(NULL == file)
        {
                return -1;
        }

        while(NULL != fgets(line_buf, SHORT_BUFFER_LEN, file))
        {
                if(0 == (line_len = strlen(line_buf)))
                {
                        continue;
                }

                if(response_len + line_len > NET_BUFFER_LEN)
                {
                        printf("NET_BUFFER_LEN(%d) not enough!\n", NET_BUFFER_LEN);
                        break;
                }

                memcpy(response + response_len, line_buf, line_len);
                response_len += line_len;
                response[response_len] = '\0';
                response_len ++;
        }

        fclose(file);

        (*reply) = (char *)malloc(response_len);
        if(NULL == (*reply))
        {
                return -1;
        }
        // assert(NULL != (*reply));

        memcpy(*reply, response, response_len);

        *reply_len = response_len;

        return 0;
}
