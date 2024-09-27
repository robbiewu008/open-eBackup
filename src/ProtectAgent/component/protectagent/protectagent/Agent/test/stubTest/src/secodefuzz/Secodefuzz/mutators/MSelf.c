/*
��Ȩ���� (c) ��Ϊ�������޹�˾ 2012-2018

ԭ��:					mac��������ר�б����㷨
						

����:					���Ȳ���		,��ֵΪ6byte

����:					n��

֧����������: 	mac

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