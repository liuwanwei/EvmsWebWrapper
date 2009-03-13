/* -- $Id: Protocol.h,v 1.12.2.4.4.5 2004/08/13 00:56:28 yuzhe Exp $ -- */
/********
 * history:
 *     add event "HANDSHAKE_REQ & HANDSHAKE_RSP" 
 * */

#ifndef _PROTOCOL_H
#define _PROTOCOL_H    1

#include <sys/types.h>
#include "NetServer.h"

// 报文类型：
typedef enum
{
        MSG_TYPE_UNKNOWN   = 0x00,      // unknown msg type
        MSG_TYPE_HANDSHAKE = 0x01,      // 握手消息：需要客户端每30秒发送一个“握手”信号，
                                        // 表示客户端处于正常状态。如果连续5次没有收到该信号，
                                        // 服务器认为客户端出现异常，自动关闭该客户端所有连接。
        MSG_TYPE_EVMS      = 0x02,      // EVMS related msg type
        MSG_TYPE_IB        = 0x03,      // Infiniband ralated msg type
        MSG_TYPE_ISCSI     = 0x04,      // iSCSI related msg type
        MSG_TYPE_SYSTEM    = 0x05,      // system specific commands, such as remote "ls"
        MSG_TYPE_BONDING   = 0x06,
        MSG_TYPE_NUM
}MAIN_MSG_TYPE;
        

// 进入系统报文（类型：0x01）
#define SYS_CHECKIN_REQ    0x02         // 进入系统

// HandShake Event (type: 0x02)
#define HANDSHAKE_REQ         0x00      // 2008-6-21
#define HANDSHAKE_RSP         0x01      // 2008-6-21

// These sub-message-type was set by 
// local module, read by web module
#define EVMS_PROMPT_MSG       0x21      // Prompt messages need to show to user
#define EVMS_NEED_USER_INPUT  0x22      // 
#define EVMS_OBJECT_INFO      0x23      // Reply evms query object command.

// These sub-message-type was set by 
// web module, read by local module.
#define EVMS_INIT_MSG         0x25      // Init evms, used only once by web module
#define EVMS_COMMAND_MSG      0x26      // Set this subtype each time web module start a new evms request
#define EVMS_DEINIT_MSG       0x27      // Deinit evms, when you need evms no more
#define MAX_EVMS_COMMAND_LINE_PARAM     10

typedef enum
{
        IB_ADD_GROUP = 0x21,            // 添加一个权限组
        IB_DEL_GROUP,                   // 删除一个权限组
        IB_GET_ALL_GROUPS,              // 获取所有的权限组名称
        IB_ADD_DEVICE_TO_GROUP,         // 添加一个存储资源到权限组
        IB_DEL_DEVICE_FROM_GROUP,       // 从权限组删除一个存储资源
        IB_GET_DEVICES_FROM_GROUP,      // 获取权限组中所有存储资源名称
        IB_GET_ALL_IB_PORTS,            // 枚举子网内的所有IB卡端口名称
        IB_ADD_IB_PORT_TO_GROUP,        // 将一个IB卡端口添加到权限组中
        IB_DEL_IB_PORT_FROM_GROUP,      // 将一个IB卡端口从权限组中删除
        IB_GET_IB_PORTS_FROM_GROUP      // 获取权限组中所有IB端口名称

}IB_RELATED_OPERATIONS;

typedef enum
{
        ISCSI_ADD_TARGET        = 0x21, // Make a volume into iscsi target
        ISCSI_DEL_TARGET        = 0x22, // Delete one iscsi target
        ISCSI_GET_ALL_TARGETS   = 0x23, //
        ISCSI_TARGET_ACCESS_CTRL= 0x24  // Set iscsi target access ctrl modes
}ISCSI_RELATED_OPERATIONS;

typedef enum
{
        BD_GET_ALL_BONDINGS     = 0x21,
        BD_ADD_BONDING          = 0x22,
        BD_MOD_BONDING          = 0x23,
        BD_DEL_BONDING          = 0x24,
        BD_GET_ALL_ETH_PORTS    = 0x25
}BONDING_RELATED_OPERATIONS;

typedef enum
{
        SYSTEM_LS_DIR = 0x01,           // list subdirs and files, almost the same as "ls -l" command
        SYSTEM_CR_DIR = 0x02            // create a directory, the dir must has the format of absolute path.
}SYSTEM_RELATED_OPERATIONS;

#define DEFAULT_RETCODE         0
#define RETCODE_OK              0
#define RETCODE_FAIL            1

/* 各种应用相关的错误类型定义*/
typedef enum
{
        EVMS_ERROR_FAILED = 1,
        EVMS_ERROR_MAX
}EVMS_ERROR_TYPES;

typedef enum 
{
        IB_ERROR_FAILED = EVMS_ERROR_MAX,
        IB_ERROR_DEVICE_NOTEXIST,       // 存储资源不存在
        IB_ERROR_PORT_NOTEXIST,         // ib端口不存在
        IB_ERROR_GROUP_NOT_EXIST,       // 权限组不存在
        IB_ERROR_MAX
}IB_ERROR_TYPES;

/***********************
*  应用报文相关结构体  *
***********************/
typedef struct __PACKET_HDR {       // 协议包包头信息；
  unsigned short len;               // 数据包长度； , including the header
  unsigned char type;               // 命令字：类型；
  unsigned char subtype;            // 命令字：子类型；
  unsigned short order;             // 操作序列号；
  unsigned short retcode;           // 操作返回码；
} PACKET_HDR, *PPACKET_HDR;         // 报文头结构；


#define PACKET_HDR_LEN              sizeof(PACKET_HDR)

#define NET_BUFFER_LEN              (64 * 1024 - 1)
#define NORMAL_BUFFER_LEN           (2048 - 1)
#define SHORT_BUFFER_LEN            (256 - 1)
#define MAX_RESPONSE_LEN            (NET_BUFFER_LEN - )

// 8 bytes header
#define MAX_FRAME_LEN               NET_BUFFER_LEN
#define MAX_CONTENT_LEN             (MAX_FRAME_LEN - sizeof(PACKET_HDR))

// socket send wrap function
ssize_t my_send(int s, const void * buf, size_t len);

#endif /* Protocol.h */
