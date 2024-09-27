/*
版权所有 (c) 华为技术有限公司 2012-2018

原理:					利用hook来源数据插入或覆盖

长度:					

数量:					MAX_COUNT*4

支持数据类型: 	eBlob,FixBlob
*/
#include "../common/PCommon.h"

#ifdef __cplusplus
extern "C" {
#endif
static int BlobMagicGetCount(SElement *pElement)
{
    ASSERT_NULL(pElement);

    if ((LlvmDataMemGetCount() == 0) && (LlvmDataNumberGetCount() == 0))
    {
        return 0;
    }
    
    return MAX_COUNT * 4;
}

static char *MemGetValue(SElement *pElement, int pos)
{
    int llvmLen;
    char* value =NULL;

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
static char *NumberGetValue(SElement *pElement , int pos)
{
    int llvmLen;

    ASSERT_NULL(pElement);

    u64 temp =LlvmDataNumberGetValue();

    // 把值获取出来，放到对应的内存里
    int size = RAND_32() % 3;
    char* copy = NULL;
    llvmLen = 0;

    u16 c16;
    u32 c32;
    u64 c64;

    if (size == 0)
    {
        c16 = temp;

        if (RAND_32() % 10 == 0)
        {
            c16 = InBswap16(c16);
        }

        if (RAND_32() % 10 == 1)
        {
            c16 = c16 + RAND_RANGE(-50, 50);
        }

        copy = (char *)&c16;
        llvmLen = 2;
    }
    else if (size == 1)
    {
        c32 = temp;

        if (RAND_32() % 10 == 0)
        {
            c32 = InBswap32(c32);
        }

        if (RAND_32() % 10 == 1)
        {
            c32 = c32 + RAND_RANGE(-50, 50);
        }

        copy = (char *)&c32;
        llvmLen = 4;
    }
    else if (size == 2)
    {
        c64 = temp;

        if (RAND_32() % 10 == 0)
        {
            c64 = InBswap64(c64);
        }

        if (RAND_32() % 10 == 1)
        {
            c64 = c64 + RAND_RANGE(-50, 50);
        }
        
        copy = (char *)&c64;
        llvmLen = 8;
    }

    int isInsert = RAND_32() % 3;

    // 0 Insert 1 Overwrite 2 replace
    MagicGetValue(pElement, copy, llvmLen, isInsert);

    return pElement->para.value;
}

static char *BlobMagicGetValue(SElement *pElement, int pos)
{
    int temp = RAND_32() % 20;
    if (temp < 5)
    {
        return NumberGetValue(pElement, pos);
    }

    return MemGetValue(pElement, pos);
}

static int BlobMagicGetIsSupport(SElement *pElement)
{
    ASSERT_NULL(pElement);
    // 支持blob   对于fixblob,因为算法不会减小长度，对最大长度还有判断，所以可以支持
    if ((pElement->para.type == ENUM_BLOB)
        || (pElement->para.type == ENUM_FIXBLOB))
    {
        return ENUM_YES;
    }

    return ENUM_NO;
}

const struct MutaterGroup g_blobMagicGroup = {
    "BlobMagic",
    BlobMagicGetCount,
    BlobMagicGetValue,
    BlobMagicGetIsSupport,
    1
};

void InitBlobMagic(void)
{   
    if (LlvmHookIsSupport() == 0)
    {
        return;
    }
    
    RegisterMutater(&g_blobMagicGroup, ENUM_BLOB_MAGIC);
}

#ifdef __cplusplus
}
#endif