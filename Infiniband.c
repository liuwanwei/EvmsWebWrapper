#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "Protocol.h"


// ��Դӳ��ű��ļ���
static const char *s_resource_mapping_script = "/usr/sbin/sanager/ResourceMapping.sh";

// 
static const char * s_return_value_file = "/tmp/return_value";

// ���������֮��ķָ���
static const char *s_param_delim = "|";

// 
static const char  s_record_splitter = '|';

static const char *s_device_param = "device=";
static const char *s_permission_param = "readonly=";
static const char *s_lun_param = "lun=";
static const char *s_group_param = "group=";
static const char *s_port_param = "port=";



/*
 * ����: ���һ���û�Ȩ����
 * ����: 
 * msg_body: Ȩ���������ַ���
 * msg_body_len: Ȩ���������ַ�������(��ʱû���õ�)
 * ����ֵ:
 * 0, ��ӳɹ�
 * ������ʧ��
 */
int AddGroup(char *msg_body, int msg_body_len)
{
        int pid;

        pid = fork();

        if (pid < 0)
        {
                return  - 1;
        }
        else if (pid == 0)
        {
                char *argument[4];
                char command[64];
                extern char **environ;

                snprintf(command, 64, "%s AddGroup %s",
                        s_resource_mapping_script, msg_body);
                argument[0] = "sh";
                argument[1] = "-c";
                argument[2] = command;
                argument[3] = NULL;
                execve("/bin/sh", argument, environ);
                exit(1);
        }
        else
        {
                int status;
                waitpid(pid, &status, 0);
                return WEXITSTATUS(status);
        }
}

/*
 * ����: ɾ��һ���û�Ȩ����
 * ����: 
 * msg_body: Ȩ���������ַ���
 * msg_body_len: Ȩ���������ַ�������(��ʱû���õ�)
 * ����ֵ:
 * 0, ɾ���ɹ�
 * ������ʧ��
 */
int DelGroup(char *msg_body, int msg_body_len)
{
        int pid;

        pid = fork();

        if (pid < 0)
        {
                return  - 1;
        }
        else if (pid == 0)
        {
                char *argument[4];
                char command[64];
                extern char **environ;

                snprintf(command, 64, "%s DelGroup %s",
                        s_resource_mapping_script, msg_body);
                argument[0] = "sh";
                argument[1] = "-c";
                argument[2] = command;
                argument[3] = NULL;
                execve("/bin/sh", argument, environ);
                exit(1);
        }
        else
        {
                int status;
                waitpid(pid, &status, 0);
                return WEXITSTATUS(status);
        }
}

/*
 * ����: ��ȡ����Ȩ����������б�
 * ����:
 * 		reply[out]: �����б��ַ���
 * 		reply_len[out]: �ַ�������
 * ����ֵ:
 * 		0�� �ɹ�����0��ʧ��
 */
int GetAllGroups(char **reply, int *reply_len)
{
	char names_buffer[NORMAL_BUFFER_LEN + 1];
	int names_buffer_len = 0;
	int group_num = 0;
	int pid;

        pid = fork();

        if (pid < 0)
        {
                return  0;
        }
        else if (pid == 0)
        {
                char *argument[4];
                char command[64];
                extern char **environ;

                snprintf(command, 64, "%s GetAllGroups",
                        s_resource_mapping_script);
                argument[0] = "sh";
                argument[1] = "-c";
                argument[2] = command;
                argument[3] = NULL;
                execve("/bin/sh", argument, environ);
                exit(1);
        }
        else
        {
		int ret;
                int status;
		char line_buffer[256];
		int name_len = 0;
		FILE * group_name_file_p = NULL;
		const char * group_name_file = "/tmp/groups_list";

                waitpid(pid, &status, 0);
                ret = WEXITSTATUS(status);

		if(0 != ret)
		{
			printf("script function %s::GetAllGroups() failed!\n", s_resource_mapping_script);
			return 0;
		}

		if(0 != access(group_name_file, F_OK))
		{
			return 0;
		}

        	group_name_file_p = fopen(group_name_file, "r+");
        	if (NULL == group_name_file_p)
        	{
                	return 0;
        	}

        	memset(names_buffer, 0, NORMAL_BUFFER_LEN + 1);

        	/* ÿһ��while ѭ������һ��IB �˿�����*/
        	while (NULL != fgets(line_buffer, 256, group_name_file_p))
        	{
                	name_len = strlen(line_buffer);
                	if (0 == name_len)
                	{
                        	break;
                	}
			
			// omit the new line char -- '0x0a' -- on the tail.
			memcpy(names_buffer + names_buffer_len, line_buffer, name_len - 1);

			names_buffer_len += name_len;

			group_num ++;
        	}

		fclose(group_name_file_p);

		if(group_num > 0)
		{
			(* reply) = (char *) malloc(names_buffer_len);
			memcpy((*reply), names_buffer, names_buffer_len);
			(* reply_len) = names_buffer_len;
		}

        	return group_num;
	}
}

