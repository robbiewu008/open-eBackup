/*
��Ȩ���� (c) ��Ϊ�������޹�˾ 2012-2018


ԭ��:					�㷨ͨ���ı����ݵ�����bitֵ��������������,
						һ����������ֻ�ı�һ��bit
						

����:					���Ȳ���		

����:					bit��

֧����������: 	�г�ֵ,ö�������

*/
#include "../common/PCommon.h"

#ifdef __cplusplus
extern "C" {
#endif

int DataElementBitFlipperGetCount(SElement *pElement)
{
    ASSERT_NULL(pElement);

    return  MIN(pElement->inLen, MAX_COUNT * 2);
}

char* DataElementBitFlipperGetValue(SElement *pElement, int pos)
{
    // ���bit���Ȳ���8���������1
    int inLen; 
    ASSERT_NULL(pElement);

    pos = RAND_RANGE(0, pElement->inLen - 1);
    
    inLen = (int)(pElement->inLen / 8) + IS_ADD_ONE(pElement->inLen % 8);

    SetElementInitoutBufEx(pElement, inLen);
    
    HwMemcpy(pElement->para.value, pElement->inBuf, inLen);

    FLIP_BIT(pElement->para.value, pos);

    return   pElement->para.value;
}

int DataElementBitFlipperGetIsSupport(SElement *pElement)
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

const struct MutaterGroup g_dataElementBitFlipperGroup = {
    "DataElementBitFlipper",
    DataElementBitFlipperGetCount,
    DataElementBitFlipperGetValue,
    DataElementBitFlipperGetIsSupport,
    1
};

void InitDataElementBitFlipper(void)
{
    RegisterMutater(&g_dataElementBitFlipperGroup, ENUM_DATAELEMENT_BIT_FLIPPER);
}

#ifdef __cplusplus
}
#endif

