/*
版权所有 (c) 华为技术有限公司 2012-2018

简单的定制化number类型变异算法，
只要在表中填入数据，你的数据就会出现在number类型元素的测试例里

*/
#include "../common/PCommon.h"

#ifdef __cplusplus
extern "C" {
#endif

int CustomNumberGetCount(SElement *pElement)
{
    ASSERT_NULL(pElement);
    int count = g_customNumberTableCount;

    if (count == 0)
    {
        return 0;
    }
    
    return MAX_COUNT / 2;
}

char* CustomNumberGetValue(SElement *pElement, int pos)
{
    int len;
    int count = g_customNumberTableCount;
    u64 value;
    ASSERT_NULL(pElement);
    // 如果bit长度不被8整除，则加1
    len = (int)(pElement->inLen / 8) + IS_ADD_ONE(pElement->inLen % 8);

    // 库没有值，则空
    if (count == 0)
    {
        value = 0;
    }
    else
    {
        value = g_customNumberTable[RAND_32() % count];
    }

    SetElementInitoutBufEx(pElement, len);

    // 有无符号分别对待
    if (pElement->para.type == ENUM_NUMBER_S)
    {
        if (pElement->inLen <= 8)
        {
            *((s8 *)pElement->para.value) = (s8)value;
        }
        else if (pElement->inLen <= 16)
        {
            *((s16 *)pElement->para.value) = (s16)value;
        }
        else if (pElement->inLen <= 32)
        {
            *((s32 *)pElement->para.value) = (s32)value;
        }
        else if (pElement->inLen <= 64)
        {
            *((s64 *)pElement->para.value) = value;
        }
    }
    else if (pElement->para.type == ENUM_NUMBER_U)
    {
        if (pElement->inLen <= 8)
        {
            *((u8 *)pElement->para.value) = (u8)value;
        }
        else if (pElement->inLen <= 16)
        {
            *((u16 *)pElement->para.value) = (u16)value;
        }
        else if (pElement->inLen <= 32)
        {
            *((u32 *)pElement->para.value) = (u32)value;
        }
        else if (pElement->inLen <= 64)
        {
            *((u64 *)pElement->para.value) = value;
        }
    }

    return pElement->para.value;
}

int CustomNumberGetIsSupport(SElement *pElement)
{
    ASSERT_NULL(pElement);
    if ((pElement->para.type == ENUM_NUMBER_S)
        || (pElement->para.type == ENUM_NUMBER_U))
    {
        return ENUM_YES;
    }

    return ENUM_NO;
}

const struct MutaterGroup g_customNumberGroup = {
    "CustomNumber",
    CustomNumberGetCount,
    CustomNumberGetValue,
    CustomNumberGetIsSupport,
    1
};

void InitCustomNumber(void)
{
    RegisterMutater(&g_customNumberGroup, ENUM_CUSTOM_NUMBER);
}

#ifdef __cplusplus
}
#endif