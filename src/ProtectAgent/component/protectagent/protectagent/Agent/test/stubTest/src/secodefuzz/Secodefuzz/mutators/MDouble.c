/*
��Ȩ���� (c) ��Ϊ�������޹�˾ 2012-2018


ԭ��:					double��������ר�б����㷨
						

����:					���Ȳ���		

����:					n��

֧����������: 	double

*/
#include "../common/PCommon.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct  {
    u64 mantissaBit:52;
    u64 exponentBit:11;
    u64 signedBit:1;
}S_Double;

int DoubleGetCount(SElement *pElement)
{
    ASSERT_NULL(pElement);
    return MAX_COUNT;
}

char* DoubleGetValue(SElement *pElement, int pos)
{
    ASSERT_NULL(pElement);
    SetElementInitoutBufEx(pElement, 8);

    HwMemcpy(pElement->para.value, pElement->inBuf, 8);
    
    S_Double *outData = (S_Double *)pElement->para.value;
    
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



int DoubleGetIsSupport(SElement *pElement)
{
    ASSERT_NULL(pElement);
    
    if (pElement->para.type == ENUM_DOUBLE)
    {
        return ENUM_YES;
    }
    return ENUM_NO;
}

const struct MutaterGroup g_doubleGroup = {
    "Double",
    DoubleGetCount,
    DoubleGetValue,
    DoubleGetIsSupport,
    1
};

void InitDouble(void)
{
    RegisterMutater(&g_doubleGroup, ENUM_MDOUBLE);
}

#ifdef __cplusplus
}
#endif

