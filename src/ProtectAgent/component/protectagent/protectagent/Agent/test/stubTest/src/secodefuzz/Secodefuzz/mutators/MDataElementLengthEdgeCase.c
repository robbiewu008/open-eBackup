/*
版权所有 (c) 华为技术有限公司 2012-2018

原理:					变异数据长度  ，
						变异算法为宽度临界值变异，
						使用数据本身填充


长度:					长度在最大值和1之间

测试例数量:		测试例数量为最大输出长度的bit宽度

支持数据类型: 	有初始值的可变数据
*/
#include "../common/PCommon.h"

#ifdef __cplusplus
extern "C" {
#endif

static int DataElementLengthEdgeCaseGetCount(SElement *pElement)
{
    u32 count;

    ASSERT_NULL(pElement);
    
    count = InGetBitNumber(pElement->para.maxLen);

    if (count == 0)
    {
        return 0;
    }

    return count * 2 - 1;
}

static char* DataElementLengthEdgeCaseGetValue(SElement *pElement, int pos)
{
    int i;
    int inLen;
    int outLen;
    ASSERT_NULL(pElement);
    
    inLen = (int)(pElement->inLen / 8);
    outLen = g_edgeCaseTable[pos];

    if ((outLen > pElement->para.maxLen) || (outLen == 0))
    {
        return SetElementOriginalValue(pElement);
    }

    SetElementInitoutBufEx(pElement, outLen);

    for (i = 0; i < outLen / inLen; i++)
    { 
        HwMemcpy(pElement->para.value + i * inLen, pElement->inBuf, inLen); 
    }

    // 拷贝剩下的
    HwMemcpy(pElement->para.value + i * inLen, pElement->inBuf, outLen - (outLen / inLen) * inLen); 

    return pElement->para.value;
}

static int DataElementLengthEdgeCaseGetIsSupport(SElement *pElement)
{
    ASSERT_NULL(pElement);
    // 有初始值的可变数据
    if (((pElement->para.type == ENUM_STRING)
        || (pElement->para.type == ENUM_BLOB))
        && (pElement->isHasInitValue == ENUM_YES))
    {
        return ENUM_YES;
    }

    return ENUM_NO;
}

const struct MutaterGroup g_dataElementLengthEdgeCaseGroup = {
    "DataElementLengthEdgeCase",
    DataElementLengthEdgeCaseGetCount,
    DataElementLengthEdgeCaseGetValue,
    DataElementLengthEdgeCaseGetIsSupport,
    1
};

void InitDataElementLengthEdgeCase(void)
{
    RegisterMutater(&g_dataElementLengthEdgeCaseGroup, ENUM_DATAELEMENT_LENGTH_EDGE_CASE);
}

#ifdef __cplusplus
}
#endif