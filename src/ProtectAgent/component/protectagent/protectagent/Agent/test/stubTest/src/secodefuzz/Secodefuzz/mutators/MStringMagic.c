/*
版权所有 (c) 华为技术有限公司 2012-2018


原理:					利用hook来源数据插入或覆盖

长度:					

数量:					MAX_COUNT*2

支持数据类型: 	ENUM_STRING
*/
#include "../common/PCommon.h"

#ifdef __cplusplus
extern "C" {
#endif

static int StringMagicGetCount(SElement *pElement)
{
    ASSERT_NULL(pElement);

    if (LlvmDataMemGetCount() == 0)
    {
        return 0;
    }

    return MAX_COUNT * 2;
}

char* StringMagicGetValue(SElement *pElement, int pos)
{
    int llvmLen;
    char * value =NULL;

    ASSERT_NULL(pElement);

    // \0增加的逻辑在函数里边
    value = LlvmDataMemGetValue(&llvmLen);

    //库没有值，则空
    if (llvmLen == 0)
    {
        return SetElementOriginalValue(pElement);
    }

    int isInsert = RAND_32() % 3;

    // 0 Insert 1 Overwrite 2 replace
    MagicGetValue(pElement, value, llvmLen, isInsert);

    return pElement->para.value;
}

static int StringMagicGetIsSupport(SElement *pElement)
{
    ASSERT_NULL(pElement);
    // 字符串
    if (pElement->para.type == ENUM_STRING)
    {
        return ENUM_YES;
    }

    return ENUM_NO;
}

const struct MutaterGroup g_stringMagicGroup = {
    .name 			= "StringMagic",
    .getCount 		= StringMagicGetCount,
    .getValue 		= StringMagicGetValue,
    .getIsSupport 	= StringMagicGetIsSupport,
};

void InitStringMagic(void)
{	
    if (LlvmHookIsSupport() == 0)
    {
        return;
    }

    RegisterMutater(&g_stringMagicGroup, ENUM_STRING_MAGIC);
}

#ifdef __cplusplus
}
#endif