int AddDeviceToGroup(char *msg_body, int msg_body_len, char ** reply, int * reply_len)
{
        /* Ҫ��ӵ���Դ���е��豸����*/
        char *device_name = NULL;

        /* ���豸��ӵ���Դ���lun���*/
        char *lun_index = NULL;

        /* ��Դ������ */
        char *group_name = NULL;

        if (NULL == msg_body || 0 >= msg_body_len)
        {
                return  - 1;
        }

        /* �����ݰ�����ȡ�豸���ơ�lun��š���Դ������*/
        char *str = NULL,  *token = NULL,  *save_ptr = NULL;

        for (str = msg_body;; str = NULL)
        {
                token = strtok_r(str, s_param_delim, &save_ptr);

                if (NULL == token)
                {
                        break;
                }

                if (!strncmp(token, s_device_param, strlen(s_device_param)))
                {
                        device_name = token + strlen(s_device_param);
                }
                else if (!strncmp(token, s_lun_param, strlen(s_lun_param)))
                {
                        lun_index = token + strlen(s_lun_param);
                }
                else if (!strncmp(token, s_group_param, strlen(s_group_param)))
                {
                        group_name = token + strlen(s_group_param);
                }
        }

        /* ���������û���������Ϊ�� */
        if (NULL == group_name || NULL == device_name || NULL == lun_index)
        {
                return  - 1;
        }

        int pid;

        pid = fork();

        if (pid < 0)
        {
                return  - 1;
        }
        else if (pid == 0)
        {
                char *argument[4];
                char command[64];
                extern char **environ;

                snprintf(command, 64, "%s AddDeviceToGroup %s %s %s",
                        s_resource_mapping_script, device_name, lun_index,
                        group_name);

                argument[0] = "sh";
                argument[1] = "-c";
                argument[2] = command;
                argument[3] = NULL;
                execve("/bin/sh", argument, environ);
                exit(1);
        }
        else
        {
		int ret;
                int status;
		FILE * return_value_file_p = NULL;
		char line_buffer[256];

                waitpid(pid, &status, 0);
                ret = WEXITSTATUS(status);

		if(0 != ret)
		{
			printf("'%s::AddDeviceToGroup %s %s %s' failed\n", s_resource_mapping_script,
									   device_name,
									   lun_index,
									   group_name);
			return ret;
		}

		if (0 != access(s_return_value_file, F_OK))
        	{
                	return  - 1;
        	}

        	return_value_file_p = fopen(s_return_value_file, "r+");
        	if (NULL == return_value_file_p)
        	{
                	return  - 1;
        	}

		
		if(NULL == fgets(line_buffer, 256, return_value_file_p))
		{
			fclose(return_value_file_p);
			printf("%s::AddDeviceToGroup failed: no VDISK return!\n", s_resource_mapping_script);
			return -1;
		}

		fclose(return_value_file_p);

		ret = strlen(line_buffer);
		(* reply) = (char *)malloc(ret);
		memcpy((*reply), line_buffer, ret);
		(* reply_len) = ret;

		return 0;
        }
}

