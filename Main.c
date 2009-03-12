
/* -- $Id: Main.cpp,v 1.12 2004/04/30 03:20:23 qianjinqi Exp $ -- */  
	
#include <stdlib.h>
#include <stdio.h>
	
// #include "ServerCtrl.h"
	
// #include "Log.h"
#include "Debuger.h"
#include "NetServer.h"
#include "Infiniband.h"
#include "evms/PrivateData.h"
	
//-- Ö÷³ÌÐò --//
	int
main (int argc, char *argv[]) 
{
	int debug_level = 0;

	if (argc > 1)
	{
		debug_level = atoi (argv[1]);
	}

	if(0 != CheckRunningEnvironment())
	{
		printf("*** IB working environment not ready! ***\n\n");
	}

	StartNetServer (debug_level);

	InitHash();
	
	//blocked here
	for (;;)
	{
		if (getchar () == '#')
		{
			StopNetServer ();
			break;
		}
	}

	DestroyHash();

	return 0;
}

