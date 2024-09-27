/*
版权所有 (c) 华为技术有限公司 2012-2018

原理:					测试用例中，Blob的随机数量的连续byte会被单独的改变，
						用小套的替换值(从测试输出上看，小套替换值基本上是"01"，"00"，"FF"，"FE")。
						变异发生的位置和byte变化的数量一样都是随机决定的。

长度:					长度不变

数量:					byte数量乘8与MAXCOUNT的最小值

支持数据类型: 	有初始值的blob，FixBlob元素

*/
#include "../common/PCommon.h"

#ifdef __cplusplus
extern "C" {
#endif

const static u8 special[4] = {0x00, 0x01, 0xfe, 0xff};

int BlobChangeSpecialGetCount(SElement *pElement)
{
    ASSERT_NULL(pElement);

    return MIN((int)pElement->inLen, MAX_COUNT);
}

char *BlobChangeSpecialGetValue(SElement *pElement, int pos)
{
    int i;
    int inLen;
    int count;
    int start, changeLen;

    ASSERT_NULL(pElement);

    inLen = (int)(pElement->inLen / 8);

    SetElementInitoutBufEx(pElement, inLen);

    InGetRegion(inLen, &start, &changeLen);

    HwMemcpy(pElement->para.value, pElement->inBuf, start);

    count = sizeof(special) - 1;
    for (i = start; i < start + changeLen; i++)
    {
        pElement->para.value[i] = special[RAND_RANGE(0, count)];
    }

    HwMemcpy(pElement->para.value + start + changeLen, 
        pElement->inBuf + start + changeLen, inLen - start - changeLen);

    return pElement->para.value;
}

int BlobChangeSpecialGetIsSupport(SElement *pElement)
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

const struct MutaterGroup g_blobChangeSpecialGroup = {
    "BlobChangeSpecial",
    BlobChangeSpecialGetCount,
    BlobChangeSpecialGetValue,
    BlobChangeSpecialGetIsSupport,
    1
};

void InitBlobChangeSpecial(void)
{
    RegisterMutater(&g_blobChangeSpecialGroup, ENUM_BLOB_CHANGE_SPECIAL);
}

#ifdef __cplusplus
}
#endif