int DelDeviceFromGroup(char *msg_body, int msg_body_len)
{
        /* ��Դ���е��豸����*/
        char *device_name = NULL;

        /* ��Դ������ */
        char *group_name = NULL;

        if (NULL == msg_body || 0 >= msg_body_len)
        {
                return  - 1;
        }

        /* �����ݰ�����ȡ�豸���ơ���Դ������*/
        char *str = NULL,  *token = NULL,  *save_ptr = NULL;

        for (str = msg_body;; str = NULL)
        {
                token = strtok_r(str, s_param_delim, &save_ptr);

                if (NULL == token)
                {
                        break;
                }

                if (!strncmp(token, s_device_param, strlen(s_device_param)))
                {
                        device_name = token + strlen(s_device_param);
                }
                else if (!strncmp(token, s_group_param, strlen(s_group_param)))
                {
                        group_name = token + strlen(s_group_param);
                }
        }

        /* ���������û���������Ϊ�� */
        if (NULL == group_name || NULL == device_name)
        {
                return  - 1;
        }

        /* �����ӽ���ִ�нű�*/
        int pid;

        pid = fork();

        if (pid < 0)
        {
                return  - 1;
        }
        else if (pid == 0)
        {
                char *argument[4];
                char command[64];
                extern char **environ;

                snprintf(command, 64, "%s DelDeviceFromGroup %s %s",
                        s_resource_mapping_script, device_name, group_name);

                argument[0] = "sh";
                argument[1] = "-c";
                argument[2] = command;
                argument[3] = NULL;
                execve("/bin/sh", argument, environ);
                exit(1);
        }
        else
        {
                int status;
                waitpid(pid, &status, 0);
                return WEXITSTATUS(status);
        }
}

int GetDevicesFromGroup(char *msg_body, int msg_body_len, char **reply, int
        *reply_len)
{
        // Ȩ�����豸�б��ļ������ļ���ָ��
        char devices_file[SHORT_BUFFER_LEN + 1];
        FILE *devices_file_p;

        // �����������豸��Ϣ���ŵ������������
        char devices_buffer[NORMAL_BUFFER_LEN + 1];
        int devices_buffer_len = 0;
        int device_num = 0;

        // ���ж�ȡ�豸��Ϣʱ���л�����
        char line_buffer[256];
	char * str = NULL;

        // �ָ�һ����Ϣ�ñ���
        char *token = NULL;
        int field_index = 0;
        char *save_ptr = NULL;
        char param[SHORT_BUFFER_LEN + 1];
        int param_len = 0;


        if (NULL == msg_body || 0 >= msg_body_len)
        {
                return  - 1;
        }

        snprintf(devices_file, SHORT_BUFFER_LEN, 
                "/proc/scsi_tgt/groups/%s/devices", msg_body);

        if (0 != access(devices_file, F_OK))
        {
                return  - 1;
        }

        devices_file_p = fopen(devices_file, "r+");
        if (NULL == devices_file_p)
        {
                return  - 1;
        }

        memset(devices_buffer, 0, NORMAL_BUFFER_LEN + 1);

        /* ÿһ��while ѭ������һ���豸��Ϣ*/
        while (NULL != fgets(line_buffer, 256, devices_file_p))
        {
                field_index = 0;

                /* 
                 * �ٶ�ÿ���кü����ֶΣ�
                 * �ֶμ��ÿո��tab ��Ϊ�ָ�����
                 * ÿ��For ѭ������һ����Ϣ
                 */
                for (str = line_buffer ; ; str = NULL)
                {
                        token = strtok_r(str, " \t", &save_ptr);

                        if (NULL == token)
                        {
                                break;
                        }

                        memset(param, 0, SHORT_BUFFER_LEN + 1);
                        switch (field_index)
                        {
                                case 0:
                                        snprintf(param, SHORT_BUFFER_LEN, 
                                                "%s%s", s_device_param, token);
                                        break;
                                case 1:
                                        snprintf(param, SHORT_BUFFER_LEN, 
                                                "%s%s", s_lun_param, token);
                                        break;
                                case 2:
                                        snprintf(param, SHORT_BUFFER_LEN, 
                                                "%s%s", s_permission_param,
                                                token);
                                        break;
                                default:
                                        break;
                        }

                        param_len = strlen(param) + 1;
                        if (1 == param_len)
                        {
                                /* ������"default" ? */
                                break;
                        }

                        /* �������������ȵ��豸��������ȡ*/
                        if (devices_buffer_len + param_len > NORMAL_BUFFER_LEN)
                        {
                                printf(
                                        "devices_buffer_len beyond max len: %d\n", NORMAL_BUFFER_LEN);
                                break;
                        }
			else
			{
				// add '|' to each new record except the first record
				if((device_num != 0) && (field_index == 0))
				{
					devices_buffer[devices_buffer_len ++] = s_record_splitter;
				}

				memcpy(devices_buffer + devices_buffer_len, param, param_len);

                        	field_index++;

                        	devices_buffer_len += param_len;
			}

                }

                device_num++;
        }

        fclose(devices_file_p);

	if(device_num > 0)
	{
		(* reply) = (char *)malloc(devices_buffer_len);
		memcpy((*reply), devices_buffer, devices_buffer_len);
		(* reply_len) = devices_buffer_len;
	}


        return device_num;
}


