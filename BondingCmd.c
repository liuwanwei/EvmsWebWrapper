#include <stdio.h>
#include <stdlib.h>

#include "BondingCmd.h"
#include "Protocol.h"
#include "Debuger.h"
#include "Functions.h"

extern int CallShell(const char *, const char *, char *);

static const char * s_return_value_file = "/tmp/return_value_bonding";
const char * s_bonding_script = "/usr/sbin/sanager/Bonding.pl";
const char * s_bonding_get_all_eth_ports= "GetAllEthPorts";
const char * s_bonding_get_all_bondings = "GetAllBondings";
const char * s_bonding_add_bonding      = "AddBonding";
const char * s_bonding_mod_bonding      = "ModBonding";
const char * s_bonding_del_bonding      = "DelBonding";

int GetAllBondings(char ** reply, int * reply_len)
{
        if(0 != CallShell(s_bonding_script, s_bonding_get_all_bondings, NULL))
        {
                return -1;
        }

        return GetLinedupResultsFromFile(s_return_value_file,
                                         reply, 
                                         reply_len);
}

int AddBonding(char * msg_body,   int msg_body_len)
{
        return CallShell(s_bonding_script, s_bonding_add_bonding, NULL);;
}

int ModBonding(char * msg_body,   int msg_body_len)
{
        return CallShell(s_bonding_script, s_bonding_mod_bonding, NULL);;
}

int DelBonding(char * msg_body,   int msg_body_len)
{
        return CallShell(s_bonding_script, s_bonding_del_bonding, NULL);
}

int GetAllEthPorts(char ** reply, int * reply_len)
{
        if(0 != CallShell(s_bonding_script, s_bonding_get_all_eth_ports, NULL))
        {
                return -1;
        }

        return GetLinedupResultsFromFile(s_return_value_file,
                                         reply,
                                         reply_len);
}

int BondingCmd(int sock_fd, char * oneframe, int len)
{
        TRACE (10, ("### Enter BondingCmd ###"));

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
                case BD_GET_ALL_BONDINGS:
                        ret = GetAllBondings(&reply, &reply_len);
                        break;
                case BD_ADD_BONDING:
                        ret = AddBonding(msg_body, msg_body_len);
                        break;
                case BD_MOD_BONDING:
                        ret = ModBonding(msg_body, msg_body_len);
                        break;
                case BD_DEL_BONDING:
                        ret = DelBonding(msg_body, msg_body_len);
                        break;
                case BD_GET_ALL_ETH_PORTS:
                        ret = GetAllEthPorts(&reply, &reply_len);
                        break;
                default:
                        break;
        }

        TRACE (10, ("### Leave BondingCmd ###"));

        if (SendAppFrame(sock_fd, 
                        MSG_TYPE_BONDING,
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
