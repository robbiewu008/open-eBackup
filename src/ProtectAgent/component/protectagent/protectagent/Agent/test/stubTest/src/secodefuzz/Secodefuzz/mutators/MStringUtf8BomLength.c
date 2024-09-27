/*
��Ȩ���� (c) ��Ϊ�������޹�˾ 2012-2018


ԭ��:					�����ַ�������  ��
						�����㷨Ϊ��˹���죬
						ʹ���ַ����������
						֮���ڲ���1-6��bom
						

����:					���ֵ��1֮����죬						

����:					���������ȿ�ƽ����MAXCOUNT/5����Сֵ

֧����������: 	�г�ʼֵ���ַ�������
*/
#include "../common/PCommon.h"

#ifdef __cplusplus
extern "C" {
#endif

int StringUtf8BomLengthGetCount(SElement *pElement)
{
    ASSERT_NULL(pElement);

    // �����ٵ�ɣ��ⶫ��ûɶ����
    return (MAX_COUNT/100);
}

char* StringUtf8BomLengthGetValue(SElement *pElement, int pos)
{
    int i;
    int j;
    int outLen;
    int inLen;

    ASSERT_NULL(pElement);

    // ���Ӽ���bom
    int bomNum = RAND_RANGE(1, 6);

    if (pos == 0)
    {
        pos = 1;
    }

    inLen = (int)(pElement->inLen / 8);
    outLen = GaussRandU32(pos % InGetBitNumber(pElement->para.maxLen));
    if (((outLen + bomNum * g_bomLen) > pElement->para.maxLen) || (outLen == 0))
    {
        return SetElementOriginalValue(pElement);
    }

    SetElementInitoutBufEx(pElement, outLen + bomNum * g_bomLen);

    for (i = 0; i < outLen / (inLen - 1); i++)
    { 
        HwMemcpy(pElement->para.value +  i * (inLen - 1), pElement->inBuf, (inLen - 1)); 
    }

    // ����ʣ�µ�
    HwMemcpy(pElement->para.value +  i * (inLen - 1), pElement->inBuf, outLen - (outLen / (inLen - 1)) * (inLen - 1)); 

    for (i = 0; i < bomNum; i++)
    { 
        // �������bom��λ��
        int pos1=RAND_RANGE(0, outLen);

        // ��copy��ߣ���copyǰ��
        for (j=outLen; j>pos1; j--)
        {
            pElement->para.value[j - 1 + g_bomLen ] = pElement->para.value[j - 1];
        }

        HwMemcpy(pElement->para.value + pos1, g_bom, g_bomLen); 

        outLen = outLen + g_bomLen;
    }

    ((char *)pElement->para.value)[outLen - 1] = 0; 
    return pElement->para.value;
}

int StringUtf8BomLengthGetIsSupport(SElement *pElement)
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

const struct MutaterGroup g_stringUtf8BomLengthGroup = {
    .name 		= "StringUtf8BomLength",
    .getCount 		= StringUtf8BomLengthGetCount,
    .getValue 		= StringUtf8BomLengthGetValue,
    .getIsSupport 	= StringUtf8BomLengthGetIsSupport,
};

void InitStringUtf8BomLength(void)
{
    RegisterMutater(&g_stringUtf8BomLengthGroup, ENUM_STRING_UTF8_BOM_LENGTH);
}

#ifdef __cplusplus
}
#endif

