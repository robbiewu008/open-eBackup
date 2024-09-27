/*
版权所有 (c) 华为技术有限公司 2012-2018

原理:					算法通过改变数据几个bit值来产生测试用例,
						一个测试用例只改变2-6个bit
						

长度:					长度不变		

数量:					bit数乘2与MAXCOUNT/5的最小值

支持数据类型: 	有初值,枚举类除外

*/
#include "../common/PCommon.h"

#ifdef __cplusplus
extern "C" {
#endif

int DataElementMBitFlipperGetCount(SElement *pElement)
{
    ASSERT_NULL(pElement);

    return MIN((int)pElement->inLen * 2, MAX_COUNT / 5);
}

char* DataElementMBitFlipperGetValue(SElement *pElement, int pos)
{
    // 如果bit长度不被8整除，则加1
    int inLen;
    int count;
    ASSERT_NULL(pElement);
    
    inLen = (int)(pElement->inLen / 8) + IS_ADD_ONE(pElement->inLen % 8);

    SetElementInitoutBufEx(pElement, inLen);
    
    HwMemcpy(pElement->para.value, pElement->inBuf, inLen);

    count = RAND_RANGE(2, 6);
    while (count--)
    {
        FLIP_BIT(pElement->para.value, RAND_RANGE(0, pElement->inLen - 1));
    }
    
    return   pElement->para.value;
}

int DataElementMBitFlipperGetIsSupport(SElement *pElement)
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
    
    // 只要有初始值，就支持
    if (pElement->isHasInitValue == ENUM_YES)
    {
        return ENUM_YES;
    }

    return ENUM_NO;
}

const struct MutaterGroup g_dataElementMBitFlipperGroup = {
    "DataElementMBitFlipper",
    DataElementMBitFlipperGetCount,
    DataElementMBitFlipperGetValue,
    DataElementMBitFlipperGetIsSupport,
    1
};

void InitDataElementMBitFlipper(void)
{
    RegisterMutater(&g_dataElementMBitFlipperGroup, ENUM_DATAELEMENT_MBIT_FLIPPER);
}

#ifdef __cplusplus
}
#endif