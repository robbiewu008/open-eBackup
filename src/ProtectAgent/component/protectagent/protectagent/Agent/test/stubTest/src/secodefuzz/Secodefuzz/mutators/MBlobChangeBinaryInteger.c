/*

版权所有 (c) 华为技术有限公司 2012-2018

原理:					改变内存，整数随机

长度:					0到最大长度之间

数量:					MAX_COUNT

支持数据类型: 	blob

*/
#include "../common/PCommon.h"

#ifdef __cplusplus
extern "C" {
#endif

#define Integer1			1
#define Integer2			2
#define Integer3			3
#define Integer10		10
#define Integer20		20
#define Integer64		64

int BlobChangeBinaryIntegerGetCount(SElement *pElement)
{
    ASSERT_NULL(pElement);

    // 爬分支变异手段
    return MAX_COUNT;
}

static size_t ChangeBinaryInteger64(u8 *data, size_t size) 
{
    size_t off;
    u64 val;

    if (size < sizeof(u64)) 
    {
        return 0;
    }

    off = RAND_RANGE(0, (size - sizeof(u64)));
    if (off + sizeof(u64)  >  size) 
    {
        return 0;
    }

    if (off < Integer64 && !RAND_RANGE(0, Integer3)) 
    {
        val = size;
        if (RAND_BOOL())
        {
            val = InBswap64(val);
        }
    } 
    else 
    {
        HwMemcpy(&val, data + off, sizeof(val));
        u64 add = RAND_RANGE(0, Integer20);
        add -= Integer10;
        if (RAND_BOOL())
        {
            val = InBswap64((u64)(InBswap64(val) + add)); // add assuming different endiannes.
        }
        else
        {
            val = val + add;               // add assuming current endiannes.
        }
#ifndef _MSC_VER  // vs编译不过
        if (add == 0 || RAND_BOOL()) // Maybe negate.
        {
            val = -val;
        }
#endif
    }

    HwMemcpy(data + off, &val, sizeof(val));
    return size;
}

static size_t ChangeBinaryInteger32(u8 *data, size_t size) 
{
    size_t off;
    u32 val;
    if (size < sizeof(u32)) 
    {
        return 0;
    }

    off = RAND_RANGE(0, (size - sizeof(u32)));
    if (off + sizeof(u32)  >  size) 
    {
        return 0;
    }

    if (off < Integer64 && !RAND_RANGE(0, Integer3)) 
    {
        val = size;
        if (RAND_BOOL())
        {
            val = InBswap32(val);
        }
    } 
    else 
    {
        HwMemcpy(&val, data + off, sizeof(val));
        u32 add = RAND_RANGE(0, Integer20);
        add -= Integer10;
        if (RAND_BOOL())
        {
            val = InBswap32((u32)(InBswap32(val) + add)); // add assuming different endiannes.
        }
        else
        {
            val = val + add;               // add assuming current endiannes.
        }
#ifndef _MSC_VER  // vs编译不过
        if (add == 0 || RAND_BOOL()) // Maybe negate.
        {
            val = -val;
        }
#endif
    }

    HwMemcpy(data + off, &val, sizeof(val));
    return size;
}

static size_t ChangeBinaryInteger16(u8 *data, size_t size) 
{
    size_t off;
    u16 val;
    
    if (size < sizeof(u16)) 
    {
        return 0;
    }
    
    off = RAND_RANGE(0, (size - sizeof(u16)));
    if (off + sizeof(u16)  >  size) 
    {
        return 0;
    }
    
    if (off < Integer64 && !RAND_RANGE(0, Integer3)) 
    {
        val = size;
        if (RAND_BOOL())
        {
            val = InBswap16(val);
        }
    } 
    else 
    {
        HwMemcpy(&val, data + off, sizeof(val));
        u16 add = RAND_RANGE(0, Integer20);
        add -= Integer10;
        if (RAND_BOOL())
        {
            val = InBswap16((u16)(InBswap16(val) + add)); // add assuming different endiannes.
        }
        else
        {
            val = val + add;               // add assuming current endiannes.
        }
        if (add == 0 || RAND_BOOL()) // Maybe negate.
        {
            val = -val;
        }
    }
    
    HwMemcpy(data + off, &val, sizeof(val));
    return size;
}

static size_t ChangeBinaryInteger8(u8 *data, size_t size) 
{
    size_t off;
    u8 val;

    if (size < sizeof(u8)) 
    {
        return 0;
    }

    off = RAND_RANGE(0, (size - sizeof(u8)));
    if (off + sizeof(u8)  >  size) 
    {
        return 0;
    }

    if (off < Integer64 && !RAND_RANGE(0, Integer3)) 
    {
        val = size;
        if (RAND_BOOL())
        {
            val = InBswap8(val);
        }
    } 
    else 
    {
        HwMemcpy(&val, data + off, sizeof(val));
        u8 add = RAND_RANGE(0, Integer20);
        add -= Integer10;

        if (RAND_BOOL())
        {
            val = InBswap8((u8)(InBswap8(val) + add)); // add assuming different endiannes.
        }
        else
        {
            val = val + add;               // add assuming current endiannes.
        }
        if (add == 0 || RAND_BOOL()) // Maybe negate.
        {
            val = -val;
        }
    }
    HwMemcpy(data + off, &val, sizeof(val));
    return size;
}

size_t Mutate_ChangeBinaryIntegerGetValue(u8 *data, size_t size) 
{
    switch (RAND_RANGE(0, Integer3)) 
    {
        case Integer3: 
            return ChangeBinaryInteger64(data, size);
        case Integer2: 
            return ChangeBinaryInteger32(data, size);
        case Integer1: 
            return ChangeBinaryInteger16(data, size);
        case 0: 
            return ChangeBinaryInteger8(data, size);
        default: 
            break;
    }
    return 0;
}

char *BlobChangeBinaryIntegerGetValue(SElement *pElement, int pos)
{
    int inLen;
    ASSERT_NULL(pElement);

    inLen = (int)(pElement->inLen / INTEGER_8);

    SetElementInitoutBufEx(pElement, inLen);

    HwMemcpy(pElement->para.value, pElement->inBuf, inLen);

    Mutate_ChangeBinaryIntegerGetValue((u8 *)pElement->para.value, inLen);

    return pElement->para.value;
}

int BlobChangeBinaryIntegerGetIsSupport(SElement *pElement)
{
    ASSERT_NULL(pElement);
    // 目前仅支持blob,增强buf变异
    if (((pElement->para.type == ENUM_BLOB)
        || (pElement->para.type == ENUM_FIXBLOB)) 
        && (pElement->isHasInitValue == ENUM_YES))
    {
        return ENUM_YES;
    }

    return ENUM_NO;
}

const struct MutaterGroup g_blobChangeBinaryIntegerGroup = {
    "BlobChangeBinaryInteger",
    BlobChangeBinaryIntegerGetCount,
    BlobChangeBinaryIntegerGetValue,
    BlobChangeBinaryIntegerGetIsSupport,
    1
};

void InitBlobChangeBinaryInteger(void)
{
    RegisterMutater(&g_blobChangeBinaryIntegerGroup, ENUM_BLOB_CHANGE_BINARY_INTEGER);
}

#ifdef __cplusplus
}
#endif