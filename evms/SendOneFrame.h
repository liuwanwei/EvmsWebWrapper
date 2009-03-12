#ifndef __SENDONEFRAME_H__
#define __SENDONEFRAME_H__


extern int SendEvmsFrame(int sock_fd, unsigned short retcode, char * data, int len, int sub_type);

#endif
