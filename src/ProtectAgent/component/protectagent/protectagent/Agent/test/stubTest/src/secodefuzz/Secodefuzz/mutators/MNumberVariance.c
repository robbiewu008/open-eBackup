/*
版权所有 (c) 华为技术有限公司 2012-2018

原理:					获取以初始值为中心，高斯变异的测试例

长度:					长度不变			(字符串的话最大33 或最大输出)	

数量:					bit值乘4

支持数据类型: 	整数和有数字初始值的字符串

*/
#include "../common/PCommon.h"

#ifdef __cplusplus
extern "C" {
#endif

int NumberVarianceGetCount(SElement *pElement)
{
    // 字符串数字按照s64等同
    if (pElement->para.type == ENUM_STRING)
    {
        return 64 * 2 * 2;
    }

    return pElement->inLen * 2 * 2;
}

char* NumberVarianceGetValue(SElement *pElement, int pos)
{
    int len;
    s64 sValue;

    ASSERT_NULL(pElement);

    if ((pElement->para.type == ENUM_STRING) || (pElement->para.type == ENUM_STRING_NUM))
    {
        SetElementInitoutBufEx(pElement, STRING_NUMBER_LEN);

        sValue = GaussRandS64(pos % 64);
        
        s64 temp = (s64)sValue + *(s64 *)pElement->numberValue;
        Inltoa(temp, pElement->para.value, 10);

        len = Instrlen(pElement->para.value) + 1;
        if (len > pElement->para.maxLen)
        {
            len = pElement->para.maxLen;
        }

        pElement->para.value[len - 1] = 0;

        // 重置长度为字符串实际长度
        pElement->para.len = len;

        return pElement->para.value;

    }

    // 如果bit长度不被8整除，则加1
    len = (int)(pElement->inLen / 8) + IS_ADD_ONE(pElement->inLen % 8);

    SetElementInitoutBufEx(pElement, len);

    if (pElement->para.type == ENUM_NUMBER_S)
    {
        switch (len)
        {
            case 1:
                *((s8  *)pElement->para.value) = (s8)GaussRandS64(pos % 8) + *(s8  *)pElement->numberValue;
                break;
            case 2:
                *((s16 *)pElement->para.value) = (s16)GaussRandS64(pos % 16) + *(s16 *)pElement->numberValue;
                break;
            case 4:
                *((s32 *)pElement->para.value) = (s32)GaussRandS64(pos % 32) + *(s32 *)pElement->numberValue;
                break;
            case 8:
                *((s64 *)pElement->para.value) = (s64)GaussRandS64(pos % 64) + *(s64 *)pElement->numberValue;
                break;
            default:
                break;
        }
    }
    else
    {
        switch (len)
        {
            case 1:
                *((u8  *)pElement->para.value) = (s8)GaussRandS64(pos % 8) + *(u8  *)pElement->numberValue;
                break;
            case 2:
                *((u16 *)pElement->para.value) = (s16)GaussRandS64(pos % 16) + *(u16 *)pElement->numberValue;
                break;
            case 4:
                *((u32 *)pElement->para.value) = (s32)GaussRandS64(pos % 32) + *(u32 *)pElement->numberValue;
                break;
            case 8:
                *((u64 *)pElement->para.value) = (s64)GaussRandS64(pos % 64) + *(u64 *)pElement->numberValue;
                break;
            default:  
                break;
        }
    }

    return pElement->para.value;
}

int NumberVarianceGetIsSupport(SElement *pElement)
{
    ASSERT_NULL(pElement);
    if (((pElement->para.type == ENUM_NUMBER_S)
        || (pElement->para.type == ENUM_NUMBER_U))
        && (pElement->isHasInitValue == ENUM_YES))
    {
        return ENUM_YES;
    }

    if (InStringIsNumber(pElement) == ENUM_YES)
    {
        return ENUM_YES;
    }

    return ENUM_NO;
}

const struct MutaterGroup g_numberVarianceGroup = {
    "NumberVariance",
    NumberVarianceGetCount,
    NumberVarianceGetValue,
    NumberVarianceGetIsSupport,
    1
};

void InitNumberVariance(void)
{
    RegisterMutater(&g_numberVarianceGroup, ENUM_NUMBER_VARIANCE);
}

#ifdef __cplusplus
}
#endif