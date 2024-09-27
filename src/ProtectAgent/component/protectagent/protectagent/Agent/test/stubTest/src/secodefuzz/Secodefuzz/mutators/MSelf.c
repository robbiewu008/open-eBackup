/*
版权所有 (c) 华为技术有限公司 2012-2018

原理:					mac数据类型专有变异算法
						

长度:					长度不变		,定值为6byte

数量:					n个

支持数据类型: 	mac

*/
#include "../common/PCommon.h"

#ifdef __cplusplus
extern "C" {
#endif

 int SelfGetCount(SElement *pElement)
{
    ASSERT_NULL(pElement);
    return MAX_COUNT;
}

char* SelfGetValue1(SElement *pElement, int pos)
{
    ASSERT_NULL(pElement);
    int inLen;
    
    inLen = (int)(pElement->inLen / 8) + IS_ADD_ONE(pElement->inLen % 8);

    SetElementInitoutBufEx(pElement, inLen);

    HwMemset(pElement->para.value, 1, inLen);
    
    return pElement->para.value;
}

char* SelfGetValuedefault(SElement *pElement, int pos)
{
    ASSERT_NULL(pElement);
    int inLen;
    
    inLen = (int)(pElement->inLen / 8) + IS_ADD_ONE(pElement->inLen % 8);

    SetElementInitoutBufEx(pElement, inLen);

    HwMemcpy(pElement->para.value, pElement->inBuf, inLen);
    
    return pElement->para.value;
}

char* SelfGetValue(SElement *pElement, int pos)
{
    ASSERT_NULL(pElement);

    if(pElement->arg == 1)
        return SelfGetValue1(pElement, pos);
    
    return SelfGetValuedefault(pElement, pos);
}

int SelfGetIsSupport(SElement *pElement)
{
    ASSERT_NULL(pElement);
    
    if (pElement->para.type == ENUM_TSELF)
    {
        return ENUM_YES;
    }

    return ENUM_NO;
}

const struct MutaterGroup g_selfGroup = {
    "self",
    SelfGetCount,
    SelfGetValue,
    SelfGetIsSupport,
    1
};

void InitSelf(void)
{
    RegisterMutater(&g_selfGroup, ENUM_MSELF);
}

#ifdef __cplusplus
}
#endif