/*
版权所有 (c) 华为技术有限公司 2012-2018


原理:					原始数据的随机位置被插入随机byte

长度:					原始长度+1

数量:					MAX_COUNT

支持数据类型: 	可变长数据类型

*/
#include "../common/PCommon.h"

#ifdef __cplusplus
extern "C" {
#endif

int DataElementOneByteInsertGetCount(SElement *pElement)
{
    ASSERT_NULL(pElement);

    // 爬分支变异手段，因此变异数量有所倾向
    return MAX_COUNT*2;
}

char *DataElementOneByteInsertGetValue(SElement *pElement, int pos)
{
    int inLen;
    int start;
    ASSERT_NULL(pElement);

    inLen = (int)(pElement->inLen / 8);

    // 最大长度算法要所有算法必须遵守
    if ((inLen + 1) > pElement->para.maxLen)
    {
        return SetElementOriginalValue(pElement);
    }

    // 本算法支持无初始值元素

    start = RAND_RANGE(0, inLen);

    SetElementInitoutBufEx(pElement, inLen + 1);

    HwMemcpy(pElement->para.value, pElement->inBuf, start);

    //一半机会给专有byte，发现问题的概率更大一些
    if (RAND_BOOL())
    {
        pElement->para.value[start] = RAND_BYTE();
    }
    else
    {
        pElement->para.value[start] = g_specialByte[RAND_RANGE(0, g_specialByteLen - 1)];
    }
    HwMemcpy(pElement->para.value + start + 1, pElement->inBuf + start, inLen - start);

    return pElement->para.value;
}

int DataElementOneByteInsertGetIsSupport(SElement *pElement)
{
    ASSERT_NULL(pElement);
    // 可变数据
    if ((pElement->para.type == ENUM_STRING) 
        || (pElement->para.type == ENUM_BLOB) 
        || (pElement->para.type == ENUM_FIXBLOB))
    {
        return ENUM_YES;
    }

    return ENUM_NO;
}

const struct MutaterGroup g_dataElementOneByteInsertGroup = {
    "DataElementOneByteInsert",
    DataElementOneByteInsertGetCount,
    DataElementOneByteInsertGetValue,
    DataElementOneByteInsertGetIsSupport,
    1
};

void InitDataElementOneByteInsert(void)
{
    RegisterMutater(&g_dataElementOneByteInsertGroup, ENUM_DATAELEMENT_ONE_BYTE_INSERT);
}

#ifdef __cplusplus
}
#endif

