/*
��Ȩ���� (c) ��Ϊ�������޹�˾ 2012-2018


ԭ��:					ÿ�α���n��byte,ʹ�����ascii��ֵ
						���켸��byteʹ�ø�˹����
						

����:					���Ȳ���		

����:					bit������4��MAXCOUNT����Сֵ

֧����������: 	�г�ʼֵ���ַ�������

*/
#include "../common/PCommon.h"

#ifdef __cplusplus
extern "C" {
#endif

int StringAsciiRandomGetCount(SElement *pElement)
{
    ASSERT_NULL(pElement);
    
    // Ŀǰ������������㷨����Ϊbit����4��500����Сֵ
    return MIN(pElement->inLen * 4, MAX_COUNT);
}

char* StringAsciiRandomGetValue(SElement *pElement, int pos)
{
    int i;
    int inLen;

    inLen = (int)(pElement->inLen / 8);

    SetElementInitoutBufEx(pElement, inLen);

    // ������α���伸����ĸ
    int number1=GaussRandU32(pos % InGetBitNumber(inLen));

    HwMemcpy(pElement->para.value, pElement->inBuf, inLen);

    for (i = 0; i < inLen; i++)
    {
        // ���������ĸ�Ƿ���Ҫ����
        if(RAND_RANGE(1, inLen) <= number1)
        {
            ((char *)pElement->para.value)[i]  = (char)RAND_RANGE(1, 127);
        }
    }

    ((char *)pElement->para.value)[inLen - 1] = 0;

    return pElement->para.value;
}

int StringAsciiRandomGetIsSupport(SElement *pElement)
{
    // �г�ʼֵ���ַ���
    if ((pElement->para.type == ENUM_STRING)
        && (pElement->isHasInitValue == ENUM_YES))
    {
        return ENUM_YES;
    }

    return ENUM_NO;
}

const struct MutaterGroup g_stringAsciiRandomGroup = {
    .name           = "StringAsciiRandom",
    .getCount       = StringAsciiRandomGetCount,
    .getValue       = StringAsciiRandomGetValue,
    .getIsSupport   = StringAsciiRandomGetIsSupport,
};

void InitStringAsciiRandom(void)
{
    RegisterMutater(&g_stringAsciiRandomGroup, ENUM_STRING_ASCII_RANDOM);
}

#ifdef __cplusplus
}
#endif

