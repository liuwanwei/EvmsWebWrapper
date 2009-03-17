#ifndef __SYSTEM_CMD_H__
#define __SYSTEM_CMD_H__

int SendCommonFrame(int sock_fd, unsigned short retcode, char * data, int data_len, char main_type, char sub_type);

int SystemCmd(int sock_fd, char * oneframe, int len);

#endif
