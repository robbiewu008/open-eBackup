/*
版权所有 (c) 华为技术有限公司 2012-2018


printf发包器，将最后变异的数据通过HexDump打印到屏幕上

<Publisher class="printf" name="p1">
</Publisher>
*/
#ifndef Open_source

#include "../../XML.h"

#ifdef __cplusplus
extern "C" {
#endif

static int PublisherPrintfOpen(char* name, SStateElement* tempAction)
{
    if (g_onOffDebugPublisher)
    {   
        printf("	----call PublisherPrintfOpen----\r\n");
    }

    return ENUM_YES;
}

static int PublisherPrintfOutput(char* name, char* buf, int length, SStateElement* tempAction)
{
    if (g_onOffDebugPublisher)
    {   
        printf("	----call PublisherPrintfOutput----\r\n");
    }

    if (g_onOffDebugPublisher)
    {   
        printf("	----publisher %s output %d byte----\r\n", name, length); 
    }

    HexDump((u8*)buf, length);
    return ENUM_YES;
}

static int PublisherPrintfClose(char* name, SStateElement* tempAction)
{
    if (g_onOffDebugPublisher)
    {   
        printf("	----call PublisherPrintfClose----\r\n");
    }

    return ENUM_YES;
}

static struct PublisherGroup g_publisherPrintf = {
    "printf",
    0xffffffff,
    PublisherPrintfOpen,
    PublisherPrintfOutput,
    PublisherPrintfClose
};

void InitPublisherPrintf(void)
{
    RegisterPublisherGroup(&g_publisherPrintf);
}

#ifdef __cplusplus
}
#endif

#endif // Open_source
