/*
��Ȩ���� (c) ��Ϊ�������޹�˾ 2012-2018

ԭ��:					�㷨ͨ���ı����ݼ���bitֵ��������������,
						һ����������ֻ�ı�2-6��bit
						

����:					���Ȳ���		

����:					bit����2��MAXCOUNT/5����Сֵ

֧����������: 	�г�ֵ,ö�������

*/
#include "../common/PCommon.h"

#ifdef __cplusplus
extern "C" {
#endif

int DataElementMBitFlipperGetCount(SElement *pElement)
{
    ASSERT_NULL(pElement);

    return MIN((int)pElement->inLen * 2, MAX_COUNT / 5);
}

char* DataElementMBitFlipperGetValue(SElement *pElement, int pos)
{
    // ���bit���Ȳ���8���������1
    int inLen;
    int count;
    ASSERT_NULL(pElement);
    
    inLen = (int)(pElement->inLen / 8) + IS_ADD_ONE(pElement->inLen % 8);

    SetElementInitoutBufEx(pElement, inLen);
    
    HwMemcpy(pElement->para.value, pElement->inBuf, inLen);

    count = RAND_RANGE(2, 6);
    while (count--)
    {
        FLIP_BIT(pElement->para.value, RAND_RANGE(0, pElement->inLen - 1));
    }
    
    return   pElement->para.value;
}

int DataElementMBitFlipperGetIsSupport(SElement *pElement)
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
    
    // ֻҪ�г�ʼֵ����֧��
    if (pElement->isHasInitValue == ENUM_YES)
    {
        return ENUM_YES;
    }

    return ENUM_NO;
}

const struct MutaterGroup g_dataElementMBitFlipperGroup = {
    "DataElementMBitFlipper",
    DataElementMBitFlipperGetCount,
    DataElementMBitFlipperGetValue,
    DataElementMBitFlipperGetIsSupport,
    1
};

void InitDataElementMBitFlipper(void)
{
    RegisterMutater(&g_dataElementMBitFlipperGroup, ENUM_DATAELEMENT_MBIT_FLIPPER);
}

#ifdef __cplusplus
}
#endif