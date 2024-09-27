/*
版权所有 (c) 华为技术有限公司 2012-2018

使用变异数据库来替换字符串
*/
#include "../common/PCommon.h"

#ifdef __cplusplus
extern "C" {
#endif

int StringStaticGetCount(SElement *pElement)
{
	return MAX_COUNT;
}

char* StringStaticGetValue(SElement *pElement, int pos)
{
    int outLen;
    ASSERT_NULL(pElement);

    pos = RAND_32() % g_stringStaticTableLen;
    outLen = Instrlen(g_stringStaticTable[pos]) + 1;
    if (outLen >= pElement->para.maxLen)
    {
        outLen = pElement->para.maxLen;
    }

    SetElementInitoutBufEx(pElement, outLen);

    HwMemcpy(pElement->para.value, g_stringStaticTable[pos], outLen);
    return pElement->para.value;
}

int StringStaticGetIsSupport(SElement *pElement)
{
    // 只要是字符串就支持
    if (pElement->para.type == ENUM_STRING)
    {
        return ENUM_YES;
    }

    return ENUM_NO;
}

const struct MutaterGroup g_stringStaticGroup = {
    "StringStatic",
    StringStaticGetCount,
    StringStaticGetValue,
    StringStaticGetIsSupport,
    1
};

void InitStringStatic(void)
{
    RegisterMutater(&g_stringStaticGroup, ENUM_STRING_STATIC);
}

#ifdef __cplusplus
}
#endif

