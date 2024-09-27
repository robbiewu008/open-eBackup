/*
��Ȩ���� (c) ��Ϊ�������޹�˾ 2012-2018


udp���������������������ͨ��udp�˿ڷ��ͳ�ȥ

<Publisher class="udp" name="UDP_L2TP">      
    <Param name="Host" value="1.1.1.1"/>      
    <Param name="Port" value="1007"/>      
    <!-- <Param name="SrcPort" value="1007"/> -->      
</Publisher>
*/
#ifndef Open_source

#include "../../XML.h"

#ifdef _WIN32
//#include <winsock.h> 
//#pragma comment (lib,"ws2_32.lib")
#endif

#ifdef _WIN32
#include <Windows.h>
#include<ws2tcpip.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#endif

#include <stdio.h>
#include <sys/types.h>

#include <string.h>

//#include <iostream>
//#include <fstream>
//#include <string>

#ifdef __cplusplus
extern "C" {
#endif

static int isOpen[MAX_PITS_PARA_NUMBER] = {0};
static int clientFd[MAX_PITS_PARA_NUMBER];
static struct sockaddr_in serAddr[MAX_PITS_PARA_NUMBER];

static void UdpMsgSender(int fd, struct sockaddr* dst, char* buf, int length)
{
    socklen_t len;

    len = sizeof(*dst);
    int i = sendto(fd, buf, length, 0, dst, len);

    if (g_onOffDebugPublisher)
    {   
        printf("	success seng %d length packets by udp publisher\r\n", i); 
    }
}

//���������������ã��ڵ�һ��ʹ������publisherʱ��Ҫ��������
static int PublisherUdpOpen(char* name,SStateElement* tempAction)
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

    if (g_onOffDebugPublisher)
    {   
        printf("	----call PublisherUdpOpen----\r\n"); 
    }

    isOpen[id] = 1;

    clientFd[id] = socket(AF_INET, SOCK_DGRAM, 0);
    if (clientFd[id] < 0)
    {
        printf("create socket fail!\n");
        return ENUM_NO;
    }

    //�����Դ�˿ڣ����֮
    if (GetPublisherParaValue_ByName(id, "SrcPort", tempAction))
    {
        Hw1Memset(&serAddr[id], 0, sizeof(serAddr[id]));
        serAddr[id].sin_family = AF_INET;
        serAddr[id].sin_addr.s_addr = htonl(INADDR_ANY);
        serAddr[id].sin_port = htons(atol(GetPublisherParaValue_ByName(id, "SrcPort", tempAction)));

        if(bind(clientFd[id], (struct sockaddr*)&serAddr, sizeof(struct sockaddr_in)) < 0) 
        {
            printf("create socket fail!\n");
            return ENUM_NO;	
        }
    }

    //��Ŀ�Ķ˿�
    Hw1Memset(&serAddr[id], 0, sizeof(serAddr[id]));
    serAddr[id].sin_family = AF_INET;
    serAddr[id].sin_addr.s_addr = inet_addr(GetPublisherParaValue_ByName(id, "Host", tempAction));
    serAddr[id].sin_port = htons(atol(GetPublisherParaValue_ByName(id, "Port", tempAction)));  //ע��������ת��

    if (g_onOffDebugPublisher)
    {
        printf("	----host is %s  dst port is %d----\r\n", GetPublisherParaValue_ByName(id, "Host", tempAction), serAddr[id].sin_port); 
    }

    return ENUM_YES;
}

static int PublisherUdpOutput(char* name, char* buf, int length, SStateElement* tempAction)
{
    if (g_onOffDebugPublisher)
    {   
        printf("	----call PublisherUdpOutput----\r\n");
    }

    int id = GetPublisherId_ByName(name, tempAction);
    if (isOpen[id] == 0)
    {   
        xml_assert(0, "publisher not open");
    }

    if (g_onOffDebugPublisher)
    {   
        printf("	----publisher %s output %d byte----\r\n", name, length); 
    }

    UdpMsgSender(clientFd[id], (struct sockaddr*)&serAddr[id], buf, length);

    return ENUM_YES;
}

//���������������ã��ڵ����˳�ʱ��Ҫ��������
static int PublisherUdpClose(char* name, SStateElement* tempAction)
{
    int id = GetPublisherId_ByName(name, tempAction);
    if (isOpen[id] == 0)
    {   
        return ENUM_YES;
    }

    if (g_onOffDebugPublisher)
    {   
        printf("	----call PublisherUdpClose----\r\n"); 
    }

    isOpen[id] = 0;

    close(clientFd[id]);
    return ENUM_YES;
}

static struct PublisherGroup g_publisherUdp = {
    "udp",
    0xffffffff,
    PublisherUdpOpen,
    PublisherUdpOutput,
    PublisherUdpClose
};

void InitPublisherUdp(void)
{
    RegisterPublisherGroup(&g_publisherUdp);
}

#ifdef __cplusplus
}
#endif

#endif // Open_source