
/* -- $Id: CtrlLinkTbl.cpp,v 1.12.2.2.4.2 2004/07/22 11:14:36 yuzhe Exp $ -- */
	 
	
#include <stdio.h>
#include <string.h>
	
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
	
#include "Debuger.h"
#include "CtrlLinkTbl.h"
#include "SysConfExt.h"
	

InitCtrlLinkTbl () 
{
	

	
	
		
	{
		
		
		
		
		
		
		
		
	
	


DeinitCtrlLinkTbl () 
{
	

	
	
		
	{
		
		
	
	



{
	
	

	
	
		
	{
		
			
		{
			
			
		
	
	
		
	
	else
		


GetLinkUser (int sock_fd, char *username, int len) 
{
	
		
	
	

	
	
		
	{
		
			
		{
			
			
			
			
		
	
	
		
	
	else
		


DelFromLinkTbl (int sock_fd) 
{
	
	
	
		return -1;
	
	

	
	
		
	{
		
			continue;
		
		
		
		
		
		
		
		
		
		
		
	
	
		
	
	else
		



#ifdef DEBUG
	
TestCtrlLinkTbl () 
{
	
		(" #################### LINK_CTRL TABLE ################################ \n");
	
		("     ipaddr    username sock_fd ctrl_tid notinformcnt buflen head tail \n");
	
		(" ------------- -------- ------- -------- ------------ ------ ---- ---- \n");
	

	
	
		
	{
		
				 
				 pcld->username, 
				 pcld->notinformcnt, 
				 pcld->tail);
	
	



#endif	/* 