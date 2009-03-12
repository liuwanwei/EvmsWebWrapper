#ifndef __INFINIBAND_CMD_H__
#define __INFINIBAND_CMD_H__

#include "Protocol.h"

int SendIBFrame(int sock_fd, unsigned short retcode, char * data, int len, int sub_type);

int InfinibandCmd(int sock_fd, char * oneframe, int len);

#endif
