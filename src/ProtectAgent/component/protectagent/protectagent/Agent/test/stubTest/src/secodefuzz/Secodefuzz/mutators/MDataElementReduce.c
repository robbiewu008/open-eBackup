/*
版权所有 (c) 华为技术有限公司 2012-2018


原理:					原始数据的随机数量的连续的byte会被删除.

长度:					0到原始长度之间

数量:					byte数量乘8与MAXCOUNT的最小值

支持数据类型: 	有初始值的可变数据

*/
#include "../common/PCommon.h"

#ifdef __cplusplus
extern "C" {
#endif

int DataElementReduceGetCount(SElement *pElement)
{
    ASSERT_NULL(pElement);

    return MAX_COUNT/5;
}

char *DataElementReduceGetValue(SElement *pElement, int pos)
{
    int len;
    int start;
    int deleteLen;

    ASSERT_NULL(pElement);

    len = (int)(pElement->inLen / 8);
    
    // 得到要删除的byte个数
    deleteLen = GaussRandU32(pos % InGetBitNumber(len));
    if (len == deleteLen)
    {
        return SetElementOriginalValue(pElement);
    }

    // 随机得到要删除的位置
    start = RAND_RANGE(0, len - deleteLen);

    SetElementInitoutBufEx(pElement, len - deleteLen);

    HwMemcpy(pElement->para.value, pElement->inBuf, start);
    HwMemcpy(pElement->para.value + start, pElement->inBuf + deleteLen + start, len - deleteLen - start);

    return pElement->para.value;
}

int DataElementReduceGetIsSupport(SElement *pElement)
{
    ASSERT_NULL(pElement);
    // 有初始值的可变数据
    if (((pElement->para.type == ENUM_STRING)
        || (pElement->para.type == ENUM_BLOB)
        || (pElement->para.type == ENUM_FIXBLOB))
        && (pElement->isHasInitValue == ENUM_YES))
    {
        return ENUM_YES;
    }

    return ENUM_NO;
}

const struct MutaterGroup g_dataElementReduceGroup = {
    "DataElementReduce",
    DataElementReduceGetCount,
    DataElementReduceGetValue,
    DataElementReduceGetIsSupport,
    1
};

void InitDataElementReduce(void)
{
    RegisterMutater(&g_dataElementReduceGroup, ENUM_DATAELEMENT_REDUCE);
}

#ifdef __cplusplus
}
#endif

