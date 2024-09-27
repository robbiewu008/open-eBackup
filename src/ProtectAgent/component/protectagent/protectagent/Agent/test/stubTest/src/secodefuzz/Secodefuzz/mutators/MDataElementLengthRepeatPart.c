/*
版权所有 (c) 华为技术有限公司 2012-2018

原理:					随机复制某部分数据，
						复制个数随机,复制1次的个数为25%
						

长度:					最大值和原长度之间变异，

数量:					MAX_COUNT

支持数据类型: 	有初始值的可变数据
*/
#include "../common/PCommon.h"

#ifdef __cplusplus
extern "C" {
#endif

static int DataElementLengthRepeatPartGetCount(SElement *pElement)
{
    ASSERT_NULL(pElement);
    return MAX_COUNT/10;
}

static char* DataElementLengthRepeatPartGetValue(SElement *pElement, int pos)
{
    int i;
    int inLen;
    int repeatLen;
    int count;
    int start;
    ASSERT_NULL(pElement);

    inLen = (int)(pElement->inLen / 8);

    // 得到要复制的byte个数
    repeatLen = GaussRandU32(RAND_32() % InGetBitNumber(inLen));

    // 减少复制1byte的个数，因为意义比较小，被随机的概率又太高
    if (repeatLen == 1)
    {
        repeatLen = GaussRandU32(RAND_32() % InGetBitNumber(inLen));
    }

    // 随机得到要插入的起始位置
    start = RAND_RANGE(0, inLen - repeatLen);

    // 随机得到要复制的个数
    count = GaussRandU32(RAND_32() % InGetBitNumber(pElement->para.maxLen / repeatLen)); 

    // 增加复制1次的个数为%25
    if (RAND_32() % 4 == 0)
    {
        count = 1;
    }

    if ((inLen + count * repeatLen) > pElement->para.maxLen)
    {
        return SetElementOriginalValue(pElement);
    }

    SetElementInitoutBufEx(pElement, (inLen + count * repeatLen));

    // 拷贝开始
    HwMemcpy(pElement->para.value , pElement->inBuf, start); 

    for (i = 0; i < count; i++)
    { 
        HwMemcpy(pElement->para.value + start + i * repeatLen, pElement->inBuf + start, repeatLen); 
    }

    // 拷贝剩下的
    HwMemcpy(pElement->para.value + start + i * repeatLen, pElement->inBuf + start, inLen - start); 

    return pElement->para.value;
}

static int DataElementLengthRepeatPartGetIsSupport(SElement *pElement)
{
    ASSERT_NULL(pElement);
    // 有初始值的可变数据
    if (((pElement->para.type == ENUM_STRING)
        || (pElement->para.type == ENUM_BLOB) 
        || (pElement->para.type == ENUM_FIXBLOB))
        &&(pElement->isHasInitValue == ENUM_YES))
    {
        return ENUM_YES;
    }

    return ENUM_NO;
}

const struct MutaterGroup g_dataElementLengthRepeatPartGroup = {
    .name           = "DataElementLengthRepeatPart",
    .getCount       = DataElementLengthRepeatPartGetCount,
    .getValue       = DataElementLengthRepeatPartGetValue,
    .getIsSupport   = DataElementLengthRepeatPartGetIsSupport,
};

void InitDataElementLengthRepeatPart(void)
{
    RegisterMutater(&g_dataElementLengthRepeatPartGroup, ENUM_DATAELEMENT_LENGTH_REPEAT_PART);
}

#ifdef __cplusplus
}
#endif