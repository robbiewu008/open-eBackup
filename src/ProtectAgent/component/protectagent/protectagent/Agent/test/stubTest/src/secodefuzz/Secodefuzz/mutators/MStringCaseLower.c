/*
版权所有 (c) 华为技术有限公司 2012-2018

原理:					所有大写字母变成小写,如果没有小写字母，则为原值
						

长度:					长度不变		

数量:					1

支持数据类型: 	有初始值的字符串类型

*/
#include "../common/PCommon.h"

#ifdef __cplusplus
extern "C" {
#endif

int StringCaseLowerGetCount(SElement *pElement)
{
    ASSERT_NULL(pElement);
    return 1;
}

char* StringCaseLowerGetValue(SElement *pElement, int pos)
{
    int i;
    int inLen = (int)(pElement->inLen / 8);

    SetElementInitoutBufEx(pElement, inLen);

    // 最后为0，无所谓
    for (i = 0; i < inLen; i++)
    {
        ((char *)pElement->para.value)[i] = InToLower(((char*)pElement->inBuf)[i]);
    }

    return pElement->para.value;
}

int StringCaseLowerGetIsSupport(SElement *pElement)
{
    // 有初始值的字符串
    if ((pElement->para.type == ENUM_STRING)
        && (pElement->isHasInitValue == ENUM_YES))
    {
        return ENUM_YES;
    }

    return ENUM_NO;
}

const struct MutaterGroup g_stringCaseLowerGroup = {
    .name           = "StringCaseLower",
    .getCount       = StringCaseLowerGetCount,
    .getValue       = StringCaseLowerGetValue,
    .getIsSupport   = StringCaseLowerGetIsSupport,
};

void InitStringCaseLower(void)
{
    RegisterMutater(&g_stringCaseLowerGroup, ENUM_STRING_CASE_LOWER);
}

#ifdef __cplusplus
}
#endif