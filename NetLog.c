
/* -- $Id: NetLog.cpp,v 1.1.2.1 2004/07/20 07:15:25 yuzhe Exp $ -- */  
	
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
	
#include "NetLog.h"
unsigned int lognum = 0;
int
logRec (const char *fmt, ...) 
{
	int cnt;

	
#ifdef DEBUG
		va_list args;
	FILE * fp;
	char msgbuf[300];

	time_t pt;
	struct tm *tm_struct;
	char buf[20];

	fp = fopen ("/tmp/vs.log", "a");
	if (fp == NULL)
		return (-1);
	va_start (args, fmt);
	pt = time (NULL);
	tm_struct = localtime (&pt);
	strftime (buf, 17, "%b %d %H:%M:%S", tm_struct);
	sprintf (msgbuf, "[%06d][%s]", lognum++, buf);
	cnt = vsprintf (&msgbuf[25], fmt, args);
	va_end (args);
	fwrite (msgbuf, strlen (msgbuf), 1, fp);
	fclose (fp);
	
#endif	/*  */
		return (cnt);
}


