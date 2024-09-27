/*
版权所有 (c) 华为技术有限公司 2012-2018


UDPChecksum实现
*/

#include "../../XML.h"

#ifdef _WIN32
#include<ws2tcpip.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

static void DoUDPChecksumFixup(SBinElement* temp)
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
        printf("		    ref=%s\r\n", ref->xpathName);
    }

    int length = GetPitsBufLength(ref);

    u32 src = htonl(FromIpstrToUint(tempXml->paramValue[1]));
    u32 dst = htonl(FromIpstrToUint(tempXml->paramValue[2]));

    memcpy(g_outBuf1, &src, 4);
    memcpy(g_outBuf1 + 4, &dst, 4);
    g_outBuf1[8] = 0;
    g_outBuf1[9] = 17;

    unsigned short aaa = length;
    aaa = htons(aaa);
    char* bbb = (char*)&aaa;
    memcpy(g_outBuf1 + 10, bbb, 2);

    char* buf = g_outBuf2;
    buf[length] = 0;
    GetPitsBufNoMutator(ref, buf);
    memcpy(g_outBuf1 + 12, buf, length + 1);

    unsigned short csum = CheckSum((unsigned short *)g_outBuf1, (length + 12 + 1) / 2);

    temp->mutaterValue = malloc(2);
    g_onerunMemory[g_onerunMemoryCount++] = temp->mutaterValue;
    *(unsigned short*)temp->mutaterValue = htons(csum);
} 

static struct FixupGroup g_UDPChecksumFixup = {
    "UDPChecksumFixup",
    0xffffffff,
    DoUDPChecksumFixup,
};

void InitUDPChecksumFixup(void)
{
    RegisterFixupGroup(&g_UDPChecksumFixup);
}

#ifdef __cplusplus
}
#endif
