/*
版权所有 (c) 华为技术有限公司 2012-2018


tcp发包器，将最后变异的数据通过tcp端口发送出去

<Publisher class="tcp" >      
    <Param name="Host" value="1.1.1.1"/>      
    <Param name="Port" value="8080"/>      
    <Param name="Timeout" value="3"/>    
</Publisher>

*/
#ifndef Open_source

#include "../../XML.h"

#ifdef _WIN32
//#include <winsock.h> 
//#pragma comment (lib,"ws2_32.lib")
#endif

#ifdef _WIN32

#include<ws2tcpip.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#endif

#include <stdio.h>
#include <sys/types.h>

#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

static int isOpen[MAX_PITS_PARA_NUMBER] = {0};
static int clientFd[MAX_PITS_PARA_NUMBER];
static struct sockaddr_in serAddr[MAX_PITS_PARA_NUMBER];
char recvbuf[200001]; // 这长度以后可能要修改

static void TcpMsgSender(int fd, struct sockaddr* dst, char* buf, int length)
{
    //socklen_t len;
    //len = sizeof(*dst);

    send(fd, buf, length, 0);
    //recvfrom(fd, buf, BUFF_LEN, 0, (struct sockaddr*)&src, &len);  //接收来自server的信息
    //sleep(1);  //一秒发送一次消息


    
    int iDataNum = 0;
    recvbuf[0] = '\0';

    iDataNum = recv(fd, recvbuf, 2000, 0);

    recvbuf[iDataNum] = '\0';

    if (g_onOffDebugPublisher)
    {
        printf("------input is %d byte-------\n", iDataNum);
        printf("%s\n", recvbuf);
    }
}

static int PublisherTcpOpen(char* name, SStateElement* tempAction)
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

    char* host = GetPublisherParaValue_ByName(id, "Host", tempAction);
    char* port = GetPublisherParaValue_ByName(id, "Port", tempAction);

    if (g_onOffDebugPublisher)
    {   
        printf("	----call PublisherTcpOpen--host is %s, port is %s--\r\n", host, port); 
    }

    isOpen[id] = 1;

    clientFd[id] = socket(AF_INET, SOCK_STREAM, 0);
    if (clientFd[id] < 0)
    {
        printf("create socket fail!\n");
        return ENUM_NO;
    }

    Hw1Memset(&serAddr[id], 0, sizeof(serAddr[id]));
    serAddr[id].sin_family = AF_INET;
    serAddr[id].sin_addr.s_addr = inet_addr(host);
    serAddr[id].sin_port = htons(atol(port));  //注意网络序转换

    if (connect(clientFd[id], (struct sockaddr *)&serAddr[id], sizeof(serAddr[id])) < 0)
    {
        perror("connect");
        return ENUM_NO;
    }

    //设置超时时间
    int timeoutDefault = 3;//3s
    if (GetPublisherParaValue_ByName(id, "Timeout", tempAction))
    {
        timeoutDefault = (atol(GetPublisherParaValue_ByName(id, "Timeout", tempAction)));
    }

    struct timeval timeout = {timeoutDefault, 0};
    setsockopt(clientFd[id], SOL_SOCKET,SO_SNDTIMEO, (const char*)&timeout, sizeof(timeout));
    setsockopt(clientFd[id], SOL_SOCKET,SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));

    return ENUM_YES;
}

static int PublisherTcpOutput(char* name, char* buf, int length, SStateElement* tempAction)
{
    if(g_onOffDebugPublisher)
    {   
        printf("	----call PublisherTcpOutput----\r\n");
    }

    int id = GetPublisherId_ByName(name, tempAction);
    if(isOpen[id] ==0)
    {   
        xml_assert(0, "publisher not open");
    }

    if(g_onOffDebugPublisher)
    {   
        printf("	----publisher %s output %d byte----\r\n", name, length); 
    }

    TcpMsgSender(clientFd[id], (struct sockaddr*)&serAddr[id], buf, length);

    return ENUM_YES;
}

static int PublisherTcpClose(char* name, SStateElement* tempAction)
{
    int id = GetPublisherId_ByName(name, tempAction);
    if (isOpen[id] == 0)
    {   
        return ENUM_YES;
    }

    if (g_onOffDebugPublisher)
    {   
        printf("	----call PublisherTcpClose----\r\n"); 
    }

    isOpen[id] = 0;

    close(clientFd[id]);
    return ENUM_YES;
}

static struct PublisherGroup g_publisherTcp = {
    "tcp",
    0xffffffff,
    PublisherTcpOpen,
    PublisherTcpOutput,
    PublisherTcpClose
};

void InitPublisherTcp(void)
{
    RegisterPublisherGroup(&g_publisherTcp);
}

#ifdef __cplusplus
}
#endif

#endif // Open_source
