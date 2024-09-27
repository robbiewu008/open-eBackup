/*
��Ȩ���� (c) ��Ϊ�������޹�˾ 2012-2018


ԭ��:					������bit��1����ͷ��1�ʹ�β��1����1��������Ϊ1������
						

����:					���Ȳ���		

����:					bit����2

֧����������: 	�г�ֵ,ö�������

*/
#include "../common/PCommon.h"

#ifdef __cplusplus
extern "C" {
#endif

int DataElementBitFillGetCount(SElement *pElement)
{
    ASSERT_NULL(pElement);

    return  MIN(pElement->inLen * 2, MAX_COUNT / 5);
}

char* DataElementBitFillGetValue(SElement *pElement, int pos)
{
    // ���bit���Ȳ���8���������1
    int inLen, i;
    ASSERT_NULL(pElement);

    pos = RAND_RANGE(0, pElement->inLen * 2 - 1);
    
    inLen = (int)(pElement->inLen / 8) + IS_ADD_ONE(pElement->inLen % 8);

    SetElementInitoutBufEx(pElement, inLen);
    
    HwMemcpy(pElement->para.value, pElement->inBuf, inLen);

    if (pos < pElement->inLen)
    {
        for (i = 0; i <= pos; i++)
        {
            FILL_BIT(pElement->para.value, i);
        }
    }
    else
    {
        pos = pos - pElement->inLen;
        for (i = pos; i < pElement->inLen; i++)
        {
            FILL_BIT(pElement->para.value, i);
        }
    }

    return   pElement->para.value;
}

int DataElementBitFillGetIsSupport(SElement *pElement)
{
    ASSERT_NULL(pElement);

    // ö�ٲ�֧��
    if (InGetTypeIsEnumOrRange(pElement->para.type) == ENUM_YES)
    {
        return ENUM_NO;
    }

    if (pElement->para.type == ENUM_STRING_NUM)
    {
        return ENUM_NO;
    }

    // self�����㷨�����Σ���Ҫ���Լ���
    if (pElement->para.type == ENUM_TSELF)
    {
        return ENUM_NO;
    }
    
    // ֻҪ�г�ʼֵ����֧�֣��������������Ϳ����������?
    if (pElement->isHasInitValue == ENUM_YES)
    {
        return ENUM_YES;
    }
    
    return ENUM_NO;
}

const struct MutaterGroup g_dataElementBitFillGroup = {
    "DataElementBitFill",
    DataElementBitFillGetCount,
    DataElementBitFillGetValue,
    DataElementBitFillGetIsSupport,
    1
};

void InitDataElementBitFill(void)
{
    RegisterMutater(&g_dataElementBitFillGroup, ENUM_DATAELEMENT_BIT_FILL);
}

#ifdef __cplusplus
}
#endif

