
/* -- $Id: DeadThreadTbl.cpp,v 1.7.2.1 2004/06/22 05:59:36 yuzhe Exp $ -- */
	 
	
#include <string.h>
#include <stdio.h>
	
#include "Debuger.h"
	
#include "DeadThreadTbl.h"
#include "NetServer.h"
	DEAD_THREAD_TBL DeadThreadTbl;	// 死亡线程列表
int
InitDeadThreadTbl () 
{
	memset (&DeadThreadTbl.tid_tbl, '\0', sizeof (DeadThreadTbl.tid_tbl));
	pthread_mutex_init (&DeadThreadTbl.mutex, NULL);
	return 0;
}
int
DeinitDeadThreadTbl () 
{
	pthread_mutex_destroy (&DeadThreadTbl.mutex);
	memset (&DeadThreadTbl, '\0', sizeof (DeadThreadTbl));
	return 0;
}


#ifdef DEBUG
	int
TestDeadThreadTbl () 
{
	printf (" ## DEAD TABLE ## \n");
	printf ("  index     tid   \n");
	printf (" -------  ------- \n");
	int i;

	for (i = 0; i < MAX_CONN_NUM; i++)
		
	{
		printf (" %7d  %7ld\n", i, DeadThreadTbl.tid_tbl[i]);
	}
	printf ("\n\n");
	return 0;
}


#endif	/*  */
	int
AddToDeadThreadTbl () 
{
	int i;

	
		//-- “死亡列表”登记 --//
		for (i = 0; i < MAX_CONN_NUM; i++)
		
	{
		int is_done = 0;

		pthread_mutex_lock (&DeadThreadTbl.mutex);
		if (DeadThreadTbl.tid_tbl[i] == 0)
			
		{
			DeadThreadTbl.tid_tbl[i] = pthread_self ();
			is_done = 1;
			TRACE (5, (" ;-)!\n"));
		}
		pthread_mutex_unlock (&DeadThreadTbl.mutex);
		if (is_done == 1)
			break;
	}
	return 0;
}


