/*
版权所有 (c) 华为技术有限公司 2012-2018


原理:					对数据bit置0，从头置1和从尾置0，置0个数依次为1到置满
						

长度:					长度不变		

数量:					bit数乘2

支持数据类型: 	有初值,枚举类不支持

*/
#include "../common/PCommon.h"

#ifdef __cplusplus
extern "C" {
#endif

int DataElementBitZeroGetCount(SElement *pElement)
{
    ASSERT_NULL(pElement);

    return  MIN(pElement->inLen * 2, MAX_COUNT / 5);
}

char* DataElementBitZeroGetValue(SElement *pElement, int pos)
{
    // 如果bit长度不被8整除，则加1
    int inLen, i;
    ASSERT_NULL(pElement);

    pos = RAND_RANGE(0, pElement->inLen * 2 - 1);
    
    inLen = (int)(pElement->inLen / 8) + IS_ADD_ONE(pElement->inLen % 8);

    SetElementInitoutBufEx(pElement, inLen);
    
    HwMemcpy(pElement->para.value, pElement->inBuf, inLen);

    if (pos < pElement->inLen)
    {
        for (i = 0; i <= pos; i++)
        {
            ZERO_BIT(pElement->para.value, i);
        }
    }
    else
    {
        pos = pos - pElement->inLen;

        for (i = pos; i < pElement->inLen; i++)
        {
            ZERO_BIT(pElement->para.value, i);
        }
    }

    return   pElement->para.value;
}

int DataElementBitZeroGetIsSupport(SElement *pElement)
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
    
    // 只要有初始值，就支持，对所有数据类型开放有意义否?
    if (pElement->isHasInitValue == ENUM_YES)
    {
        return ENUM_YES;
    }
    return ENUM_NO;
}

const struct MutaterGroup g_dataElementBitZeroGroup = {
    "DataElementBitZero",
    DataElementBitZeroGetCount,
    DataElementBitZeroGetValue,
    DataElementBitZeroGetIsSupport,
    1
};

void InitDataElementBitZero(void)
{
    RegisterMutater(&g_dataElementBitZeroGroup, ENUM_DATAELEMENT_BIT_ZERO);
}

#ifdef __cplusplus
}
#endif

