
/* -- $Id: SysCheckInCmd.cpp,v 1.12.2.1.4.3 2004/07/22 11:14:41 yuzhe Exp $ -- */
	 
	
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <string.h>
	
#include <unistd.h>
#include <sys/types.h>
	
#include <sys/socket.h>
#include <netinet/in.h>
	
#include "SysCheckInCmd.h"
#include "SysConfExt.h"
	
#include "NetLog.h"
#include "Debuger.h"
	
#include "CtrlLinkTbl.h"
#include "Protocol.h"
	
SysCheckInCmd (int sock_fd) 
{
	
		//-- �Ǽǿͻ������� --//
	int i;
	
	
	
	

	
	
	
	
	
	
		
	{
		
			
		{
			
				
			{
				
					// repeated link
					TRACE (6,
						   ("Repeated connection from same ip address\n"));
				
			
		
		
		else if (-1 == pos)
			
		{
			
		
	
	
		
	{
		
			// Link Table is full
			TRACE (6, ("Link Ctrl Table is full!\n"));
		
	
	
	else
		
	{
		
			// �ҵ��ձ���
			pcld = CtrlLinkTbl;
		
		
		
			// strncpy(pcld->username, username, MAX_USRNAME_LEN);
			pcld->socket_fd = sock_fd;
		
		
			/* pthread_self
			 * PURPOSE
			 * return identifier of current thread
			 */ 
			pcld->ctrl_tid = pthread_self ();
		
	
	
  
	


