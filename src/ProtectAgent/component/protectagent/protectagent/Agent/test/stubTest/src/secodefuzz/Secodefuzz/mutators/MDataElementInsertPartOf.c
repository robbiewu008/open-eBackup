/*
版权所有 (c) 华为技术有限公司 2012-2018

原理:					将样本的一部分随机插入到样本中

长度:					0到最大长度之间

数量:					MAX_COUNT

支持数据类型: 	blob ,FixBlob,String

*/
#include "../common/PCommon.h"

#ifdef __cplusplus
extern "C" {
#endif

int DataElementInsertPartOfGetCount(SElement *pElement)
{
    ASSERT_NULL(pElement);

    // 爬分支变异手段
    return MAX_COUNT;
}

// Inserts part of From[0,ToSize) into To.
// Returns new size of To on success or 0 on failure.
size_t InsertPartOf(const uint8_t *from, size_t fromSize, uint8_t *to, size_t toSize, size_t maxToSize) 
{
    size_t availableSpace;
    size_t maxCopySize;
    size_t copySize;
    size_t fromBeg;
    size_t toInsertPos;
    size_t tailSize;

    if (toSize >= maxToSize) 
    {
        return 0;
    }

    availableSpace = maxToSize - toSize;
    maxCopySize = MIN(availableSpace, fromSize);
    copySize = RAND_RANGE(0, maxCopySize - 1) + 1;
    fromBeg = RAND_RANGE(0, fromSize - copySize);
    toInsertPos = RAND_RANGE(0, toSize);

    tailSize = toSize - toInsertPos;
    if (to == from) 
    {
        char* temp = HwMalloc(maxToSize);
        HwMemcpy(temp, from + fromBeg, copySize);
        HwMemMove(to + toInsertPos + copySize, to + toInsertPos, tailSize);
        HwMemMove(to + toInsertPos, temp, copySize);
        HwFree(temp);
    } 
    else 
    {
        HwMemMove(to + toInsertPos + copySize, to + toInsertPos, tailSize);
        HwMemMove(to + toInsertPos, from + fromBeg, copySize);
    }
    return toSize + copySize;
}

char *DataElementInsertPartOfGetValue(SElement *pElement, int pos)
{
     int inLen;
    ASSERT_NULL(pElement);

    inLen = (int)(pElement->inLen / 8);

    SetElementInitoutBufEx(pElement, pElement->para.maxLen);

    HwMemcpy(pElement->para.value, pElement->inBuf, inLen);

    pElement->para.len = InsertPartOf(
        (uint8_t *)pElement->para.value, inLen, (uint8_t *)pElement->para.value, inLen, pElement->para.maxLen);

    return pElement->para.value;
}

int DataElementInsertPartOfGetIsSupport(SElement *pElement)
{
    ASSERT_NULL(pElement);
    // 目前仅支持blob,增强buf变异
    if (((pElement->para.type == ENUM_BLOB) 
        || (pElement->para.type == ENUM_FIXBLOB)
        || (pElement->para.type == ENUM_STRING)) 
        && (pElement->isHasInitValue == ENUM_YES))
    {
        return ENUM_YES;
    }

    return ENUM_NO;
}

const struct MutaterGroup g_dataElementInsertPartOfGroup = {
    "DataElementInsertPartOf",
    DataElementInsertPartOfGetCount,
    DataElementInsertPartOfGetValue,
    DataElementInsertPartOfGetIsSupport,
    1
};

void InitDataElementInsertPartOf(void)
{
    RegisterMutater(&g_dataElementInsertPartOfGroup, ENUM_DATAELEMENT_INSERT_PART_OF);
}

#ifdef __cplusplus
}
#endif