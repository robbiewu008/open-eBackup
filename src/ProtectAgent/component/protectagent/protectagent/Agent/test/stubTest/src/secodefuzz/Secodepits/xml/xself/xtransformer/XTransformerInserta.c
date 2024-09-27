/*
��Ȩ���� (c) ��Ϊ�������޹�˾ 2012-2018


��Ҫ���ڼӽ��ܣ������֮��Ĳ���

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

#include "../../XML.h"

#ifdef __cplusplus
extern "C" {
#endif

// ������ʱ����
static void InsertaTransformer(SBinElement* temp)
{
    SXMLElement * tempXml = temp->mutatorElement->xmlElement;
    SBinElement * ref = NULL;

    // Ӧ���ҵ��ľ����Լ�  ,block��˵
    ref = BinElementFoundRelationofByName( temp, tempXml->paramValue1[0]);
    if (ref == NULL)
    {
        return;
    }

    if (g_onOffDebugDoTransformer)
    {
        printf("		    ref=%s\r\n", ref->xpathName);
    }

    int length = GetPitsBufLength(ref);

    char* buf = malloc(length);
    GetPitsBufNoMutator(ref, buf);

    char* buf1 = malloc(length * 2);
    g_onerunMemory[g_onerunMemoryCount++] = buf1;

    int i = 0;
    for (i = 0; i < length; i++)
    {
        buf1[2 * i] =  buf[i];
        buf1[2 * i + 1] = 'a';
    }

    temp->mutaterValue = buf1;
    temp->mutaterLength = length * 2;

    free(buf);
} 

static struct TransformerGroup g_InsertaTransformer = {
    "inserta",
    0xffffffff,
    InsertaTransformer,
};

void InitInsertaTransformer(void)
{
    RegisterTransformerGroup(&g_InsertaTransformer);
}

#ifdef __cplusplus
}
#endif