/* Get all ib port names linked on current Storage Server */
int GetAllIBPorts(char **reply, int *reply_len)
{
        int pid;

        pid = fork();

        if (pid < 0)
        {
                return  - 1;
        }
        else if (pid == 0)
        {
                char *argument[4];
                char command[64];
                extern char **environ;

                snprintf(command, 64, "%s GetAllIBPorts",
                        s_resource_mapping_script);
                argument[0] = "sh";
                argument[1] = "-c";
                argument[2] = command;
                argument[3] = NULL;
                execve("/bin/sh", argument, environ);
                exit(1);
        }
        else
        {
		int ret;
                int status;
		const char * ib_ports_file = "/tmp/all_linked_ports";
        	FILE *ib_ports_file_p;

        	// �����������豸��Ϣ���ŵ������������
        	char ib_ports_buffer[NORMAL_BUFFER_LEN + 1];
        	int ib_ports_buffer_len = 0;
        	int port_num = 0;

        	// ���ж�ȡ�豸��Ϣʱ���л�����
        	char line_buffer[256];
		int port_name_len = 0;

                waitpid(pid, &status, 0);

                ret = WEXITSTATUS(status);

		if(0 != ret )
		{
			printf("script function %s::GetAllGroups() failed!\n", s_resource_mapping_script);
			return 0;
		}

        	if (0 != access(ib_ports_file, F_OK))
        	{
                	return  - 1;
        	}

        	ib_ports_file_p = fopen(ib_ports_file, "r+");
        	if (NULL == ib_ports_file_p)
        	{
                	return  - 1;
        	}

        	memset(ib_ports_buffer, 0, NORMAL_BUFFER_LEN + 1);

        	while (NULL != fgets(line_buffer, 256, ib_ports_file_p))
		{
			port_name_len = strlen(line_buffer) + 1;
			if(1 == port_name_len)
			{
				break;
			}

			if(ib_ports_buffer_len + port_name_len < NORMAL_BUFFER_LEN + 1)
			{
				memcpy(ib_ports_buffer + ib_ports_buffer_len, line_buffer, port_name_len);
				ib_ports_buffer_len += port_name_len;
				port_num ++;
			}
			else
			{
				break;
			}
		}

		fclose(ib_ports_file_p);

		if(port_num >0)
		{
			(* reply) = (char *)malloc(ib_ports_buffer_len);
			memcpy((* reply), ib_ports_buffer, ib_ports_buffer_len);
			(* reply_len) = ib_ports_buffer_len;
		}

		return port_num;
        }
}

int AddIBPortToGroup(char *msg_body, int msg_body_len)
{
        /* Ҫ��ӵ���Դ���е�IB �˿�����*/
        char *port_name = NULL;

        /* ��Դ������ */
        char *group_name = NULL;

        if (NULL == msg_body || 0 >= msg_body_len)
        {
                return  - 1;
        }

        /* �����ݰ�����ȡ�˿����ơ���Դ������*/
        char *str = NULL,  *token = NULL,  *save_ptr = NULL;

        for (str = msg_body;; str = NULL)
        {
                token = strtok_r(str, s_param_delim, &save_ptr);

                if (NULL == token)
                {
                        break;
                }

                if (!strncmp(token, s_port_param, strlen(s_port_param)))
                {
                        port_name = token + strlen(s_port_param);
                }
                else if (!strncmp(token, s_group_param, strlen(s_group_param)))
                {
                        group_name = token + strlen(s_group_param);
                }
        }

        /*�û���������Ϊ�� */
        if (NULL == group_name || NULL == port_name)
        {
                return  - 1;
        }

        /* ִ�нű� */
        int pid;

        pid = fork();

        if (pid < 0)
        {
                return  - 1;
        }
        else if (pid == 0)
        {
                char *argument[4];
                char command[128];
                extern char **environ;

                snprintf(command, 128, "%s AddIBPortToGroup %s %s",
                        s_resource_mapping_script, port_name, group_name);

		// printf("wwliu: %s\n", command);

                argument[0] = "sh";
                argument[1] = "-c";
                argument[2] = command;
                argument[3] = NULL;
                execve("/bin/sh", argument, environ);
                exit(1);
        }
        else
        {
                int status;
                waitpid(pid, &status, 0);
                return WEXITSTATUS(status);
        }
}

