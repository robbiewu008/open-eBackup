/*
版权所有 (c) 华为技术有限公司 2012-2018


原理:					获取初始值周围加减50的值,供100个测试例，不考虑溢出，翻转等

长度:					长度不变			(字符串的话最大33 或最大输出)	

数量:					100

支持数据类型: 	整数和有数字初始值的字符串

*/
#include "../common/PCommon.h"

#ifdef __cplusplus
extern "C" {
#endif

int NumberSmallRangeGetCount(SElement *pElement)
{
    // 仔细想想，管他溢出还是翻转，来吧
    return MAX_COUNT / 5;
}

char* NumberSmallRangeGetValue(SElement *pElement, int pos)
{
    int len;
    ASSERT_NULL(pElement);

    if ((pElement->para.type == ENUM_STRING) || (pElement->para.type == ENUM_STRING_NUM))
    {
        SetElementInitoutBufEx(pElement, STRING_NUMBER_LEN);
        
        s64 temp = (*(s64  *)pElement->numberValue + pos - 50);
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
            *((s8 *)pElement->para.value)  = *(s8 *)pElement->numberValue + pos - 50;
        }
        else if (pElement->inLen <= 16)
        {
            *((s16 *)pElement->para.value) = *(s16 *)pElement->numberValue + pos - 50;
        }
        else if (pElement->inLen <= 32)
        {
            *((s32 *)pElement->para.value) = *(s32 *)pElement->numberValue + pos - 50;
        }
        else if (pElement->inLen <= 64)
        {
            *((s64 *)pElement->para.value) = *(s64 *)pElement->numberValue + pos - 50;
        }
    }
    else if (pElement->para.type == ENUM_NUMBER_U)
    {
        if (pElement->inLen <= 8)
        {
            *((u8 *)pElement->para.value)  = *(u8 *)pElement->numberValue + pos - 50;
        }
        else if (pElement->inLen <= 16)
        {
            *((u16 *)pElement->para.value) = *(u16 *)pElement->numberValue + pos - 50;
        }
        else if (pElement->inLen <= 32)
        {
            *((u32 *)pElement->para.value) = *(u32 *)pElement->numberValue + pos - 50;
        }
        else if (pElement->inLen <= 64)
        {
            *((u64 *)pElement->para.value) = *(u64 *)pElement->numberValue + pos - 50;
        }
    }
    
    return pElement->para.value;
}

int NumberSmallRangeGetIsSupport(SElement *pElement)
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

const struct MutaterGroup g_numberSmallRangeGroup = {
    "NumberSmallRange",
    NumberSmallRangeGetCount,
    NumberSmallRangeGetValue,
    NumberSmallRangeGetIsSupport,
    1
};

void InitNumberSmallRange(void)
{
    RegisterMutater(&g_numberSmallRangeGroup, ENUM_NUMBER_SMALL_RANGE);
}

#ifdef __cplusplus
}
#endif

