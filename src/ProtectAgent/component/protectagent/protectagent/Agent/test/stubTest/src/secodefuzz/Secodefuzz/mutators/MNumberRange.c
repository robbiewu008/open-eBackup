/*
��Ȩ���� (c) ��Ϊ�������޹�˾ 2012-2018


ԭ��:					��Χ���ֱ���

����:					���Ȳ���			

����:					��Χ������500����Сֵ

֧����������: 	��Χ��������
*/
#include "../common/PCommon.h"

#ifdef __cplusplus
extern "C" {
#endif

int NumberRangeGetCount(SElement *pElement)
{
    ASSERT_NULL(pElement);

    return MAX_COUNT;
}

char* NumberRangeVarianceGetValue(SElement *pElement, int pos)
{
    int len;
    ASSERT_NULL(pElement);

    // ���bit���Ȳ���8���������1
    len = (int)(pElement->inLen / 8) + IS_ADD_ONE(pElement->inLen % 8);

    SetElementInitoutBufEx(pElement, len);

    s64 sValue = GaussRandU64(pos % 64);

    sValue = sValue + pElement->min;

    if (sValue > pElement->max)
        sValue = pElement->max;

    if (sValue < pElement->min)
        sValue = pElement->min;

    *((s32 *)pElement->para.value) = sValue;
    
    return pElement->para.value;
}

char* NumberRange1GetValue(SElement *pElement, int pos)
{
    int len;
    ASSERT_NULL(pElement);

    // ���bit���Ȳ���8���������1
    len = (int)(pElement->inLen / 8) + IS_ADD_ONE(pElement->inLen % 8);

    SetElementInitoutBufEx(pElement, len);

    *((s32 *)pElement->para.value) = RAND_RANGE(pElement->min, pElement->max);
    
    return pElement->para.value;
}

static char* NumberRangeGetValue(SElement *pElement, int pos)
{
    if (RAND_32() % 2 == 0)
    {
        return NumberRange1GetValue(pElement, pos);
    }
    else
    {
        return NumberRangeVarianceGetValue(pElement, pos);
    }
}

int NumberRangeGetIsSupport(SElement *pElement)
{
    ASSERT_NULL(pElement);
    if (pElement->para.type == ENUM_NUMBER_RANGE)
    {
        return ENUM_YES;
    }

    return ENUM_NO;
}

const struct MutaterGroup g_numberRangeGroup = {
    "NumberRange",
    NumberRangeGetCount,
    NumberRangeGetValue,
    NumberRangeGetIsSupport,
    1
};

void InitNumberRange(void)
{
    RegisterMutater(&g_numberRangeGroup, ENUM_NUMBER_RANGE_M);
}

#ifdef __cplusplus
}
#endif

