/*
版权所有 (c) 华为技术有限公司 2012-2018


原理:					变异字符串长度  ，
						变异算法为高斯变异，
						使用随机的ascii来扩充字符串，
						偶数测试例在原有字符串上扩充，
						奇数测试例不使用原有字符串						

长度:					长度在最大值和1之间				

数量:					最大输出长度开平方与MAXCOUNT的最小值

支持数据类型: 	字符串类型

*/
#include "../common/PCommon.h"

#ifdef __cplusplus
extern "C" {
#endif

int StringLengthAsciiRandomGetCount(SElement *pElement)
{
    ASSERT_NULL(pElement);

    return MIN(Insqrt(pElement->para.maxLen), MAX_COUNT/100);
}

char* StringLengthAsciiRandomGetValue(SElement *pElement, int pos)
{
    int i;
    int start, changeLen, outLen;
    int inLen = 0;

    changeLen = GaussRandU32(pos % InGetBitNumber(pElement->para.maxLen));

    // 偶数测试例在源字符串上追加
    // 奇数测试例不使用原始字符串
    if (pos % 2 == 0)
    {
        if (pElement->isHasInitValue == ENUM_YES)
        {
            inLen = (int)(pElement->inLen / 8);
        }
        else 
        {
         inLen = 0;
        }
    }
    outLen = inLen + changeLen;

    if (outLen > pElement->para.maxLen)
    {
        outLen = pElement->para.maxLen;
    }

    SetElementInitoutBufEx(pElement, outLen);

    if (inLen > 0)
    {
        HwMemcpy(pElement->para.value, pElement->inBuf, inLen);
        start = inLen -1;
    }
    else
    {
        start = 0;
    }

    for (i = start; i < outLen - 1; i++)
    {
        ((char *)pElement->para.value)[i] = (char)RAND_RANGE(0x20, 127);
    }

    ((char *)pElement->para.value)[outLen - 1] = 0;

    return pElement->para.value;
}

int StringLengthAsciiRandomGetIsSupport(SElement *pElement)
{
    // 本测试例不需要有初始值
    if (pElement->para.type == ENUM_STRING)
    {
        return ENUM_YES;
    }

    return ENUM_NO;
}

const struct MutaterGroup g_stringLengthAsciiRandomGroup = {
    .name 			= "StringLengthAsciiRandom",
    .getCount 		= StringLengthAsciiRandomGetCount,
    .getValue 		= StringLengthAsciiRandomGetValue,
    .getIsSupport 	= StringLengthAsciiRandomGetIsSupport,
};

void InitStringLengthAsciiRandom(void)
{
    RegisterMutater(&g_stringLengthAsciiRandomGroup, ENUM_STRING_LENGTH_ASCII_RANDOM);
}

#ifdef __cplusplus
}
#endif

