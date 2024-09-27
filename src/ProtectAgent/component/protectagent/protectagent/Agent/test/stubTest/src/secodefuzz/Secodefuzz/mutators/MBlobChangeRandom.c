/*
版权所有 (c) 华为技术有限公司 2012-2018

原理:					测试用例中Blob的随机数量的连续的bytes会被改变。
						变异发生的位置和byte变化的数量一样都是随机决定的。
						使用随机byte填充

长度:					长度不变

数量:					bit值与MAXCOUNT的最小值

支持数据类型: 	有初始值的blob，FixBlob元素

*/
#include "../common/PCommon.h"

#ifdef __cplusplus
extern "C" {
#endif

int BlobChangeRandomGetCount(SElement *pElement)
{
    ASSERT_NULL(pElement);

    return MIN((int)pElement->inLen, MAX_COUNT);
}

char *BlobChangeRandomGetValue(SElement *pElement, int pos)
{
    int i;
    int inLen;
    int start;
    int changeLen;
    
    ASSERT_NULL(pElement);

    inLen = (int)(pElement->inLen / 8);

    SetElementInitoutBufEx(pElement, inLen);

    InGetRegion(inLen, &start, &changeLen);

    HwMemcpy(pElement->para.value, pElement->inBuf, start);

    for (i = start; i < start + changeLen; i++)
    {
        pElement->para.value[i] = RAND_BYTE();
    }

    HwMemcpy(pElement->para.value + start + changeLen, 
        pElement->inBuf + start + changeLen, inLen - start - changeLen);

    return pElement->para.value;
}

int BlobChangeRandomGetIsSupport(SElement *pElement)
{
    // 只要是字符串就支持
    if (((pElement->para.type == ENUM_BLOB) 
        || (pElement->para.type == ENUM_FIXBLOB)) 
        && (pElement->isHasInitValue == ENUM_YES))
    {
        return ENUM_YES;
    }

    return ENUM_NO;
}

const struct MutaterGroup g_blobChangeRandomGroup = {
    "BlobChangeRandom",
    BlobChangeRandomGetCount,
    BlobChangeRandomGetValue,
    BlobChangeRandomGetIsSupport,
    1
};

void InitBlobChangeRandom(void)
{
    RegisterMutater(&g_blobChangeRandomGroup, ENUM_BLOB_CHANGE_RANDOM);
}

#ifdef __cplusplus
}
#endif