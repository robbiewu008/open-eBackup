/*
版权所有 (c) 华为技术有限公司 2012-2018


原理:					字符串枚举变异
						

长度:					!					

数量:					枚举数量

支持数据类型: 	字符串枚举类型

*/
#include "../common/PCommon.h"

#ifdef __cplusplus
extern "C" {
#endif

int StringEnumGetCount(SElement *pElement)
{
    ASSERT_NULL(pElement);
    return pElement->enumCount;
}

char* StringEnumGetValue(SElement *pElement, int pos)
{
    int tempPos = pos % pElement->enumCount;
    int len = Instrlen(pElement->enumStringTable[tempPos]) + 1;
    SetElementInitoutBufEx(pElement, len);
    HwMemcpy(pElement->para.value, pElement->enumStringTable[tempPos], len); 
    return pElement->para.value;
}

int StringEnumGetIsSupport(SElement *pElement)
{
    if (pElement->para.type == ENUM_STRING_ENUM)
    {
        return ENUM_YES;
    }

    return ENUM_NO;
}

const struct MutaterGroup g_stringEnumGroup = {
    .name           = "StringEnum",
    .getCount       = StringEnumGetCount,
    .getValue       = StringEnumGetValue,
    .getIsSupport   = StringEnumGetIsSupport,
};

void InitStringEnum(void)
{
    RegisterMutater(&g_stringEnumGroup, ENUM_STRING_ENUM_M);
}

#ifdef __cplusplus
}
#endif

