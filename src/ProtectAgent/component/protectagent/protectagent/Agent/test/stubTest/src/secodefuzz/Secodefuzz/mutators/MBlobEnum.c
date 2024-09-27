/*
版权所有 (c) 华为技术有限公司 2012-2018

原理:					Blob枚举变异
						

长度:							

数量:					枚举数量

支持数据类型: 	Blob枚举类型

*/
#include "../common/PCommon.h"

#ifdef __cplusplus
extern "C" {
#endif

int BlobEnumGetCount(SElement *pElement)
{
    ASSERT_NULL(pElement);
    return pElement->enumCount;
}

char* BlobEnumGetValue(SElement *pElement, int pos)
{
    int tempPos =pos % pElement->enumCount;
    int len = pElement->enumBloblTable[tempPos];
    SetElementInitoutBufEx(pElement, len);
    HwMemcpy(pElement->para.value, pElement->enumBlobTable[tempPos], len); 
    return pElement->para.value;
}

int BlobEnumGetIsSupport(SElement *pElement)
{
    if (pElement->para.type == ENUM_BLOB_ENUM)
    {
        return ENUM_YES;
    }

    return ENUM_NO;
}

const struct MutaterGroup g_blobEnumGroup = {
    .name           = "BlobEnum",
    .getCount       = BlobEnumGetCount,
    .getValue       = BlobEnumGetValue,
    .getIsSupport   = BlobEnumGetIsSupport,
};

void InitBlobEnum(void)
{
    RegisterMutater(&g_blobEnumGroup, ENUM_BLOB_ENUM_M);
}

#ifdef __cplusplus
}
#endif