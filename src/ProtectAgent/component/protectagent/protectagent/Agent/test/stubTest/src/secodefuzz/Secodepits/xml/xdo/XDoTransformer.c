/*
版权所有 (c) 华为技术有限公司 2012-2018


Transformer主要用于加解密，编解码之类的操作

<DataModel name="inserta_datamodel" >
	<Block name="Data" >
		<Blob value="huawei is ok!" />
		<Blob value="123456789" />
		<Transformer class="inserta">
			<Param name="ref" value="Data" />
		</Transformer>
	</Block >
</DataModel>

*/

#include "../XML.h"

#ifdef __cplusplus
extern "C" {
#endif

struct TransformerGroup* g_transformerGroup[MAX_SELF_COUNT];
int g_transformerCount = 0;


static int GetTransformerNo_ByName(char* name)
{
    int i = 0;
    for (i = 0; i < g_transformerCount; i++)
    {
        if (strcmp(g_transformerGroup[i]->name, name) == 0)
        {      
            break;
        }
    }

    if (i == g_transformerCount)
        ; // 挂死

    return i;
}

void DoTransformer(SBinElement* temp)
{
    For_Tree_Start(temp)
    {
        SXMLElement * tempXml = temp->mutatorElement->xmlElement;
        if ((tempXml != NULL)
        && (tempXml->isTransformer == 1))
        {
            if (g_onOffDebugDoTransformer)
            {
                printf("	do transformer :xpath=%s\r\n", temp->xpathName);
            }

            if (g_onOffDebugDoTransformer)
            {
                printf("	          class=%s\r\n", tempXml->className1);
            }

            int no = GetTransformerNo_ByName(tempXml->className1);
            g_transformerGroup[no]->transformer(temp);
        }
    }
    For_Tree_End(temp)
} 

int RegisterTransformerGroup( struct TransformerGroup* transformerGroup)
{
    g_transformerGroup[g_transformerCount] = transformerGroup;
    transformerGroup->no = g_transformerCount;
    g_transformerCount ++;
    
    return 1;
}

extern void InitInsertaTransformer(void);

void DoTransformerInit(void)
{
    InitInsertaTransformer();
}

#ifdef __cplusplus
}
#endif
