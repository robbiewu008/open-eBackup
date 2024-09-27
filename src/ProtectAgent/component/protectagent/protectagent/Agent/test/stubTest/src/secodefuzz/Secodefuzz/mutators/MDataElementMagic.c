/*
版权所有 (c) 华为技术有限公司 2012-2018


原理:					利用hook来源数据替换

长度:					

数量:					MAX_COUNT*2

支持数据类型: 	可变长数据类型
*/
#include "../common/PCommon.h"

#ifdef __cplusplus
extern "C" {
#endif

int DataElementMagicGetCount(SElement *pElement)
{
    if (LlvmDataMemGetCount() == 0)
    {
        return 0;
    }
    
    return MAX_COUNT * 2;
}

char* DataElementMagicGetValue(SElement *pElement, int pos)
{
    int llvmLen;
    char* value =NULL;
    ASSERT_NULL(pElement);

    value = LlvmDataMemGetValue(&llvmLen);

    // 库没有值，则空
    if (llvmLen == 0)
    {
        return SetElementOriginalValue(pElement);
    }

    if (llvmLen > pElement->para.maxLen)
    {
        llvmLen = pElement->para.maxLen;
    }

    SetElementInitoutBufEx(pElement, llvmLen);
    HwMemcpy(pElement->para.value, value, llvmLen); 

    return (char *)pElement->para.value;
}

int DataElementMagicGetIsSupport(SElement *pElement)
{
    if ((pElement->para.type == ENUM_STRING)
        || (pElement->para.type == ENUM_BLOB)
        || (pElement->para.type == ENUM_FIXBLOB))
    {
        return ENUM_YES;
    }

    return ENUM_NO;
}

const struct MutaterGroup g_dataElementMagicGroup = {
    "DataElementMagic",
    DataElementMagicGetCount,
    DataElementMagicGetValue,
    DataElementMagicGetIsSupport,
    1
};

void InitDataElementMagic(void)
{
    if (LlvmHookIsSupport() == 0)
    {
        return;
    }
    
    RegisterMutater(&g_dataElementMagicGroup, ENUM_DATAELEMENT_MAGIC);
}

#ifdef __cplusplus
}
#endif

