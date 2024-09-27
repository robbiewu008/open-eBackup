/*
��Ȩ���� (c) ��Ϊ�������޹�˾ 2012-2018


ԭ��:					�����ַ�������  ��
						�����㷨Ϊ��˹���죬
						ʹ�������ascii�������ַ�����
						ż����������ԭ���ַ��������䣬
						������������ʹ��ԭ���ַ���						

����:					���������ֵ��1֮��				

����:					���������ȿ�ƽ����MAXCOUNT����Сֵ

֧����������: 	�ַ�������

*/
#include "../common/PCommon.h"

#ifdef __cplusplus
extern "C" {
#endif

int StringLengthAsciiRandomGetCount(SElement *pElement)
{
    ASSERT_NULL(pElement);

    return MIN(Insqrt(pElement->para.maxLen), MAX_COUNT/100);
}

char* StringLengthAsciiRandomGetValue(SElement *pElement, int pos)
{
    int i;
    int start, changeLen, outLen;
    int inLen = 0;

    changeLen = GaussRandU32(pos % InGetBitNumber(pElement->para.maxLen));

    // ż����������Դ�ַ�����׷��
    // ������������ʹ��ԭʼ�ַ���
    if (pos % 2 == 0)
    {
        if (pElement->isHasInitValue == ENUM_YES)
        {
            inLen = (int)(pElement->inLen / 8);
        }
        else 
        {
         inLen = 0;
        }
    }
    outLen = inLen + changeLen;

    if (outLen > pElement->para.maxLen)
    {
        outLen = pElement->para.maxLen;
    }

    SetElementInitoutBufEx(pElement, outLen);

    if (inLen > 0)
    {
        HwMemcpy(pElement->para.value, pElement->inBuf, inLen);
        start = inLen -1;
    }
    else
    {
        start = 0;
    }

    for (i = start; i < outLen - 1; i++)
    {
        ((char *)pElement->para.value)[i] = (char)RAND_RANGE(0x20, 127);
    }

    ((char *)pElement->para.value)[outLen - 1] = 0;

    return pElement->para.value;
}

int StringLengthAsciiRandomGetIsSupport(SElement *pElement)
{
    // ������������Ҫ�г�ʼֵ
    if (pElement->para.type == ENUM_STRING)
    {
        return ENUM_YES;
    }

    return ENUM_NO;
}

const struct MutaterGroup g_stringLengthAsciiRandomGroup = {
    .name 			= "StringLengthAsciiRandom",
    .getCount 		= StringLengthAsciiRandomGetCount,
    .getValue 		= StringLengthAsciiRandomGetValue,
    .getIsSupport 	= StringLengthAsciiRandomGetIsSupport,
};

void InitStringLengthAsciiRandom(void)
{
    RegisterMutater(&g_stringLengthAsciiRandomGroup, ENUM_STRING_LENGTH_ASCII_RANDOM);
}

#ifdef __cplusplus
}
#endif

