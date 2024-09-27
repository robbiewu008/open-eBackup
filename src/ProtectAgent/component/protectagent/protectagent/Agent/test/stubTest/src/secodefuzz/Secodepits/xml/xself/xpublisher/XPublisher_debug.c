/*
��Ȩ���� (c) ��Ϊ�������޹�˾ 2012-2018


debug���������������������ͨ��HexDump��ӡ����Ļ��
����ӡDISPLAY_MAX_BIN_LENGTH =2600���ֽ�

<Publisher class="debug" name="p1">
</Publisher>

*/
#ifndef Open_source

#include "../../XML.h"

#ifdef __cplusplus
extern "C" {
#endif

static int PublisherDebugOpen(char* name,SStateElement* tempAction)
{
    if (g_onOffDebugPublisher)
    {   
        printf("	----call PublisherDebugOpen----\r\n"); 
    }

    return ENUM_YES;
}

static int PublisherDebugOutput(char* name, char* buf, int length, SStateElement* tempAction)
{
    if (g_onOffDebugPublisher)
    {   
        printf("	----call PublisherDebugOutput----\r\n"); 
    }

    if (g_onOffDebugPublisher)
    {   
        printf("	----publisher %s output %d byte----\r\n", name, length); 
    }

    printf("	----lenght is %d----\r\n", length);  

    if (length > DISPLAY_MAX_BIN_LENGTH)
    {      
        length = DISPLAY_MAX_BIN_LENGTH;
    }

    HexDump((u8*)buf, length);

    if (length > DISPLAY_MAX_BIN_LENGTH)
    {      
        printf("... ... shenglue\r\n"); 
    }

    return ENUM_YES;
}


static int PublisherDebugClose(char* name, SStateElement* tempAction)
{
    if (g_onOffDebugPublisher)
    {   
        printf("	----call PublisherDebugClose----\r\n"); 
    }

    return ENUM_YES;
}

static struct PublisherGroup publisher_debug = {
    "debug",
    0xffffffff,
    PublisherDebugOpen,
    PublisherDebugOutput,
    PublisherDebugClose
};

void InitPublisherDebug(void)
{
    RegisterPublisherGroup(&publisher_debug);
}

#ifdef __cplusplus
}
#endif

#endif // Open_source
