/*
版权所有 (c) 华为技术有限公司 2012-2018


原理:					每次变异n个byte,使用随机ascii的值
						变异几个byte使用高斯变异
						

长度:					长度不变		

数量:					bit数量乘4与MAXCOUNT的最小值

支持数据类型: 	有初始值的字符串类型

*/
#include "../common/PCommon.h"

#ifdef __cplusplus
extern "C" {
#endif

int StringAsciiRandomGetCount(SElement *pElement)
{
    ASSERT_NULL(pElement);
    
    // 目前设置这个变异算法数量为bit数乘4与500的最小值
    return MIN(pElement->inLen * 4, MAX_COUNT);
}

char* StringAsciiRandomGetValue(SElement *pElement, int pos)
{
    int i;
    int inLen;

    inLen = (int)(pElement->inLen / 8);

    SetElementInitoutBufEx(pElement, inLen);

    // 计算这次变异变几个字母
    int number1=GaussRandU32(pos % InGetBitNumber(inLen));

    HwMemcpy(pElement->para.value, pElement->inBuf, inLen);

    for (i = 0; i < inLen; i++)
    {
        // 计算这个字母是否需要变异
        if(RAND_RANGE(1, inLen) <= number1)
        {
            ((char *)pElement->para.value)[i]  = (char)RAND_RANGE(1, 127);
        }
    }

    ((char *)pElement->para.value)[inLen - 1] = 0;

    return pElement->para.value;
}

int StringAsciiRandomGetIsSupport(SElement *pElement)
{
    // 有初始值的字符串
    if ((pElement->para.type == ENUM_STRING)
        && (pElement->isHasInitValue == ENUM_YES))
    {
        return ENUM_YES;
    }

    return ENUM_NO;
}

const struct MutaterGroup g_stringAsciiRandomGroup = {
    .name           = "StringAsciiRandom",
    .getCount       = StringAsciiRandomGetCount,
    .getValue       = StringAsciiRandomGetValue,
    .getIsSupport   = StringAsciiRandomGetIsSupport,
};

void InitStringAsciiRandom(void)
{
    RegisterMutater(&g_stringAsciiRandomGroup, ENUM_STRING_ASCII_RANDOM);
}

#ifdef __cplusplus
}
#endif

