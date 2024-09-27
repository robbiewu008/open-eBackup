/*
��Ȩ���� (c) ��Ϊ�������޹�˾ 2012-2018


ԭ��:					float64��������ר�б����㷨
						

����:					���Ȳ���		

����:					n��

֧����������: 	float64

*/
#include "../common/PCommon.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct  {
    u64 mantissaBit:52;
    u64 exponentBit:11;
    u64 signedBit:1;
}S_Float64;

int Float64GetCount(SElement *pElement)
{
    ASSERT_NULL(pElement);
    return MAX_COUNT;
}

char* Float64GetValue(SElement *pElement, int pos)
{
    ASSERT_NULL(pElement);
    SetElementInitoutBufEx(pElement, 8);

    HwMemcpy(pElement->para.value, pElement->inBuf, 8);
    
    S_Float64 *outData = (S_Float64 *)pElement->para.value;
    
    int temp = RAND_32() % 20;

    if (temp < 2)
    {
        outData->signedBit = RAND_BOOL();
    }
    else if (temp < 8)
    {
        outData->exponentBit = RAND_RANGE(0, 1024);
    }
    else  if (temp < 15)
    {
        int a = RAND_RANGE(0, 52);
        outData->mantissaBit = GaussRandS64(a);
    }
    else
    {
        // �������������
        u64 t = RAND_64();
        HwMemcpy(pElement->para.value, &t, 8);
    }
    
    return pElement->para.value;
}

int Float64GetIsSupport(SElement *pElement)
{
    ASSERT_NULL(pElement);
    
    if (pElement->para.type == ENUM_FLOAT64)
    {
        return ENUM_YES;
    }
    return ENUM_NO;
}

const struct MutaterGroup g_float64Group = {
    "Float64",
    Float64GetCount,
    Float64GetValue,
    Float64GetIsSupport,
    1
};

void InitFloat64(void)
{
    RegisterMutater(&g_float64Group, ENUM_MFLOAT64);
}

#ifdef __cplusplus
}
#endif