int DelIBPortFromGroup(char *msg_body, int msg_body_len)
{
        /* Ҫ����Դ����ɾ����IB �˿�����*/
        char *port_name = NULL;

        /* ��Դ������ */
        char *group_name = NULL;

        if (NULL == msg_body || 0 >= msg_body_len)
        {
                return  - 1;
        }

        /* �����ݰ�����ȡ�˿����ơ���Դ������*/
        char *str = NULL,  *token = NULL,  *save_ptr = NULL;

        for (str = msg_body;; str = NULL)
        {
                token = strtok_r(str, s_param_delim, &save_ptr);

                if (NULL == token)
                {
                        break;
                }

                if (!strncmp(token, s_port_param, strlen(s_port_param)))
                {
                        port_name = token + strlen(s_port_param);
                }
                else if (!strncmp(token, s_group_param, strlen(s_group_param)))
                {
                        group_name = token + strlen(s_group_param);
                }
        }

        /*�û���������Ϊ�� */
        if (NULL == group_name || NULL == port_name)
        {
                return  - 1;
        }

        /* ִ�нű� */
        int pid;

        pid = fork();

        if (pid < 0)
        {
                return  - 1;
        }
        else if (pid == 0)
        {
                char *argument[4];
                char command[64];
                extern char **environ;

                snprintf(command, 64, "%s DelIBPortFromGroup %s %s",
                        s_resource_mapping_script, port_name, group_name);

                argument[0] = "sh";
                argument[1] = "-c";
                argument[2] = command;
                argument[3] = NULL;
                execve("/bin/sh", argument, environ);
                exit(1);
        }
        else
        {
                int status;
                waitpid(pid, &status, 0);
                return WEXITSTATUS(status);
        }
}

int GetIBPortsFromGroup(char *msg_body, int msg_body_len, char **reply, int
        *reply_len)
{
        // Ȩ����IB �˿��б��ļ������ļ���ָ��
        char devices_file[SHORT_BUFFER_LEN + 1];
        FILE *devices_file_p;

        // ����������IB �˿���Ϣ���ŵ������������
        char devices_buffer[NORMAL_BUFFER_LEN + 1];
        int devices_buffer_len = 0;

        int port_num = 0;

        // ���ж�ȡ�豸��Ϣʱ���л�����
        char line_buffer[256];

        int name_len = 0;

        if (NULL == msg_body || 0 >= msg_body_len)
        {
                return  - 1;
        }

        snprintf(devices_file, SHORT_BUFFER_LEN, 
                "/proc/scsi_tgt/groups/%s/names", msg_body);

        if (0 != access(devices_file, F_OK))
        {
                return  - 1;
        }

        devices_file_p = fopen(devices_file, "r+");
        if (NULL == devices_file_p)
        {
                return  - 1;
        }

        memset(devices_buffer, 0, NORMAL_BUFFER_LEN + 1);

        /* ÿһ��while ѭ������һ��IB �˿�����*/
        while (NULL != fgets(line_buffer, 256, devices_file_p))
        {

                name_len = strlen(line_buffer) + 1;
                if (1 == name_len)
                {
                        break;
                }

                /* �������������ȵ��豸��������ȡ*/
                if (devices_buffer_len + name_len > NORMAL_BUFFER_LEN)
                {
                        printf("devices_buffer_len beyond max len: %d\n",
                                NORMAL_BUFFER_LEN);
                        break;
                }

                memcpy(devices_buffer + devices_buffer_len, line_buffer, name_len);

                devices_buffer_len += name_len;

                port_num++;
        }

        fclose(devices_file_p);

	if(port_num > 0)
	{
		(* reply) = (char *)malloc(devices_buffer_len);
		memcpy((*reply), devices_buffer, devices_buffer_len);
		(* reply_len) = devices_buffer_len;
	}

        return port_num;
}

/*
int main()
{
        return CheckRunningEnvironment();
}
*/
