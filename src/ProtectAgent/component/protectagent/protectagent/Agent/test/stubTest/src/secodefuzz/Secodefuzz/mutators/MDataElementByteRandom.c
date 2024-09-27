/*
版权所有 (c) 华为技术有限公司 2012-2018


原理:					原始数据的某个byte的值被随机

长度:					长度不变

数量:					byte数量乘256与MAXCOUNT*4的最小值

支持数据类型: 	有初值,枚举类除外

*/
#include "../common/PCommon.h"

#ifdef __cplusplus
extern "C" {
#endif

int DataElementByteRandomGetCount(SElement *pElement)
{
    ASSERT_NULL(pElement);

    if (pElement->inLen == 0)
    {
        return 0;
    }

    // 爬分支最好的变异手段，因此变异数量有所倾向
    return MAX_COUNT * 4;
}

char *DataElementByteRandomGetValue(SElement *pElement, int pos)
{ 
    int inLen;
    int start;  

    ASSERT_NULL(pElement);

    inLen = (int)(pElement->inLen / 8);

    // 随机得到 位置,因为有初值，所以不用考虑in_len=0
    start = RAND_RANGE(0, inLen - 1);

    SetElementInitoutBufEx(pElement, inLen);

    HwMemcpy(pElement->para.value, pElement->inBuf, inLen); 

    //一半机会给专有byte，发现问题的概率更大一些
    if (RAND_BOOL())
    {
        pElement->para.value[start] = RAND_BYTE();
    }
    else
    {
        pElement->para.value[start] = g_specialByte[RAND_RANGE(0, g_specialByteLen - 1)];
    }

    return pElement->para.value;
}

int DataElementByteRandomGetIsSupport(SElement *pElement)
{
    ASSERT_NULL(pElement);

    // 枚举不支持
    if (InGetTypeIsEnumOrRange(pElement->para.type) == ENUM_YES)
    {
        return ENUM_NO;
    }

    if (pElement->para.type == ENUM_STRING_NUM)
    {
        return ENUM_NO;
    }

    // self变异算法先屏蔽，想要打开自己打
    if (pElement->para.type == ENUM_TSELF)
    {
        return ENUM_NO;
    }
    
    // 有初始值的可变数据
    if (pElement->isHasInitValue == ENUM_YES)
    {
        return ENUM_YES;
    }

    return ENUM_NO;
}

const struct MutaterGroup g_dataElementByteRandomGroup = {
    "DataElementByteRandom",
    DataElementByteRandomGetCount,
    DataElementByteRandomGetValue,
    DataElementByteRandomGetIsSupport,
    1
};

void InitDataElementByteRandom(void)
{
    RegisterMutater(&g_dataElementByteRandomGroup, ENUM_DATAELEMENT_BYTE_RANDOM);
}

#ifdef __cplusplus
}
#endif

