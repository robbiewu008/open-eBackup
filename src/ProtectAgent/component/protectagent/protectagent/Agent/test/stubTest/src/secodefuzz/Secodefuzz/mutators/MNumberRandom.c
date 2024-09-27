/*
版权所有 (c) 华为技术有限公司 2012-2018


原理:					产生基础元素范围内的随机数

长度:					长度不变			(字符串的话最大33 或最大输出)	

数量:					最大数开平方与500的最小值

支持数据类型: 	整数和有数字初始值的字符串

*/
#include "../common/PCommon.h"

#ifdef __cplusplus
extern "C" {
#endif

#define maxValue64 0xffffffffffffffff
#define maxValue   0xffffffff

int NumberRandomGetCount(SElement *pElement)
{
    ASSERT_NULL(pElement);
    // 字符串数字按照s64等同
    if (pElement->para.type == ENUM_STRING)
    {
        return MIN(Insqrt(maxValue64 >> (64 - 64)), MAX_COUNT);
    }

    return MIN(Insqrt(maxValue64 >> (64 - pElement->inLen)), MAX_COUNT);
}

char* NumberRandomGetValue(SElement *pElement, int pos)
{
    int len;
    ASSERT_NULL(pElement);

    if ((pElement->para.type == ENUM_STRING) || (pElement->para.type == ENUM_STRING_NUM))
    {
        SetElementInitoutBufEx(pElement, STRING_NUMBER_LEN);
        
        s64 temp = (s64)(RAND_64() & (maxValue64 >> (64 - pElement->inLen)));
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

    // 有无符号分别对待
    if (pElement->para.type == ENUM_NUMBER_S)
    {
        if (pElement->inLen <= 8)
        {
            *((s8 *)pElement->para.value) = (u8)(RAND_64() & (maxValue64 >> (64 - pElement->inLen)));
        }
        else if (pElement->inLen <= 16)
        {
            *((s16 *)pElement->para.value) = (u16)(RAND_64() & (maxValue64 >> (64 - pElement->inLen)));
        }
        else if (pElement->inLen <= 32)
        {
            *((s32 *)pElement->para.value) = (u32)(RAND_64() & (maxValue64 >> (64 - pElement->inLen)));
        }
        else if (pElement->inLen <= 64)
        {
            *((s64 *)pElement->para.value) = (u64)(RAND_64() & (maxValue64 >> (64 - pElement->inLen)));
        }
    }
    else if (pElement->para.type == ENUM_NUMBER_U)
    {
        if (pElement->inLen <= 8)
        {
            *((u8 *)pElement->para.value) = (u8)(RAND_64() & (maxValue64 >> (64 - pElement->inLen)));
        }
        else if (pElement->inLen <= 16)
        {
            *((u16 *)pElement->para.value) = (u16)(RAND_64() & (maxValue64 >> (64 - pElement->inLen)));
        }
        else if (pElement->inLen <= 32)
        {
            *((u32 *)pElement->para.value) = (u32)(RAND_64() & (maxValue64 >> (64 - pElement->inLen)));
        }
        else if (pElement->inLen <= 64)
        {
            *((u64 *)pElement->para.value) = (u64)(RAND_64() & (maxValue64 >> (64 - pElement->inLen)));
        }
    }
    
    return pElement->para.value;
}


int NumberRandomGetIsSupport(SElement *pElement)
{
    ASSERT_NULL(pElement);
    if ((pElement->para.type == ENUM_NUMBER_S)
        || (pElement->para.type == ENUM_NUMBER_U))
    {
        return ENUM_YES;
    }

    if (InStringIsNumber(pElement) == ENUM_YES)
    {
        return ENUM_YES;
    }

    return ENUM_NO;
}

const struct MutaterGroup g_numberRandomGroup = {
    "NumberRandom",
    NumberRandomGetCount,
    NumberRandomGetValue,
    NumberRandomGetIsSupport,
    1
};

void InitNumberRandom(void)
{
    RegisterMutater(&g_numberRandomGroup, ENUM_NUMBER_RANDOM);
}

#ifdef __cplusplus
}
#endif

