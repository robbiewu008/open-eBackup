/*
版权所有 (c) 华为技术有限公司 2012-2018


原理:					float32数据类型专有变异算法
						

长度:					长度不变		

数量:					n个

支持数据类型: 	float32

*/
#include "../common/PCommon.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct  {
    u32 mantissaBit:23;
    u32 exponentBit:8;
    u32 signedBit:1;
}S_Float32;

int Float32GetCount(SElement *pElement)
{
    ASSERT_NULL(pElement);
    return MAX_COUNT;
}

char* Float32GetValue(SElement *pElement, int pos)
{
    ASSERT_NULL(pElement);
    SetElementInitoutBufEx(pElement, 4);

    HwMemcpy(pElement->para.value, pElement->inBuf, 4);
    
    S_Float32 *outData = (S_Float32 *)pElement->para.value;
    
    int temp = RAND_32() % 20;

    if (temp < 2)
    {
        outData->signedBit = RAND_BOOL();
    }
    else if (temp < 8)
    {
        outData->exponentBit = RAND_RANGE(0, 128);
    }
    else  if (temp < 15)
    {
        int a = RAND_RANGE(0, 23);
        outData->mantissaBit = GaussRandS32(a);
    }
    else
    {
        // 整体随机，管它
        u32 t = RAND_32();
        HwMemcpy(pElement->para.value, &t, 4);
    }
    
    return pElement->para.value;
}

int Float32GetIsSupport(SElement *pElement)
{
    ASSERT_NULL(pElement);
    
    if (pElement->para.type == ENUM_FLOAT32)
    {
        return ENUM_YES;
    }
    return ENUM_NO;
}

const struct MutaterGroup g_float32Group = {
    "Float32",
    Float32GetCount,
    Float32GetValue,
    Float32GetIsSupport,
    1
};

void InitFloat32(void)
{
    RegisterMutater(&g_float32Group, ENUM_MFLOAT32);
}

#ifdef __cplusplus
}
#endif


