/*
��Ȩ���� (c) ��Ϊ�������޹�˾ 2012-2018

ԭ��:					ԭ�ַ�������1-6��bom

֧����������: 	�г�ʼֵ���ַ�������
*/
#include "../common/PCommon.h"

#ifdef __cplusplus
extern "C" {
#endif

int StringUtf8BomGetCount(SElement *pElement)
{
    ASSERT_NULL(pElement);

    // �����ٵ�ɣ��ⶫ��ûɶ����
    return (MAX_COUNT / 100);
}

char* StringUtf8BomGetValue(SElement *pElement, int pos)
{
    int i;
    int j;
    int outLen;

    ASSERT_NULL(pElement);

    // ���Ӽ���bom
    int bomNum = RAND_RANGE(1, 6);

    outLen = (int)(pElement->inLen / 8);

    if (((outLen + bomNum * g_bomLen) > pElement->para.maxLen) || (outLen == 0))
    {
        return SetElementOriginalValue(pElement);
    }

    SetElementInitoutBufEx(pElement, outLen + bomNum * g_bomLen);

    // ����ʣ�µ�
    HwMemcpy(pElement->para.value, pElement->inBuf, outLen); 

    for (i = 0; i < bomNum; i++)
    { 
        // �������bom��λ��
        int pos1=RAND_RANGE(0, outLen);
        // ��copy��ߣ���copyǰ��

        for (j = outLen; j > pos1; j--)
        {
            pElement->para.value[j - 1 + g_bomLen ] = pElement->para.value[j - 1];
        }

        HwMemcpy(pElement->para.value + pos1, g_bom, g_bomLen); 

        outLen = outLen + g_bomLen;
    }

    ((char *)pElement->para.value)[outLen - 1] = 0; 
    return pElement->para.value;
}

int StringUtf8BomGetIsSupport(SElement *pElement)
{
    ASSERT_NULL(pElement);

    // �г�ʼֵ���ַ���
    if ((pElement->para.type == ENUM_STRING)
        && (pElement->isHasInitValue == ENUM_YES)
        && ((int)(pElement->inLen / 8) > 1))
    {
        return ENUM_YES;
    }

    return ENUM_NO;
}

const struct MutaterGroup g_stringUtf8BomGroup = {
    .name 			= "StringUtf8Bom",
    .getCount 		= StringUtf8BomGetCount,
    .getValue 		= StringUtf8BomGetValue,
    .getIsSupport 	= StringUtf8BomGetIsSupport,
};

void InitStringUtf8Bom(void)
{
    RegisterMutater(&g_stringUtf8BomGroup, ENUM_STRING_UTF8_BOM);
}

#ifdef __cplusplus
}
#endif

