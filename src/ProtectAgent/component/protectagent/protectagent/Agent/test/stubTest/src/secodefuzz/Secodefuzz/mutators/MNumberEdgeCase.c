/*
版权所有 (c) 华为技术有限公司 2012-2018

原理:					根据元素基础空间，变异各种大小的bit临界值的整数
						

长度:					长度不变	(字符串的话最大33 或最大输出)	

数量:					bit数乘2 *2

支持数据类型: 	整数和有数字初始值的字符串

*/
#include "../common/PCommon.h"

#ifdef __cplusplus
extern "C" {
#endif

int NumberEdgeCaseGetCount(SElement *pElement)
{
    ASSERT_NULL(pElement);

    // 字符串数字按照s64等同
    if (pElement->para.type == ENUM_STRING)
    {
        return 64 * 2 * 2;
    }

    return pElement->inLen * 2 * 2;
}

char* NumberEdgeCaseGetValue(SElement *pElement, int pos)
{
    int len;
    ASSERT_NULL(pElement);

    if ((pElement->para.type == ENUM_STRING) || (pElement->para.type == ENUM_STRING_NUM))
    {
        SetElementInitoutBufEx(pElement, STRING_NUMBER_LEN);

        s64 temp;

        if (pos < 64 * 2)
        {
            temp = 0 + (s64)g_edgeCaseTable[pos];
        }
        else
        {
            temp = 0 - (s64)g_edgeCaseTable[pos - 64 * 2];
        }

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

    if (pos < pElement->inLen * 2)
    {
        // 有无符号分别对待
        if (pElement->para.type == ENUM_NUMBER_S)
        {
            if (pElement->inLen <= 8)
            {
                *((s8 *)pElement->para.value) = 0 + (s8)g_edgeCaseTable[pos];
            }
            else if (pElement->inLen <= 16)
            {
                *((s16 *)pElement->para.value) = 0 + (s16)g_edgeCaseTable[pos];
            }
            else if (pElement->inLen <= 32)
            {
                *((s32 *)pElement->para.value) = 0 + (s32)g_edgeCaseTable[pos];
            }
            else if (pElement->inLen <= 64)
            {
                *((s64 *)pElement->para.value) = 0 + (s64)g_edgeCaseTable[pos];
            }
        }
        else if (pElement->para.type == ENUM_NUMBER_U)
        {
            if (pElement->inLen <= 8)
            {
                *((u8 *)pElement->para.value) = 0 + (u8)g_edgeCaseTable[pos];
            }
            else if (pElement->inLen <= 16)
            {
                *((u16 *)pElement->para.value) = 0 + (u16)g_edgeCaseTable[pos];
            }
            else if (pElement->inLen <= 32)
            {
                *((u32 *)pElement->para.value) = 0 + (u32)g_edgeCaseTable[pos];
            }
            else if (pElement->inLen <= 64)
            {
                *((u64 *)pElement->para.value) = 0 + (u64)g_edgeCaseTable[pos];
            }
        }
    }
    else
    {
        pos = pos - pElement->inLen * 2;
        // 有无符号分别对待
        if (pElement->para.type == ENUM_NUMBER_S)
        {
            if (pElement->inLen <= 8)
            {
                *((s8 *)pElement->para.value) = 0 - (s8)g_edgeCaseTable[pos];
            }
            else if (pElement->inLen <= 16)
            {
                *((s16 *)pElement->para.value) = 0 - (s16)g_edgeCaseTable[pos];
            }
            else if (pElement->inLen <= 32)
            {
                *((s32 *)pElement->para.value) = 0 - (s32)g_edgeCaseTable[pos];
            }
            else if (pElement->inLen <= 64)
            {
                *((s64 *)pElement->para.value) = 0 - (s64)g_edgeCaseTable[pos];
            }
        }
        else if (pElement->para.type == ENUM_NUMBER_U)
        {
            if (pElement->inLen <= 8)
            {
                *((u8 *)pElement->para.value) = 0  - (u8)g_edgeCaseTable[pos];
            }
            else if (pElement->inLen <= 16)
            {
                *((u16 *)pElement->para.value) = 0 - (u16)g_edgeCaseTable[pos];
            }
            else if(pElement->inLen <= 32)
            {
                *((u32 *)pElement->para.value) = 0 - (u32)g_edgeCaseTable[pos];
            }
            else if (pElement->inLen <= 64)
            {
                *((u64 *)pElement->para.value) = 0 - (u64)g_edgeCaseTable[pos];
            }
        }
    }

    return pElement->para.value;
}

int NumberEdgeCaseGetIsSupport(SElement *pElement)
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

const struct MutaterGroup g_numberEdgeCaseGroup = {
    "NumberEdgeCase",
    NumberEdgeCaseGetCount,
    NumberEdgeCaseGetValue,
    NumberEdgeCaseGetIsSupport,
    1
};

void InitNumberEdgeCase(void)
{
    RegisterMutater(&g_numberEdgeCaseGroup, ENUM_NUMBER_EDGE_CASE);
}

#ifdef __cplusplus
}
#endif