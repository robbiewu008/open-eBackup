/*
版权所有 (c) 华为技术有限公司 2012-2018


原理:					枚举数字变异

长度:					长度不变			

数量:					枚举数量

支持数据类型: 	枚举数字类型
*/
#include "../common/PCommon.h"
 
#ifdef __cplusplus
extern "C" {
#endif

int NumberEnumGetCount(SElement *pElement)
{
    ASSERT_NULL(pElement);

    return pElement->enumCount;
}

char* NumberEnumGetValue(SElement *pElement, int pos)
{
    int len;
    ASSERT_NULL(pElement);

    //  如果bit长度不被8整除，则加1
    len = (int)(pElement->inLen / 8) + IS_ADD_ONE(pElement->inLen % 8);

    SetElementInitoutBufEx(pElement, len);

    *((s32 *)pElement->para.value) = pElement->enumNumberTable[pos % pElement->enumCount];
    
    return pElement->para.value;
}

int NumberEnumGetIsSupport(SElement *pElement)
{
    ASSERT_NULL(pElement);
    if (pElement->para.type == ENUM_NUMBER_ENUM)
    {
        return ENUM_YES;
    }

    return ENUM_NO;
}

const struct MutaterGroup g_numberEnumGroup = {
    "NumberEnum",
    NumberEnumGetCount,
    NumberEnumGetValue,
    NumberEnumGetIsSupport,
    1
};

void InitNumberEnum(void)
{
    RegisterMutater(&g_numberEnumGroup, ENUM_NUMBER_ENUM_M);
}

#ifdef __cplusplus
}
#endif

