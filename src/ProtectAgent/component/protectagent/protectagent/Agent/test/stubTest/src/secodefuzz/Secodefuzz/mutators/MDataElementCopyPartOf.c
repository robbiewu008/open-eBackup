/*
版权所有 (c) 华为技术有限公司 2012-2018


原理:					将样本的一部分copy覆盖到另一部分

长度:					长度不变

数量:					MAX_COUNT

支持数据类型: 	blob ,FixBlob,String

*/
#include "../common/PCommon.h"

#ifdef __cplusplus
extern "C" {
#endif

int DataElementCopyPartOfGetCount(SElement *pElement)
{
    ASSERT_NULL(pElement);

    // 爬分支变异手段
    return MAX_COUNT;
}

size_t CopyPartOf(const uint8_t *from, size_t fromSize, uint8_t *to, size_t toSize) 
{
    size_t toBeg = RAND_RANGE(0, toSize - 1);
    size_t copySize = RAND_RANGE(0, toSize - toBeg - 1) + 1;
    
    copySize = MIN(copySize, fromSize);
    
    size_t fromBeg = RAND_RANGE(0, fromSize - copySize);
    HwMemMove(to + toBeg, from + fromBeg, copySize);
    return toSize;
}

char *DataElementCopyPartOfGetValue(SElement *pElement, int pos)
{
    int inLen;
    ASSERT_NULL(pElement);

    inLen = (int)(pElement->inLen / 8);

    SetElementInitoutBufEx(pElement, inLen);

    HwMemcpy(pElement->para.value, pElement->inBuf, inLen);

    CopyPartOf((uint8_t *)pElement->para.value, inLen, (uint8_t *)pElement->para.value, inLen);

    return pElement->para.value;
}

int DataElementCopyPartOfGetIsSupport(SElement *pElement)
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

const struct MutaterGroup g_dataElementCopyPartOfGroup = {
    "DataElementCopyPartOf",
    DataElementCopyPartOfGetCount,
    DataElementCopyPartOfGetValue,
    DataElementCopyPartOfGetIsSupport,
    1
};

void InitDataElementCopyPartOf(void)
{
    RegisterMutater(&g_dataElementCopyPartOfGroup, ENUM_DATAELEMENT_COPY_PART_OF);
}

#ifdef __cplusplus
}
#endif

