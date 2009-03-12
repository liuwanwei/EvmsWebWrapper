/* -- $Id: Debuger.h,v 1.12 2004/04/30 03:20:20 qianjinqi Exp $ -- */

#ifndef _DEBUGER_H_
#define _DEBUGER_H_

// FIXME #define _DEBUG_

#ifdef _DEBUG_

#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>

//#define NULL 0x00000000L

class CDebuger
{
private:
	static CDebuger *pDebuger;
	int m_debug_level;

public:
	CDebuger();
	~CDebuger();
	static CDebuger *Instance();
	int GetDebugLevel();
	void SetDebugLevel(int);
};


#define TRACE(level, info){	\
	CDebuger *pd = CDebuger::Instance();	\
	int g_debug_level = pd->GetDebugLevel();	\
	time_t g_t;	\
	g_t = time(NULL);	\
	if(g_debug_level >= level){	\
		printf("\nProcess %d on %s:%d@%s ---- ", getpid(), ctime(&g_t), __LINE__, __FILE__);	\
		printf info;	\
		fflush(stdout);	\
	}	\
}


#define PRINT(level, info){	\
	CDebuger *pd = CDebuger::Instance();	\
	int g_debug_level = pd->GetDebugLevel();	\
	if(g_debug_level >= level){	\
		printf info;	\
		fflush(stdout);	\
	}	\
}

#define OUTPUT(info){	\
	time_t g_t;	\
	g_t = time(NULL);	\
	printf("\nProcess %d on %s:%d@%s ---- ", getpid(), ctime(&g_t), __LINE__, __FILE__);	\
	printf info;	\
	fflush(stdout);	\
}

#else 

#define TRACE(level, info){	\
	printf info;	\
	printf("\n");\
}

#define PRINT(level, info){	\
	;	\
}

#define OUPUT( info){	\
	;	\
}

#endif



#endif
