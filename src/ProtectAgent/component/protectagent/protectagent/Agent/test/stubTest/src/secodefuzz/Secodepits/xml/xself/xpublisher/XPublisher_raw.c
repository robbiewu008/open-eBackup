/*
��Ȩ���� (c) ��Ϊ�������޹�˾ 2012-2018


raw���������������������ͨ������ֱ�ӷ��ͳ�ȥ

<Publisher class="RawEther" name="p1">
	<Param name="Interface" value="eth0"/>
</Publisher>

*/
#ifndef Open_source

#include "../../XML.h"

#ifdef _WIN32
// #include <winsock.h> 
// #pragma comment (lib,"ws2_32.lib")
#endif

#ifdef _WIN32
#include<ws2tcpip.h>
#else
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <netinet/if_ether.h>
#include <net/if_arp.h>
#include <netpacket/packet.h>
#include <net/if.h>
#include <net/ethernet.h>
#include <arpa/inet.h>
#endif

#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _WIN32
static int isOpen[MAX_PITS_PARA_NUMBER] = {0};
static int sockRawFd[MAX_PITS_PARA_NUMBER] = {0};
static struct sockaddr_ll sll[MAX_PITS_PARA_NUMBER]; // ԭʼ�׽��ֵ�ַ�ṹ	
static struct ifreq req[MAX_PITS_PARA_NUMBER]; // ����ӿڵ�ַ

static void RawMsgSender(int fd, struct sockaddr* dst, char* buf, int length)
{
    sendto(fd, buf, length, 0, dst, sizeof(struct sockaddr_ll));
}

static int PublisherRawOpen(char* name, SStateElement* tempAction)
{
    int id = GetPublisherId_ByName(name, tempAction);
    if (id >= 10)
    {   
        xml_assert(0, "publisher not found by name");
    }

    if (isOpen[id] == 1)
    {   
        return ENUM_YES;
    }

    char * interfaceName = GetPublisherParaValue_ByName(id, "Interface", tempAction);
    if (interfaceName == NULL)
    {
        printf("Interface is lost\r\n");
        exit(-1);
    }

    if(g_onOffDebugPublisher)
    {   
        printf("	----call PublisherRawOpen, Interface is %s ----\r\n", interfaceName); 
    }

    isOpen[id] = 1;

    // 1.����ͨ���õ�ԭʼ�׽���
    sockRawFd[id] = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));

    strncpy(req[id].ifr_name, interfaceName, IFNAMSIZ);			// ָ����������
    if(-1 == ioctl(sockRawFd[id], SIOCGIFINDEX, &req[id]))	 // ��ȡ����ӿ�
    {
        printf("ioctl open %s error\r\n", interfaceName);
        close(sockRawFd[id]);
        exit(-1);
    }

    /* ������ӿڸ�ֵ��ԭʼ�׽��ֵ�ַ�ṹ*/
    bzero(&sll[id], sizeof(sll[id]));
    sll[id].sll_ifindex = req[id].ifr_ifindex;

    return ENUM_YES;
}

static int PublisherRawOutput(char* name, char* buf, int length, SStateElement* tempAction)
{
    if (g_onOffDebugPublisher)
    {   
        printf("	----call PublisherRawOutput----\r\n");
    }

    int id = GetPublisherId_ByName(name, tempAction);
    if (isOpen[id] ==0)
    {   
        xml_assert(0 , "publisher not open");
    }

    if (g_onOffDebugPublisher)
    {   
        printf("	----publisher %s output %d byte----\r\n", name, length); 
    }

    RawMsgSender(sockRawFd[id], (struct sockaddr*)&sll[id], buf, length);
    return ENUM_YES;
}

static int PublisherRawClose(char* name, SStateElement* tempAction)
{
    // ���ò��رվ�ֱ���˳�
    if (tempAction->isClose && (strcmp(tempAction->isClose, "no") == 0 ))
    {
        if (g_onOffDebugPublisher)
        {
            printf("	----call PublisherRawClose--noclose--\r\n"); 
        }

        return ENUM_YES;
    }

    int id = GetPublisherId_ByName(name, tempAction);
    if (isOpen[id] == 0)
    {   
        return ENUM_YES;
    }

    if (g_onOffDebugPublisher)
    {   
        printf("	----call PublisherRawClose----\r\n"); 
    }

    isOpen[id] = 0;

    close(sockRawFd[id]);
    return ENUM_YES;
}

static  struct PublisherGroup g_publisherRaw = {
    "raw",
    0xffffffff,
    PublisherRawOpen,
    PublisherRawOutput,
    PublisherRawClose
};
#endif

void InitPublisherRaw(void)
{
#ifndef _WIN32
    RegisterPublisherGroup(&g_publisherRaw);
#endif
}

#ifdef __cplusplus
}
#endif

#endif // Open_source
