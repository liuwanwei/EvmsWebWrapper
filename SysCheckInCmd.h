/* -- $Id: SysCheckInCmd.h,v 1.12 2004/04/30 03:20:25 qianjinqi Exp $ -- */

#ifndef _SYS_CHECKIN_CMD_H_
#define _SYS_CHECKIN_CMD_H_

#include "Protocol.h"

typedef struct __SYS_CHECK_IN_REQ_PKT
{
      	PACKET_HDR pkthdr;
  	char username[MAX_USRNAME_LEN];
	char password[MAX_PASSWD_LEN];
} SYS_CHECK_IN_REQ_PKT, *PSYS_CHECK_IN_REQ_PKT;

typedef struct __SYS_CHECK_IN_RSP_PKT 
{
	PACKET_HDR pkthdr;                
	unsigned int privilege;      
} SYS_CHECK_IN_RSP_PKT, *PSYS_CHECK_IN_RSP_PKT;

extern int SysCheckInCmd(int __sock_fd);

#endif /* SysCheckInCmd.h */
