/*
版权所有 (c) 华为技术有限公司 2012-2018


使用变异数据库来替换字符串
之后在插入1-6个bom
*/
#include "../common/PCommon.h"

#ifdef __cplusplus
extern "C" {
#endif

int StringUtf8BomStaticGetCount(SElement *pElement)
{
    ASSERT_NULL(pElement);

    // 次数少点吧，这东西没啥大用
    return (MAX_COUNT / 100);
}

char* StringUtf8BomStaticGetValue(SElement *pElement, int pos)
{
    int i;
    int j;
    int outLen;
    ASSERT_NULL(pElement);

    // 增加几个bom
    int bomNum = RAND_RANGE(1, 6);

    pos = RAND_32() % g_stringStaticTableLen;
    outLen = Instrlen(g_stringStaticTable[pos]) + 1;
    if (outLen >= pElement->para.maxLen)
    {
        outLen = pElement->para.maxLen;
    }

    if (((outLen + bomNum*g_bomLen) > pElement->para.maxLen) || (outLen == 0))
    {
        return SetElementOriginalValue(pElement);
    }

    SetElementInitoutBufEx(pElement, outLen + bomNum * g_bomLen);
    HwMemcpy(pElement->para.value, g_stringStaticTable[pos], outLen);

    for (i = 0; i < bomNum; i++)
    { 
        // 计算插入bom的位置
        int pos1=RAND_RANGE(0, outLen);
        // 先copy后边，在copy前边

        for (j = outLen; j > pos1; j--)
        {
            pElement->para.value[j - 1 + g_bomLen ] = pElement->para.value[j - 1];
        }

        HwMemcpy(pElement->para.value + pos1, g_bom, g_bomLen); 

        outLen = outLen + g_bomLen;
    }

    ((char *)pElement->para.value)[outLen - 1] = 0; 
    return pElement->para.value;
}

int StringUtf8BomStaticGetIsSupport(SElement *pElement)
{
    // 本测试例不需要有初始值
    if (pElement->para.type == ENUM_STRING)
    {
        return ENUM_YES;
    }

    return ENUM_NO;
}

const struct MutaterGroup g_stringUtf8BomStaticGroup = {
    .name 		= "StringUtf8BomStatic",
    .getCount 		= StringUtf8BomStaticGetCount,
    .getValue 		= StringUtf8BomStaticGetValue,
    .getIsSupport 	= StringUtf8BomStaticGetIsSupport,
};

void InitStringUtf8BomStatic(void)
{
    RegisterMutater(&g_stringUtf8BomStaticGroup, ENUM_STRING_UTF8_BOM_STATIC);
}

#ifdef __cplusplus
}
#endif

