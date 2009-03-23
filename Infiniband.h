#ifndef __INFINIBAND_H__
        #define __INFINIBAND_H__


        // �ⲿ�ӿڣ����ⲿģ�����

        /* ���IB���Ĺ��������Ƿ�����*/
        int CheckRunningEnvironment();

        int AddGroup(char *msg_body, int msg_body_len);
        int DelGroup(char *msg_body, int msg_body_len);
        int GetAllGroups(char **reply, int *reply_len);
        int AddDeviceToGroup(char *msg_body, int msg_body_len, char ** reply, int * reply_len);
        int DelDeviceFromGroup(char *msg_body, int msg_body_len);
        int GetDevicesFromGroup(char *msg_body, int msg_body_len, char **reply,
                int *reply_len);
        int GetAllIBPorts(char **reply, int *reply_len);
        int AddIBPortToGroup(char *msg_body, int msg_body_len);
        int DelIBPortFromGroup(char *msg_body, int msg_body_len);
        int GetIBPortsFromGroup(char *msg_body, int msg_body_len, char **reply,
                int *reply_len);


#endif
