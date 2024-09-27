/*
版权所有 (c) 华为技术有限公司 2012-2018


原理:					随机交换相邻两部分数据
						随机交换不相邻两部分数据
						

长度:					长度不变

数量:					MAX_COUNT

支持数据类型: 	有初始值的数据类型,ENUM_STRING,ENUM_BLOB,ENUM_FIXBLOB
*/
#include "../common/PCommon.h"

#ifdef __cplusplus
extern "C" {
#endif

static int DataElementSwapTwoPartGetCount(SElement *pElement)
{
    ASSERT_NULL(pElement);
    return MAX_COUNT;
}

// 交换不相邻两部分
static char* DataElementSwapNoNearTwoPartGetValue(SElement *pElement, int pos)
{
    int inLen;
    int partLen1;
    int partLen2;
    int start1;
    int start2;
    ASSERT_NULL(pElement);

    inLen = (int)(pElement->inLen / 8);

    // 得到第一部分长度
    partLen1 = GaussRandU32(RAND_32() % InGetBitNumber(inLen));

    // 得到第二部分长度
    partLen2 = GaussRandU32(RAND_32() % InGetBitNumber(inLen));

    if ((partLen1 + partLen2) > inLen)
    {
        return SetElementOriginalValue(pElement);
    }

    // 随机第一部分的起始位置
    start1 = RAND_RANGE(0, inLen  - (partLen1 + partLen2));

    // 随机第二部分的起始位置
    start2 = RAND_RANGE(0, inLen  - (partLen1 + partLen2 + start1)) + start1 + partLen1;
    

    SetElementInitoutBufEx(pElement, inLen);

    // 拷贝开始
    HwMemcpy(pElement->para.value, pElement->inBuf, start1); 

    // 拷贝第二部分
    HwMemcpy(pElement->para.value + start1, pElement->inBuf + start2, partLen2); 

    // 拷贝中间
    HwMemcpy(pElement->para.value + start1 + partLen2, 
        pElement->inBuf + start1 + partLen1, start2 - start1 - partLen1); 

    // 拷贝第一部分
    HwMemcpy(pElement->para.value + start2 + partLen2 - partLen1, pElement->inBuf + start1, partLen1); 

    // 拷贝剩下的
    HwMemcpy(pElement->para.value + start2 + partLen2, 
        pElement->inBuf + start2 + partLen2, inLen - (start2 + partLen2)); 

    return pElement->para.value;
}

// 交换相邻两部分
static char* DataElementSwapNearTwoPartGetValue(SElement *pElement, int pos)
{
    int inLen;
    int partLen1;
    int partLen2;
    int start;
    ASSERT_NULL(pElement);

    inLen = (int)(pElement->inLen / 8);

    // 得到第一部分长度
    partLen1 = GaussRandU32(RAND_32() % InGetBitNumber(inLen));

    // 得到第二部分长度
    partLen2 = GaussRandU32(RAND_32() % InGetBitNumber(inLen));

    if ((partLen1 + partLen2) > inLen)
    {
        return SetElementOriginalValue(pElement);
    }

    // 随机得到要交换的起始位置
    start = RAND_RANGE(0, inLen  - (partLen1 + partLen2));

    SetElementInitoutBufEx(pElement, inLen);

    // 拷贝开始
    HwMemcpy(pElement->para.value, pElement->inBuf, start); 

    // 拷贝第二部分
    HwMemcpy(pElement->para.value + start, pElement->inBuf + start + partLen1, partLen2); 

    // 拷贝第一部分
    HwMemcpy(pElement->para.value + start + partLen2, pElement->inBuf + start, partLen1); 

    // 拷贝剩下的
    HwMemcpy(pElement->para.value + start + partLen1 + partLen2, 
        pElement->inBuf + start + partLen1 + partLen2, inLen - (start + partLen1 + partLen2)); 

    return pElement->para.value;
}

static char* DataElementSwapTwoPartGetValue(SElement *pElement, int pos)
{
    if (RAND_32() % 2 == 0)
    {
        return DataElementSwapNearTwoPartGetValue(pElement, pos);
    }
    else
    {
        return DataElementSwapNoNearTwoPartGetValue(pElement, pos);
    }
}

static int DataElementSwapTwoPartGetIsSupport(SElement *pElement)
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

const struct MutaterGroup g_dataElementSwapTwoPartGroup = {
    .name           = "DataElementSwapTwoPart",
    .getCount       = DataElementSwapTwoPartGetCount,
    .getValue       = DataElementSwapTwoPartGetValue,
    .getIsSupport   = DataElementSwapTwoPartGetIsSupport,
};

void InitDataElementSwapTwoPart(void)
{
    RegisterMutater(&g_dataElementSwapTwoPartGroup, ENUM_DATAELEMENT_SWAP_TWO_PART);
}

#ifdef __cplusplus
}
#endif

