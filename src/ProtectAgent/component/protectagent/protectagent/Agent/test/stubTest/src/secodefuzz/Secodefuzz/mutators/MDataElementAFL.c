/*
版权所有 (c) 华为技术有限公司 2012-2018

原理:					变异算法来自AFL

长度:					0到原始长度之间

数量:					变异的数量是byte数量乘8

支持数据类型: 	有初始值的可变数据类型

*/
#include "../common/PCommon.h"

#ifdef __cplusplus
extern "C" {
#endif

int DataElementAFLGetCount(SElement *pElement)
{
    ASSERT_NULL(pElement);

    // 爬分支变异手段，因此变异数量有所倾向
    return 0;
}

char *DataElementAFLGetValue(SElement *pElement, int pos)
{
    return NULL;
}

int DataElementAFLGetIsSupport(SElement *pElement)
{
    ASSERT_NULL(pElement);
    // 有初始值的可变数据
    if (((pElement->para.type == ENUM_BLOB) 
        || (pElement->para.type == ENUM_FIXBLOB))
        && (pElement->isHasInitValue == ENUM_YES))
    {
        return ENUM_YES;
    }

    return ENUM_NO;
}

const struct MutaterGroup g_dataElementAFLGroup = {
    "DataElementAFL",
    DataElementAFLGetCount,
    DataElementAFLGetValue,
    DataElementAFLGetIsSupport,
    1
};

void InitDataElementAFL(void)
{
    RegisterMutater(&g_dataElementAFLGroup, ENUM_DATAELEMENT_AFL);
}

#ifdef __cplusplus
}
#endif