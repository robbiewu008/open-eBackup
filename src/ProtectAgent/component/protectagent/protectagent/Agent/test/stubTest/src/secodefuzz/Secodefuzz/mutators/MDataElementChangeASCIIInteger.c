/*
版权所有 (c) 华为技术有限公司 2012-2018


原理:					改变数据里边的ascii为数字的部分，用数字随机

长度:					0到最大长度之间

数量:					MAX_COUNT

支持数据类型: 	blob ,FixBlob,String

*/
#include "../common/PCommon.h"

#ifdef __cplusplus
extern "C" {
#endif

int DataElementChangeASCIIIntegerGetCount(SElement *pElement)
{
    ASSERT_NULL(pElement);

    // 爬分支变异手段，因此变异数量有所倾向
    return MAX_COUNT;
}

size_t MutateChangeASCIIInteger(uint8_t *data, size_t size) 
{
    size_t i;
    size_t e;
    size_t b = RAND_RANGE(0, size - 1);
    
    while (b < size && !InIsDigit(data[b])) 
    {
        b++;
    }

    if (b == size) 
    {
        return 0;
    }

    e = b;
    while (e < size && InIsDigit(data[e])) 
    {
        e++;
    }

    // now we have digits in [b, e).
    // strtol and friends don't accept non-zero-teminated data, parse it manually.
    uint64_t val = data[b] - '0';
    for (i = b + 1; i < e; i++)
    {
        val = val * 10 + data[i] - '0';
    }

    // Mutate the integer value.
    switch (RAND_RANGE(0, 4)) 
    {
        case 0: 
            val++; 
            break;
        case 1: 
            val--; 
            break;
        case 2: 
            val /= 2; 
            break;
        case 3: 
            val *= 2; 
            break;
        case 4: 
            val = RAND_RANGE(0, val * val);
            break;
        default: ;
    }
    // Just replace the bytes with the new ones, don't bother moving bytes.
    for (i = b; i < e; i++) 
    {
        size_t idx = e + b - i - 1;
        data[idx] = (val % 10) + '0';
        val /= 10;
    }
    return size;
}

char *DataElementChangeASCIIIntegerGetValue(SElement *pElement, int pos)
{
    int inLen;
    ASSERT_NULL(pElement);

    inLen = (int)(pElement->inLen / 8);

    SetElementInitoutBufEx(pElement, inLen);

    HwMemcpy(pElement->para.value, pElement->inBuf, inLen);

    MutateChangeASCIIInteger((uint8_t *)pElement->para.value, inLen);

    return pElement->para.value;
}

int DataElementChangeASCIIIntegerGetIsSupport(SElement *pElement)
{
    ASSERT_NULL(pElement);
    // 目前仅支持blob,增强buf变异
    if (((pElement->para.type == ENUM_BLOB)
        || (pElement->para.type == ENUM_FIXBLOB)
        || (pElement->para.type == ENUM_STRING)) 
        && (pElement->isHasInitValue == ENUM_YES))
    {
        return ENUM_YES;
    }

    return ENUM_NO;
}

const struct MutaterGroup g_dataElementChangeASCIIIntegerGroup = {
    "DataElementChangeASCIIInteger",
    DataElementChangeASCIIIntegerGetCount,
    DataElementChangeASCIIIntegerGetValue,
    DataElementChangeASCIIIntegerGetIsSupport,
    1
};

void InitDataElementChangeASCIIInteger(void)
{
    RegisterMutater(&g_dataElementChangeASCIIIntegerGroup, ENUM_DATAELEMENT_CHANGE_ASCII_INTEGER);
}

#ifdef __cplusplus
}
#endif

