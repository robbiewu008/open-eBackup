/*
版权所有 (c) 华为技术有限公司 2012-2018


crc32实现
*/

#include "../../XML.h"

#ifdef _WIN32
#include<ws2tcpip.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

static void DoCrc32Fixup(SBinElement* temp)
{
    SXMLElement* tempXml = temp->mutatorElement->xmlElement;
    SBinElement* ref = NULL;

    Hw1Memset(temp->mutaterValue, 0, temp->mutaterLength);

    //
    ref = BinElementFoundRelationofByName(temp, tempXml->paramValue[0]);
    if (ref == NULL)
    {
        return;
    }

    if (g_onOffDebugDoFixup)
    {
        printf("		    ref=%s\r\n",ref->xpathName);
    }

    int length = GetPitsBufLength(ref);

    char* buf = g_outBuf2;

    GetPitsBufNoMutator(ref, buf);

    unsigned int csum = CheckSum32((unsigned char *)buf, length);
    *(unsigned int*)temp->mutaterValue = htonl(csum);
} 

static struct FixupGroup g_crc32Fixup = {
    "crc32",
    0xffffffff,
    DoCrc32Fixup,
};

void InitCrc32Fixup(void)
{
    RegisterFixupGroup(&g_crc32Fixup);
}

#ifdef __cplusplus
}
#endif
