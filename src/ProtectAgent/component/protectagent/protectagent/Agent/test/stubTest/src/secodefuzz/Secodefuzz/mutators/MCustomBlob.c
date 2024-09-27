/*
版权所有 (c) 华为技术有限公司 2012-2018

简单的定制化blob类型变异算法，
只要在两个表中填入数据，你的数据就会出现在blob的测试例里

*/
#include "../common/PCommon.h"

#ifdef __cplusplus
extern "C" {
#endif

int CustomBlobGetCount(SElement *pElement)
{
    ASSERT_NULL(pElement);
    int count = g_customBlobTableCount;

    if (count == 0)
    {
        return 0;
    }
        
    return MAX_COUNT / 2;
}

char* CustomBlobGetValue(SElement *pElement, int pos)
{
    int count = g_customBlobTableCount;
    int llvmLen;

    ASSERT_NULL(pElement);

    // 库没有值，不变异
    if (count == 0)
    {
        return SetElementOriginalValue(pElement);
    }

    // 库里随机抽取一个
    size_t idx = RAND_32() % count;

    llvmLen = g_customBlobTableLen[idx];

    // 插入，覆盖，还是完整替换
    int isInsert = RAND_32() % 3;

    // 0 Insert 1 Overwrite 2 replace
    MagicGetValue(pElement, (char*)g_customBlobTable[idx], llvmLen, isInsert);
    
    return pElement->para.value;
}

int CustomBlobGetIsSupport(SElement *pElement)
{
    ASSERT_NULL(pElement);
    // 只要是Blob就支持
    if ((pElement->para.type == ENUM_BLOB) 
        || (pElement->para.type == ENUM_FIXBLOB))
    {
        return ENUM_YES;
    }

    return ENUM_NO;
}

const struct MutaterGroup g_customBlobGroup = {
    "CustomBlob",
    CustomBlobGetCount,
    CustomBlobGetValue,
    CustomBlobGetIsSupport,
    1
};

void InitCustomBlob(void)
{
    RegisterMutater(&g_customBlobGroup, ENUM_CUSTOM_BLOB);
}

#ifdef __cplusplus
}
#endif