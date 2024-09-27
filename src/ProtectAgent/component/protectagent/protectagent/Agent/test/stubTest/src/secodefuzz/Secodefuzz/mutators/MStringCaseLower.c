/*
��Ȩ���� (c) ��Ϊ�������޹�˾ 2012-2018

ԭ��:					���д�д��ĸ���Сд,���û��Сд��ĸ����Ϊԭֵ
						

����:					���Ȳ���		

����:					1

֧����������: 	�г�ʼֵ���ַ�������

*/
#include "../common/PCommon.h"

#ifdef __cplusplus
extern "C" {
#endif

int StringCaseLowerGetCount(SElement *pElement)
{
    ASSERT_NULL(pElement);
    return 1;
}

char* StringCaseLowerGetValue(SElement *pElement, int pos)
{
    int i;
    int inLen = (int)(pElement->inLen / 8);

    SetElementInitoutBufEx(pElement, inLen);

    // ���Ϊ0������ν
    for (i = 0; i < inLen; i++)
    {
        ((char *)pElement->para.value)[i] = InToLower(((char*)pElement->inBuf)[i]);
    }

    return pElement->para.value;
}

int StringCaseLowerGetIsSupport(SElement *pElement)
{
    // �г�ʼֵ���ַ���
    if ((pElement->para.type == ENUM_STRING)
        && (pElement->isHasInitValue == ENUM_YES))
    {
        return ENUM_YES;
    }

    return ENUM_NO;
}

const struct MutaterGroup g_stringCaseLowerGroup = {
    .name           = "StringCaseLower",
    .getCount       = StringCaseLowerGetCount,
    .getValue       = StringCaseLowerGetValue,
    .getIsSupport   = StringCaseLowerGetIsSupport,
};

void InitStringCaseLower(void)
{
    RegisterMutater(&g_stringCaseLowerGroup, ENUM_STRING_CASE_LOWER);
}

#ifdef __cplusplus
}
#endif