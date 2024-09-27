/*
版权所有 (c) 华为技术有限公司 2012-2018


publisher分发文件
*/
#ifndef Open_source

#include "../XML.h"

#ifdef __cplusplus
extern "C" {
#endif

struct PublisherGroup* g_publisherGroup[MAX_SELF_COUNT] = {0};
int g_publisherCount = 0;

int GetPublisherId_ByName(char* name, SStateElement* tempAction)
{
    STestElement* tempTestElement = tempAction->testElement;
    int i = 0;

    //如果没有名字，返回第一个publisher
    if (name == 0)
    {   
        return 0;
    }

    for (i = 0; i < tempTestElement->publisherCount; i++)
    {
        if (strcmp(tempTestElement->publisherName[i], name) == 0)
        {      
            break;
        }
    }

    return i;
}

char* GetPublisherParaValue_ByName(int id, char* name, SStateElement* tempAction)
{
    STestElement* tempTestElement = tempAction->testElement;
    int i = 0;
    for (i = 0; i < MAX_PITS_PARA_NUMBER; i++)
    {
        if (tempTestElement->publisherParamName[id][i] == NULL)
        {      
            return NULL;
        }

        if (strcmp(tempTestElement->publisherParamName[id][i], name) == 0)
        {      
            return tempTestElement->publisherParamValue[id][i];
        }
    }

    return NULL;
}

static int GetPublisherNo_ByClassName(char* name)
{
    int i = 0;
    for (i = 0; i < g_publisherCount; i++)
    {
        if (strcmp(g_publisherGroup[i]->name, name) == 0)
        {      
            break;
        }
    }

    if (i == g_publisherCount)
        ; // 挂死

    return i;
}

static int GetPublisherNo_ByName(char* publisherName, SStateElement* tempAction)
{
    STestElement* tempTestElement = tempAction->testElement;

    int i = 0;
    int no = 0;
     
    if (publisherName == 0)
    {
        //如果没有指定名字，则选第一个publisher
        i = 0;
    }
    else
    {
        for (i = 0; i < tempTestElement->publisherCount; i++)
        {
            if (strcmp(tempTestElement->publisherName[i], publisherName) == 0)
            {
                break;
            }
        }
    }

    if (i == tempTestElement->publisherCount)
    {
        xml_assert(0, "Publisher find failed");
    }

    no = GetPublisherNo_ByClassName(tempTestElement->publisherClass[i]);

    return no;
}

int RegisterPublisherGroup(struct PublisherGroup* publisherGroup)
{
    g_publisherGroup[g_publisherCount] = publisherGroup;
    publisherGroup->no = g_publisherCount;
    g_publisherCount ++;
    return 1;
}

void DoPublisherOpen(char* name, SStateElement* tempAction)
{
    int no = GetPublisherNo_ByName(name, tempAction);
    if (g_publisherGroup[no] == NULL)
    {
        return;
    }
    g_publisherGroup[no]->open(name, tempAction);
}

void DoPublisherOutput(char* name, char* buf, int length, SStateElement* tempAction)
{
    int no = GetPublisherNo_ByName(name, tempAction);
    if (g_publisherGroup[no] ==NULL)
    {   
        return;
    }

    if (g_onOffDebugPublisher)
    {
        printf("	----output lenght is %d byte----\r\n", length);  
        HexDump( (u8*)buf, length);
    }

    g_publisherGroup[no]->output(name, buf, length, tempAction);
}

void DoPublisherClose(char* name, SStateElement* tempAction)
{
    int no = GetPublisherNo_ByName(name, tempAction);
    if (g_publisherGroup[no] == NULL)
    {
        return;
    }

    g_publisherGroup[no]->close(name, tempAction);
}

// publisher
extern void InitPublisherRaw(void);
extern void InitPublisherUdp(void);
extern void InitPublisherTcp(void);
extern void InitPublisherFile(void);
extern void InitPublisherDebug(void);
extern void InitPublisherPrintf(void);
extern void InitPublisherBuf(void);

void DoPublisherInit(void)
{
    InitPublisherRaw();
    InitPublisherUdp();
    InitPublisherTcp();
    InitPublisherFile();
    InitPublisherDebug();
    InitPublisherPrintf();
    InitPublisherBuf();
}

#ifdef __cplusplus
}
#endif

#endif // Open_source
