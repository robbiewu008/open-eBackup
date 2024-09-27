/*
版权所有 (c) 华为技术有限公司 2012-2018


原理:					float16数据类型专有变异算法
						

长度:					长度不变		

数量:					n个

支持数据类型: 	float16

*/
#include "../common/PCommon.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct  {
    u16 mantissaBit:10;
    u16 exponentBit:5;
    u16 signedBit:1;
}S_Float16;


int Float16GetCount(SElement *pElement)
{
    ASSERT_NULL(pElement);
    return MAX_COUNT;
}

char* Float16GetValue(SElement *pElement, int pos)
{
    ASSERT_NULL(pElement);
    SetElementInitoutBufEx(pElement, 2);

    HwMemcpy(pElement->para.value, pElement->inBuf, 2);
    
    S_Float16 *outData = (S_Float16 *)pElement->para.value;
    
    int temp = RAND_32() % 20;

    if (temp < 2)
    {
        outData->signedBit = RAND_BOOL();
    }
    else if (temp < 8)
    {
        outData->exponentBit = RAND_RANGE(0, 16);
    }
    else  if (temp < 15)
    {
        outData->mantissaBit = RAND_RANGE(0, 512);
    }
    else
    {
        // 整体随机，管它
        u16 t = RAND_16();
        HwMemcpy(pElement->para.value, &t, 2);
    }
    
    return pElement->para.value;
}


int Float16GetIsSupport(SElement *pElement)
{
    ASSERT_NULL(pElement);
    
    if (pElement->para.type == ENUM_FLOAT16)
    {
        return ENUM_YES;
    }
    return ENUM_NO;
}

const struct MutaterGroup g_float16Group = {
    "Float16",
    Float16GetCount,
    Float16GetValue,
    Float16GetIsSupport,
    1
};

void InitFloat16(void)
{
    RegisterMutater(&g_float16Group, ENUM_MFLOAT16);
}

#ifdef __cplusplus
}
#endif

