#ifndef __INFINIBAND_CMD_H__
#define __INFINIBAND_CMD_H__

#include "Protocol.h"

int InfinibandCmd(int sock_fd, char * oneframe, int len);

int GetAllGroups(char **reply, int *reply_len);

int AddGroup(char *msg_body, int msg_body_len);

int DelGroup(char *msg_body, int msg_body_len);

int AddDeviceToGroup(char *msg_body, int msg_body_len, char ** reply, int * reply_len);

int DelDeviceFromGroup(char *msg_body, int msg_body_len);

int GetDevicesFromGroup(char *msg_body, int msg_body_len, char **reply, int *reply_len);

int GetAllIBPorts(char **reply, int *reply_len);

int AddIBPortToGroup(char *msg_body, int msg_body_len);

int DelIBPortFromGroup(char *msg_body, int msg_body_len);

int GetIBPortsFromGroup(char *msg_body, int msg_body_len, char **reply, int *reply_len);


#endif
