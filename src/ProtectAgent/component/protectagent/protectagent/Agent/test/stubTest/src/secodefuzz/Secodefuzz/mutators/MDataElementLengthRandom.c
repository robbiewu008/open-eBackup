/*
版权所有 (c) 华为技术有限公司 2012-2018

原理:					变异数据长度  ，
						变异算法随机
						使用数据本身填充
						

长度:					长度在最大值和1之间

数量:					最大输出长度开平方与MAXCOUNT/5的最小值

支持数据类型: 	有初始值的可变数据
*/
#include "../common/PCommon.h"

#ifdef __cplusplus
extern "C" {
#endif

static int DataElementLengthRandomGetCount(SElement *pElement)
{
    ASSERT_NULL(pElement);
    return MIN(Insqrt(pElement->para.maxLen), MAX_COUNT / 100);
}

static char* DataElementLengthRandomGetValue(SElement *pElement, int pos)
{
    int i;
    int outLen;
    int inLen;
    ASSERT_NULL(pElement);

    inLen = (int)(pElement->inLen / 8);
    outLen = RAND_RANGE(1, pElement->para.maxLen);

    SetElementInitoutBufEx(pElement, outLen);

    for (i = 0; i < outLen / inLen; i++)
    { 
        HwMemcpy(pElement->para.value + i * inLen, pElement->inBuf, inLen); 
    }

    // 拷贝剩下的
    HwMemcpy(pElement->para.value + i * inLen, pElement->inBuf, outLen - (outLen / inLen) * inLen); 

    return pElement->para.value;
}

static int DataElementLengthRandomGetIsSupport(SElement *pElement)
{
    ASSERT_NULL(pElement);
    // 有初始值的可变数据
    if (((pElement->para.type == ENUM_STRING)
        || (pElement->para.type == ENUM_BLOB))
        && (pElement->isHasInitValue == ENUM_YES))
    {
        return ENUM_YES;
    }

    return ENUM_NO;
}

const struct MutaterGroup g_dataElementLengthRandomGroup = {
    .name           = "DataElementLengthRandom",
    .getCount       = DataElementLengthRandomGetCount,
    .getValue       = DataElementLengthRandomGetValue,
    .getIsSupport   = DataElementLengthRandomGetIsSupport,
};

void InitDataElementLengthRandom(void)
{
    RegisterMutater(&g_dataElementLengthRandomGroup, ENUM_DATAELEMENT_LENGTH_RANDOM);
}

#ifdef __cplusplus
}
#endif