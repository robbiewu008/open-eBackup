/*
��Ȩ���� (c) ��Ϊ�������޹�˾ 2012-2018

ԭ��:					����Сд��ĸ��ɴ�д,���û��Сд��ĸ����Ϊԭֵ
						

����:					���Ȳ���		

����:					1

֧����������: 	�г�ʼֵ���ַ�������

*/
#include "../common/PCommon.h"

#ifdef __cplusplus
extern "C" {
#endif

int StringCaseUpperGetCount(SElement *pElement)
{
    ASSERT_NULL(pElement);
    return 1;
}

char* StringCaseUpperGetValue(SElement *pElement, int pos)
{
    int i;
    int inLen = (int)(pElement->inLen / 8);

    SetElementInitoutBufEx(pElement, inLen);

    // ���Ϊ0������ν
    for (i = 0; i < inLen; i++)
    {
        ((char *)pElement->para.value)[i] = InToUpper(((char*)pElement->inBuf)[i]);
    }

    return pElement->para.value;
}

int StringCaseUpperGetIsSupport(SElement *pElement)
{
    // �г�ʼֵ���ַ���
    if ((pElement->para.type == ENUM_STRING)
        && (pElement->isHasInitValue == ENUM_YES))
    {
        return ENUM_YES;
    }

    return ENUM_NO;
}

const struct MutaterGroup g_stringCaseUpperGroup = {
    .name             = "StringCaseUpper",
    .getCount        = StringCaseUpperGetCount,
    .getValue         = StringCaseUpperGetValue,
    .getIsSupport   = StringCaseUpperGetIsSupport,
};

void InitStringCaseUpper(void)
{
    RegisterMutater(&g_stringCaseUpperGroup, ENUM_STRING_CASE_UPPER);
}

#ifdef __cplusplus
}
#endif