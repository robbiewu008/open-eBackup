/*
版权所有 (c) 华为技术有限公司 2012-2018

原理:					变异字符串长度  ，
						变异算法为宽度临界值变异，
						使用字符串本身填充
						

长度:					最大值和1之间

数量:					最大输出长度的bit宽度

支持数据类型: 	有初始值的字符串类型
*/
#include "../common/PCommon.h"

#ifdef __cplusplus
extern "C" {
#endif

static int StringLengthEdgeCaseGetCount(SElement *pElement)
{
    u32 count;
    int inLen;
    ASSERT_NULL(pElement);

    inLen = (int)(pElement->inLen / 8);

    if (inLen <= 1)
    {
        return 0;
    }

    count = InGetBitNumber(pElement->para.maxLen);

    if (count == 0)
    {
        return 0;
    }
    
    return count  * 2 - 1;
}

static char* StringLengthEdgeCaseGetValue(SElement *pElement, int pos)
{
    int i;
    int outLen;
    int inLen;

    ASSERT_NULL(pElement);

    if (pos == 0)
    {
        pos = 1;
    }

    inLen = (int)(pElement->inLen / 8);
    outLen = g_edgeCaseTable[pos];

    SetElementInitoutBufEx(pElement, outLen);

    for (i = 0; i < outLen / (inLen - 1); i++)
    { 
        HwMemcpy(pElement->para.value + i * (inLen - 1), pElement->inBuf, (inLen - 1)); 
    }

    // 拷贝剩下的
    HwMemcpy(pElement->para.value + i * (inLen - 1), 
        pElement->inBuf, outLen - (outLen / (inLen - 1)) * (inLen - 1)); 

    ((char *)pElement->para.value)[outLen - 1] = 0; 
    return pElement->para.value;
}

static int StringLengthEdgeCaseGetIsSupport(SElement *pElement)
{
    ASSERT_NULL(pElement);
    // 有初始值的字符串
    if ((pElement->para.type == ENUM_STRING)
        && (pElement->isHasInitValue == ENUM_YES)
        && ((int)(pElement->inLen / 8) > 1))
    {
        return ENUM_YES;
    }

    return ENUM_NO;
}

const struct MutaterGroup g_stringLengthEdgeCaseGroup = {
    "StringLengthEdgeCase",
    StringLengthEdgeCaseGetCount,
    StringLengthEdgeCaseGetValue,
    StringLengthEdgeCaseGetIsSupport,
    1
};

void InitStringLengthEdgeCase(void)
{
    RegisterMutater(&g_stringLengthEdgeCaseGroup, ENUM_STRING_LENGTH_EDGE_CASE);
}

#ifdef __cplusplus
}
#endif