#ifndef __ISCSI_CMD_H__
#define __ISCSI_CMD_H__

#include "Protocol.h"

/*
 * This's some concepts about IET iscsi implementation:
 * iqn: iscsi qualified name.
 * tid: iscsi target id.
 * sid: iscsi session id. 
 * cid: iscsi connection id.
 *      There's one more sid/cid pair associated with a target 
 *      when a new initiator log on.
 *
 * The access control way is implemented by using /etc/initiator.allow 
 * and /etc/initiator.deny. The pragma used in these two file is like
 * the way used in /etc/hosts.allow, we can got them from "man hosts.allow".
 * But it's so hard to make a web-based GUI for it. So we could just use
 * expainative text and input boxes to make through.
 */

int IscsiCmd(int sock_fd, char * one_frame, int len);

#endif
