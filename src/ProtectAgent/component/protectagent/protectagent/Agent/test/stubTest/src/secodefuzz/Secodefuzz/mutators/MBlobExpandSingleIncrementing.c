/*
版权所有 (c) 华为技术有限公司 2012-2018

原理:					测试用例中Blob会被插入随机数量的连续的bytes。
						插入的位置采用纯随机			
						插入的连续byte的数量从1到255，使用高斯变异

						第一个被插入的值是取值范围从0x00到0xff的随机选取的值
						第二个被插入的值会在前一个值的基础上加1，
						第三个被插入的值会继续再加1。
						当0xff发生的时候，后面的值会回滚到0x00.
						此后的值会持续加1，直到所有的插入值都有值了为止。

长度:					原始长度到最大长度之间

数量:					MAX_COUNT/5

支持数据类型: 	有初始值的blob元素

*/
#include "../common/PCommon.h"

#ifdef __cplusplus
extern "C" {
#endif

static int BlobExpandSingleIncrementingGetCount(SElement *pElement)
{
    ASSERT_NULL(pElement);
    
    return MAX_COUNT / 50;
}

static char *BlobExpandSingleIncrementingGetValue(SElement *pElement, int pos)
{
    int i;
    int outLen, inLen;
    int start;
    int changeLen;
    char value;
    ASSERT_NULL(pElement);

    inLen = (int)(pElement->inLen / 8);

    start = RAND_RANGE(0, inLen);

    // 只插入1到255个随机byte
    changeLen = GaussRandU32(RAND_32() % 8);

    outLen = inLen + changeLen;

    if (outLen > pElement->para.maxLen)
    {
        outLen = pElement->para.maxLen;
        changeLen = pElement->para.maxLen - inLen;
    }

    SetElementInitoutBufEx(pElement, outLen);

    HwMemcpy(pElement->para.value, pElement->inBuf, start);

    value = RAND_BYTE();
    for (i = start; i < start + changeLen; i++)
    {
        pElement->para.value[i] = value;
        value++;
    }

    HwMemcpy(pElement->para.value + start + changeLen, pElement->inBuf + start, inLen - start);

    return pElement->para.value;
}

static int BlobExpandSingleIncrementingGetIsSupport(SElement *pElement)
{
    // 只要是字符串就支持
    if ((pElement->para.type == ENUM_BLOB)
        || (pElement->para.type == ENUM_FIXBLOB))
    {
        return ENUM_YES;
    }

    return ENUM_NO;
}

const struct MutaterGroup g_blobExpandSingleIncrementingGroup = {
    "BlobExpandSingleIncrementing",
    BlobExpandSingleIncrementingGetCount,
    BlobExpandSingleIncrementingGetValue,
    BlobExpandSingleIncrementingGetIsSupport,
    1
};

void InitBlobExpandSingleIncrementing(void)
{
    RegisterMutater(&g_blobExpandSingleIncrementingGroup, ENUM_BLOB_EXPAND_SINGLE_INCREMENTING);
}

#ifdef __cplusplus
}
#endif