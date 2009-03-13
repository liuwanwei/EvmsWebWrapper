#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "IscsiCmd.h"
#include "Functions.h"
#include "Debuger.h"

// Iscsi shell script path
const char * s_iscsi_script = "/usr/sbin/sanager/Iscsi.pl";
const char * s_iscsi_add_target_func    = "AddTarget";
const char * s_iscsi_del_target_func    = "DelTarget";
const char * s_iscsi_get_all_targets    = "GetAllTargets";
const char * s_iscsi_target_access_ctrl = "TargetAccessCtrl";

static const char * s_return_value_file = "/tmp/return_value_iscsi";

// If we need the error message, every function need reply and reply_len param
int AddIscsiTarget(char * msg_body, int msg_body_len);
int DelIscsiTarget(char * msg_body, int msg_body_len);
int GetAllIscsiTargets(char ** reply, int * reply_len);
int IscsiTargetAccessCtrl(char * msg_body, int msg_body_len);

// int SendIscsiFrame(int, unsigned short , char * , int , int );

int
IscsiCmd (int sock_fd, char *oneframe, int len)
{
        TRACE (10, ("### Enter IscsiCmd ###"));

        int ret = -1;
        PPACKET_HDR hdr;
        char * msg_body;
        int msg_body_len;
        char * reply = NULL;
        int reply_len = 0;

        hdr = (PPACKET_HDR)oneframe;

        msg_body = oneframe + PACKET_HDR_LEN;
        msg_body_len = len - PACKET_HDR_LEN;

        switch(hdr->subtype)
        {
        case ISCSI_ADD_TARGET:
                ret = AddIscsiTarget(msg_body, msg_body_len);
                break;
        case ISCSI_DEL_TARGET:
                ret = DelIscsiTarget(msg_body, msg_body_len);
                break;
        case ISCSI_GET_ALL_TARGETS:
                ret = GetAllIscsiTargets(&reply, &reply_len);
                break;
        case ISCSI_TARGET_ACCESS_CTRL:
                ret = IscsiTargetAccessCtrl(msg_body, msg_body_len);
                break;
        default:
                break;
        }

        TRACE (10, ("### Leave IscsiCmd ###"));

        // printf("IscsiCmd.c::ret = %d\n", ret);

        if (SendAppFrame(sock_fd, 
                        MSG_TYPE_ISCSI,
                        ((PACKET_HDR *) oneframe)->subtype,
                        (unsigned short) ret, 
                        reply, 
                        reply_len) <= 0)
        {
                if(NULL != reply)
                {
                        free(reply);
                }
                
                return -1;
        }

        /* 切记不要忘了释放动态分配的内存*/
        if(NULL != reply)
        {
                free(reply);
        }

        return 0;
}

/*
int SendIscsiFrame(int sock_fd, unsigned short retcode, char * data, int data_len, int sub_type)
{
        printf("### Enter SendIscsiFrame###\n");

        char one_frame[MAX_FRAME_LEN];

        if (-1 == sock_fd)
        {
                printf("sock_fd == -1\n");
                return -1;
        }


        PACKET_HDR hdr;

        hdr.len = PACKET_HDR_LEN + data_len;
        hdr.type = MSG_TYPE_ISCSI;
        hdr.subtype = sub_type;
        hdr.retcode = retcode;

        memcpy (one_frame, &hdr, PACKET_HDR_LEN);
        memcpy (one_frame + PACKET_HDR_LEN, data, data_len);

        // now we change data_len's real meaning
        data_len = PACKET_HDR_LEN + data_len;

        if (data_len != my_send (sock_fd, one_frame, data_len))
        {
                perror ("send");
                return -1;
        }

        printf("### Leave SendIscsiFrame ###\n");

        return hdr.len;
}
*/


int AddIscsiTarget(char * msg_body, int msg_body_len)
{
        if(NULL == msg_body
        || msg_body_len <= 0)
        {
                return -1;
        }

        return CallShell(s_iscsi_script, s_iscsi_add_target_func, msg_body);
}
int DelIscsiTarget(char * msg_body, int msg_body_len)
{
        if(NULL == msg_body
        || msg_body_len <= 0)
        {
                return -1;
        }

        return CallShell(s_iscsi_script, s_iscsi_del_target_func, msg_body);
}

int GetAllIscsiTargets(char ** reply, int * reply_len)
{
        if(0 != CallShell(s_iscsi_script, s_iscsi_get_all_targets, NULL))
        {
                return -1;
        }

        return GetLinedupResultsFromFile(s_return_value_file,
                                         reply,
                                         reply_len);
}

int IscsiTargetAccessCtrl(char * msg_body, int msg_body_len)
{
        if(NULL == msg_body
        || msg_body_len <= 0)
        {
                return -1;
        }

        return CallShell(s_iscsi_script, s_iscsi_target_access_ctrl, msg_body);
}
