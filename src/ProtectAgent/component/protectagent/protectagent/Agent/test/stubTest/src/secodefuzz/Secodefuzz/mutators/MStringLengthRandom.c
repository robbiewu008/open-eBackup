/*
��Ȩ���� (c) ��Ϊ�������޹�˾ 2012-2018


ԭ��:					�����ַ�������  ��
						�����㷨Ϊƽ�����죬
						ʹ���ַ����������
						

����:					���ֵ��1֮����죬

����:					���������ȿ�ƽ����MAXCOUNT/5����Сֵ

֧����������: 	�г�ʼֵ���ַ�������
*/
#include "../common/PCommon.h"

#ifdef __cplusplus
extern "C" {
#endif

static int StringLengthRandomGetCount(SElement *pElement)
{
    ASSERT_NULL(pElement);
    return MIN(Insqrt(pElement->para.maxLen), MAX_COUNT/100);
}

static char* StringLengthRandomGetValue(SElement *pElement, int pos)
{
    int i;
    int outLen;
    int inLen;

    ASSERT_NULL(pElement);

    if (pos == 0)
    {
        pos = 1;
    }

    inLen = (int)(pElement->inLen / 8);
    outLen = RAND_RANGE(1, pElement->para.maxLen);

    SetElementInitoutBufEx(pElement, outLen);

    for (i = 0; i < outLen / (inLen - 1); i++)
    { 
        HwMemcpy(pElement->para.value + i * (inLen - 1), pElement->inBuf, (inLen - 1)); 
    }

    // ����ʣ�µ�
    HwMemcpy(pElement->para.value + i * (inLen - 1), 
        pElement->inBuf, outLen - (outLen / (inLen - 1)) * (inLen - 1)); 

    ((char *)pElement->para.value)[outLen-1] = 0; 
    return pElement->para.value;

}

static int StringLengthRandomGetIsSupport(SElement *pElement)
{
    ASSERT_NULL(pElement);
    // �г�ʼֵ���ַ���
    if ((pElement->para.type == ENUM_STRING)
        && (pElement->isHasInitValue == ENUM_YES)
        && ((int)(pElement->inLen / 8) > 1))
    {
        return ENUM_YES;
    }

    return ENUM_NO;
}

const struct MutaterGroup g_stringLengthRandomGroup = {
    .name 			= "StringLengthRandom",
    .getCount 		= StringLengthRandomGetCount,
    .getValue 		= StringLengthRandomGetValue,
    .getIsSupport 	= StringLengthRandomGetIsSupport,
};

void InitStringLengthRandom(void)
{
    RegisterMutater(&g_stringLengthRandomGroup, ENUM_STRING_LENGTH_RANDOM);
}

#ifdef __cplusplus
}
#endif

