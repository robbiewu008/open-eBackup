/*
版权所有 (c) 华为技术有限公司 2012-2018


原理:					通过复制元素的个数来生成测试例
						个数多少使用高斯变异
						

长度:					原始数据长度的整数倍	

数量:					最大输出长度开平方与MAXCOUNT/5的最小值

支持数据类型: 	有初始值的可变数据

*/
#include "../common/PCommon.h"

#ifdef __cplusplus
extern "C" {
#endif

int DataElementDuplicateGetCount(SElement *pElement)
{
    ASSERT_NULL(pElement);
    return MIN(Insqrt(pElement->para.maxLen), MAX_COUNT / 100);
}

char *DataElementDuplicateGetValue(SElement *pElement, int pos)
{
    int i;
    int inLen;
    int times; 
    int maxTimes;
    ASSERT_NULL(pElement);
    
    inLen = (int)(pElement->inLen / 8);
    maxTimes = pElement->para.maxLen / inLen;

    // 得到重复几次
    times = GaussRandU32(pos % InGetBitNumber(maxTimes));

    SetElementInitoutBufEx(pElement, inLen * times);

    for (i = 0; i < times; i++)
    { 
        HwMemcpy(pElement->para.value + i * inLen, pElement->inBuf, inLen); 
    }

    return pElement->para.value;
}

int DataElementDuplicateGetIsSupport(SElement *pElement)
{
    ASSERT_NULL(pElement);
    // 有初始值的可变数据
    if (((pElement->para.type == ENUM_STRING)
        || (pElement->para.type == ENUM_BLOB))
        && (pElement->isHasInitValue == ENUM_YES))
    {
        return ENUM_YES;
    }
    
    return ENUM_NO;
}

const struct MutaterGroup g_dataElementDuplicateGroup = {
    "DataElementDuplicate",
    DataElementDuplicateGetCount,
    DataElementDuplicateGetValue,
    DataElementDuplicateGetIsSupport,
    1
};

void InitDataElementDuplicate(void)
{
    RegisterMutater(&g_dataElementDuplicateGroup, ENUM_DATAELEMENT_DUPLICATE);
}

#ifdef __cplusplus
}
#endif

