/*
版权所有 (c) 华为技术有限公司 2012-2018


分发fixup的函数

<DataModel name="crc32_datamodel" >
	<Blob name="Data" value="huawei is ok!" />
	<Number name="CRC" size="32" >
		<Fixup class="crc32">
			<Param name="ref" value="Data" />
		</Fixup>
	</Number>
</DataModel>

*/

#include "../XML.h"

#ifdef __cplusplus
extern "C" {
#endif

struct FixupGroup* g_fixupGroup[MAX_SELF_COUNT];
int g_fixupCount = 0;

static int GetFixupNo_ByName(char* name)
{
    int i = 0;
    for (i = 0; i < g_fixupCount; i++)
    {
        if (strcmp(g_fixupGroup[i]->name, name) == 0)
        {      
            break;
        }
    }

    if (i == g_fixupCount)
        ; // 挂死

    return i;
}

void DoFixup(SBinElement* temp)
{
    For_Tree_Start(temp)
    {
        SXMLElement * tempXml = temp->mutatorElement->xmlElement;
        if ((tempXml != NULL)
            && (tempXml->isFixup == 1))
        {
            if (g_onOffDebugDoFixup)
            {
                printf("	do fixup :xpath=%s\r\n", temp->xpathName);
            }

            if (g_onOffDebugDoFixup)
            {
                printf("	          class=%s\r\n", tempXml->className);
            }

            int no = GetFixupNo_ByName(tempXml->className);
            g_fixupGroup[no]->fixup(temp);
        }
    }
    For_Tree_End(temp)
} 

int RegisterFixupGroup( struct FixupGroup* fixupGroup)
{
    g_fixupGroup[g_fixupCount] = fixupGroup;
    fixupGroup->no = g_fixupCount;
    g_fixupCount ++;
    
    return 1;
}

extern void InitCrc32Fixup(void);
extern void InitIcmpChecksumFixup(void);
extern void InitUDPChecksumFixup(void);

void DoFixupInit(void)
{
    InitCrc32Fixup();
    InitIcmpChecksumFixup();
    InitUDPChecksumFixup();
}

#ifdef __cplusplus
}
#endif
