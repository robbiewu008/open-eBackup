/*
��Ȩ���� (c) ��Ϊ�������޹�˾ 2012-2018


ԭ��:					����ı��ַ������ַ��Ĵ�Сд��
						ÿ�θı���ٸ���ĸʹ�ø�˹����
						

����:					���Ȳ���		

����:					��ĸ������8��MAXCOUNT����Сֵ

֧����������: 	�г�ʼֵ���ַ�������

*/
#include "../common/PCommon.h"

#ifdef __cplusplus
extern "C" {
#endif

int StringCaseRandomGetCount(SElement *pElement)
{
    ASSERT_NULL(pElement);
    
    // Ŀǰ������������㷨����Ϊbit����500����Сֵ
    return MIN(InGetLetterNumber((char*)pElement->inBuf) * 8, MAX_COUNT);
}

char* StringCaseRandomGetValue(SElement *pElement, int pos)
{
    int i;
    int inLen;
    char tmp1, tmp2;

    inLen = (int)(pElement->inLen / 8);

    SetElementInitoutBufEx(pElement, inLen);

    int number = InGetLetterNumber((char*)pElement->inBuf);

    // ������α���伸����ĸ
    int number1 = GaussRandU32(pos % InGetBitNumber(number));

    for (i = 0; i < inLen; i++)
    {
        tmp1 = ((char*)pElement->inBuf)[i];

        if (InIsLetter(tmp1))
        {
            // ���������ĸ�Ƿ���Ҫ����
            if (RAND_RANGE(1, number) <= number1)
            {
                tmp2 = (char)InToUpper(tmp1);
                if (tmp1 == tmp2)
                {
                    tmp2 = (char)InToLower(tmp1);
                }
                tmp1 = tmp2;
            }
        }
        ((char *)pElement->para.value)[i] = tmp1;
    }
    // ���Ϊ0������ν

    return pElement->para.value;
}

int StringCaseRandomGetIsSupport(SElement *pElement)
{
    // �г�ʼֵ���ַ���
    if ((pElement->para.type == ENUM_STRING)
        && (pElement->isHasInitValue == ENUM_YES))
    {
        return ENUM_YES;
    }

    return ENUM_NO;
}

const struct MutaterGroup g_stringCaseRandomGroup = {
    .name           = "StringCaseRandom",
    .getCount       = StringCaseRandomGetCount,
    .getValue       = StringCaseRandomGetValue,
    .getIsSupport   = StringCaseRandomGetIsSupport,
};

void InitStringCaseRandom(void)
{
    RegisterMutater(&g_stringCaseRandomGroup, ENUM_STRING_CASE_RANDOM);
}

#ifdef __cplusplus
}
#endif

