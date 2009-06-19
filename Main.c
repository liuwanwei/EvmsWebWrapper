
/* -- $Id: Main.cpp,v 1.12 2004/04/30 03:20:23 qianjinqi Exp $ -- */  
	
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
	
// #include "ServerCtrl.h"
	
// #include "Log.h"
#include "Debuger.h"
#include "NetServer.h"
#include "Infiniband.h"
#include "evms/PrivateData.h"

#define SANAGER_WORKING_DIR 	"/usr/sbin/sanager/"

int g_debug_level = 0;
char g_pidfile[256];

void catch_hup_sig(int sig)
{
	printf("catched a HUP signal.\n");
}

void catch_term_sig(int sig)
{
	StopNetServer();
	DestroyHash();
}

pid_t get_running_pid(void)
{
	pid_t	pid = 0;
	FILE*	lockfd = NULL;

	if ((lockfd = fopen(g_pidfile, "r"))!=NULL &&	
		fscanf(lockfd, "%d", &pid) == 1 && pid > 0) 
	{
		if (kill(pid, 0)>=0 || errno!=ESRCH) 
		{
			fclose(lockfd);
			return(pid);
		}
	}
	if (lockfd != NULL) 
	{
		fclose(lockfd);
	}
	
	return(-1);
}

int daemon_start(void)
{
	pid_t		pid;
	FILE *		lockfd;
	sigset_t	sighup;
    	const char cmdname[] = "net_server";

	sprintf(g_pidfile, "/var/log/net_server.pid");
	
	if ((pid=get_running_pid()) > 0 && pid != getpid()) 
	{
		fprintf(stderr, "%s: already running [pid %d].\n"
		,cmdname, pid);
		return(-1);
	}

	/* Guess not. Go ahead and start things up */
	pid = fork();
	if (pid < 0) 
	{
		fprintf(stderr, "%s: could not start daemon\n",	cmdname);
		perror("fork");
		exit(-1);
	}
	else if (pid > 0) 
	{
		fprintf(stderr, "- Daemon started, PID is %d.\n", (int) pid);
		exit(0);
	}

	pid = getpid();

	lockfd = fopen(g_pidfile, "w");
	if (lockfd != NULL) 
	{
		fprintf(lockfd, "%d\n", pid);
		fclose(lockfd);
	}
	if (setsid() < 0) 
	{
		fprintf(stderr, "%s: could not start daemon\n", cmdname);
		perror("setsid");
	}

	sigemptyset(&sighup);
	sigaddset(&sighup, SIGHUP);
	if (sigprocmask(SIG_UNBLOCK, &sighup, NULL) < 0) {
		fprintf(stderr, "%s: could unblock SIGHUP signal\n", cmdname);
	}

	(void)signal(SIGHUP, 	catch_hup_sig);
	(void)signal(SIGTERM, 	catch_term_sig);
	umask(022);

	chdir(SANAGER_WORKING_DIR);

	return(0);
}
	
//-- ������ --//
	int
main (int argc, char *argv[]) 
{
	int c;
	int running_foreground = 0;

	while((c = getopt(argc, argv, "d:f")) != -1)
	{
		switch(c)
		{
			case 'd':
				g_debug_level = atoi(optarg);
				break;
			case 'f':
				running_foreground = 1;
				break;
			default:
				break;
		}
	}

	if(0 != CheckRunningEnvironment())
	{
		printf("*** IB working environment not ready! ***\n\n");
	}

	// start as daemon process
	if(! running_foreground)
	{
		if(0 != daemon_start())
		{
			return 0;
		}
	}

	InitHash();

	// blocked here
	StartNetServer();

	return 0;
}


