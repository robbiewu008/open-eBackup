/*
版权所有 (c) 华为技术有限公司 2012-2018


buf发包器，最后变异的数据通过DT_Pits_GetMutatorBuf(char** buf, int *len) 函数获取

<Publisher class="buf" name="p1">
</Publisher>

*/
#ifndef Open_source

#include "../../XML.h"

#ifdef __cplusplus
extern "C" {
#endif

static int PublisherBufOpen(char* name, SStateElement* tempAction)
{
    if (g_onOffDebugPublisher)
    {   
        printf("	----call PublisherBufOpen----\r\n");
    }

    return ENUM_YES;
}

static int PublisherBufOutput(char* name, char* buf, int length, SStateElement* tempAction)
{
    if (g_onOffDebugPublisher)
    {   
        printf("	----call PublisherBufOutput----\r\n");
    }

    if (g_onOffDebugPublisher)
    {   
        printf("	----publisher %s output %d byte----\r\n", name, length); 
    }

    g_publisherBuf = buf;
    g_publisherBufLen = length;
    return ENUM_YES;
}

static int PublisherBufClose(char* name, SStateElement* tempAction)
{
    if (g_onOffDebugPublisher)
    {   
        printf("	----call PublisherBufClose----\r\n");
    }

    return ENUM_YES;
}

static struct PublisherGroup g_publisherBuf1 = {
    "buf",
    0xffffffff,
    PublisherBufOpen,
    PublisherBufOutput,
    PublisherBufClose
};

void InitPublisherBuf(void)
{
    RegisterPublisherGroup(&g_publisherBuf1);
}

#ifdef __cplusplus
}
#endif

#endif // Open_source
