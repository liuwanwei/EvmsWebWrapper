#include "EvmsCmd.h"
#include "Protocol.h"
#include "Debuger.h"

#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "evms/SendOneFrame.h"
#include "evms/evms.h"
#include "evms/PrivateData.h"

// #include "test/test.h"
//

static char *s_evms = "evms";
static char *s_delim = " ";

/*
static int shell_execute(char *oneframe, int len)
{
    TRACE(10, ("### Enter shell_execute ###"));

    // My_Printf("hahaah");

    char *cmd;

    cmd = oneframe + sizeof(PACKET_HDR);

    TRACE(10,
	  ("cmd = %s, app data len = %lu\n", cmd,
	   len - sizeof(PACKET_HDR)));

    int pid;

    pid = fork();

    if (pid < 0) {
	perror("fork");
	return -1;
    } else if (pid == 0) {
	char *argument[4];
	char command[64];
	extern char **environ;
	snprintf(command, 64, "./evms.sh %s", cmd);
	argument[0] = "sh";
	argument[1] = "-c";
	argument[2] = command;
	argument[3] = NULL;

	execve("/bin/bash", argument, environ);

	return -1;
    } else {
	int status;
	int ret;
	waitpid(pid, &status, 0);
	ret = WEXITSTATUS(status);

	return (ret == 255 ? 0 : ret);
    }


    TRACE(10, ("### Leave shell_execute ###"));

    return 0;
}
*/

static int
init_evms_cli (int sock_fd, char *oneframe, int len)
{
	TRACE (10, ("### Enter init_evms_cli ###"));

	char *frame_start;
	char *save_ptr;
	char *token;
	char *token_table[MAX_EVMS_COMMAND_LINE_PARAM];
	int token_num = 1;

	frame_start = oneframe + sizeof (PACKET_HDR);

	// Set "evms" as the default parameter
	token_table[0] = s_evms;

	token = strtok_r (frame_start, s_delim, &save_ptr);
	if (NULL != token)
	{

		token_table[token_num] = token;
		token_num++;

		while ((token =
				strtok_r (frame_start, s_delim, &save_ptr)) != NULL)
		{
			token_table[token_num] = token;
			token_num++;

			if (token_num > MAX_EVMS_COMMAND_LINE_PARAM)
			{
				TRACE (10, (" Evms param number beyond max length !"));
				break;
			}
		}

	}

	TRACE (10, ("### Leave init_evms_cli ###"));

	// every evms control link will blocked here 
	return evms_init (token_num, token_table, sock_fd);
}

static int
handle_evms_cli_command (char *cmd, int len)
{
	int ret = -1;

	TRACE (10, ("### Enter handle_evms_cli_command ###"));

	ret = evms_func (cmd, len);

	TRACE (10, ("### Leave handle_evms_cli_command ###"));

	return ret;
}

static int
deinit_evms_cli(int sock_fd)
{
	int ret = -1;
	TRACE (10, ("### Enter deinit_evms_cli ###"));

	ret = evms_deinit();

	TRACE (10, ("### Leave deinit_evms_cli ###"));

	return ret;
}

int
EvmsCmd (int sock_fd, char *oneframe, int len)
{
	TRACE (10, ("### Enter EvmsCmd ###"));

	int ret = -1;

	switch (((PACKET_HDR *) oneframe)->subtype)
	{
	case EVMS_INIT_MSG:
		ret = init_evms_cli (sock_fd, oneframe, len);
		break;
	case EVMS_COMMAND_MSG:
		ret = handle_evms_cli_command (oneframe + sizeof (PACKET_HDR),
									   len - sizeof (PACKET_HDR));
		/* Clear reserved private data used in one cmommand */
		ClearNode();
		break;
	case EVMS_DEINIT_MSG:
		ret = deinit_evms_cli(sock_fd);
		break;
	default:
		break;
	}


	TRACE (10, ("### Leave EvmsCmd ###"));

	if (SendEvmsFrame
		(sock_fd, (unsigned short) ret, NULL, 0,
		 ((PACKET_HDR *) oneframe)->subtype) <= 0)
	{
		return -1;
	}

	return 0;
}
