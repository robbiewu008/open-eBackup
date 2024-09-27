/*
版权所有 (c) 华为技术有限公司 2012-2018


原理:					随机改变字符串中字符的大小写。
						每次改变多少个字母使用高斯变异
						

长度:					长度不变		

数量:					字母数量乘8与MAXCOUNT的最小值

支持数据类型: 	有初始值的字符串类型

*/
#include "../common/PCommon.h"

#ifdef __cplusplus
extern "C" {
#endif

int StringCaseRandomGetCount(SElement *pElement)
{
    ASSERT_NULL(pElement);
    
    // 目前设置这个变异算法数量为bit数与500的最小值
    return MIN(InGetLetterNumber((char*)pElement->inBuf) * 8, MAX_COUNT);
}

char* StringCaseRandomGetValue(SElement *pElement, int pos)
{
    int i;
    int inLen;
    char tmp1, tmp2;

    inLen = (int)(pElement->inLen / 8);

    SetElementInitoutBufEx(pElement, inLen);

    int number = InGetLetterNumber((char*)pElement->inBuf);

    // 计算这次变异变几个字母
    int number1 = GaussRandU32(pos % InGetBitNumber(number));

    for (i = 0; i < inLen; i++)
    {
        tmp1 = ((char*)pElement->inBuf)[i];

        if (InIsLetter(tmp1))
        {
            // 计算这个字母是否需要变异
            if (RAND_RANGE(1, number) <= number1)
            {
                tmp2 = (char)InToUpper(tmp1);
                if (tmp1 == tmp2)
                {
                    tmp2 = (char)InToLower(tmp1);
                }
                tmp1 = tmp2;
            }
        }
        ((char *)pElement->para.value)[i] = tmp1;
    }
    // 最后为0，无所谓

    return pElement->para.value;
}

int StringCaseRandomGetIsSupport(SElement *pElement)
{
    // 有初始值的字符串
    if ((pElement->para.type == ENUM_STRING)
        && (pElement->isHasInitValue == ENUM_YES))
    {
        return ENUM_YES;
    }

    return ENUM_NO;
}

const struct MutaterGroup g_stringCaseRandomGroup = {
    .name           = "StringCaseRandom",
    .getCount       = StringCaseRandomGetCount,
    .getValue       = StringCaseRandomGetValue,
    .getIsSupport   = StringCaseRandomGetIsSupport,
};

void InitStringCaseRandom(void)
{
    RegisterMutater(&g_stringCaseRandomGroup, ENUM_STRING_CASE_RANDOM);
}

#ifdef __cplusplus
}
#endif

