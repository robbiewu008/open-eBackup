/*
版权所有 (c) 华为技术有限公司 2012-2018


IcmpChecksum实现

*/

#include "../../XML.h"

#ifdef _WIN32
#include<ws2tcpip.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

static void DoIcmpChecksumFixup(SBinElement* temp)
{
    SXMLElement * tempXml = temp->mutatorElement->xmlElement;
    SBinElement * ref = NULL;

    Hw1Memset(temp->mutaterValue, 0, temp->mutaterLength);

    //
    ref = BinElementFoundRelationofByName( temp, tempXml->paramValue[0]);

    if (ref == NULL)
    {
        return;
    }

    if (g_onOffDebugDoFixup)
    {
        printf("		    ref=%s\r\n", ref->xpathName);
    }

    int length = GetPitsBufLength(ref);

    char* buf = g_outBuf2;
    buf[length] = 0;

    GetPitsBufNoMutator(ref, buf);

    unsigned short csum = CheckSum((unsigned short *)buf, (length + 1) / 2);
    *(unsigned short*)temp->mutaterValue = htons(csum);
} 

static struct FixupGroup g_icmpChecksumFixup = {
    "checksums.IcmpChecksumFixup",
    0xffffffff,
    DoIcmpChecksumFixup,
};


void InitIcmpChecksumFixup(void)
{
    RegisterFixupGroup(&g_icmpChecksumFixup);
}

#ifdef __cplusplus
}
#endif
