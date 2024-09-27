/*
版权所有 (c) 华为技术有限公司 2012-2018

原理:					测试用例中Blob会被插入随机数量的连续的bytes。
						插入的位置采用纯随机			
						插入的连续byte的数量从1到255，使用高斯变异
						插入的每个byte的值都是不同的随机选择的值。

长度:					原始长度到最大长度之间

数量:					MAX_COUNT/5

支持数据类型: 	有初始值的blob元素

*/
#include "../common/PCommon.h"

#ifdef __cplusplus
extern "C" {
#endif

static int BlobExpandAllRandomGetCount(SElement *pElement)
{
    ASSERT_NULL(pElement);

    return MAX_COUNT / 50;
}

static char *BlobExpandAllRandomGetValue(SElement *pElement, int pos)
{
    int i;
    int outLen, inLen;
    int start;
    int changeLen;
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

    for (i = start; i < start + changeLen; i++)
    {
        pElement->para.value[i] = RAND_BYTE();
    }

    HwMemcpy(pElement->para.value + start + changeLen, pElement->inBuf + start, inLen - start);

    return pElement->para.value;
}

static int BlobExpandAllRandomGetIsSupport(SElement *pElement)
{
    // 只要是字符串就支持
    if ((pElement->para.type == ENUM_BLOB)
        || (pElement->para.type == ENUM_FIXBLOB))
    {
        return ENUM_YES;
    }

    return ENUM_NO;
}

const struct MutaterGroup g_blobExpandAllRandomGroup = {
    "BlobExpandAllRandom",
    BlobExpandAllRandomGetCount,
    BlobExpandAllRandomGetValue,
    BlobExpandAllRandomGetIsSupport,
    1
};

void InitBlobExpandAllRandom(void)
{
    RegisterMutater(&g_blobExpandAllRandomGroup, ENUM_BLOB_EXPAND_ALL_RANDOM);
}

#ifdef __cplusplus
}
#endif