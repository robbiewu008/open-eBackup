/*
��Ȩ���� (c) ��Ϊ�������޹�˾ 2012-2018


ԭ��:					����hook��Դ���ݲ���򸲸�

����:					

����:					MAX_COUNT*2

֧����������: 	ENUM_STRING
*/
#include "../common/PCommon.h"

#ifdef __cplusplus
extern "C" {
#endif

static int StringMagicGetCount(SElement *pElement)
{
    ASSERT_NULL(pElement);

    if (LlvmDataMemGetCount() == 0)
    {
        return 0;
    }

    return MAX_COUNT * 2;
}

char* StringMagicGetValue(SElement *pElement, int pos)
{
    int llvmLen;
    char * value =NULL;

    ASSERT_NULL(pElement);

    // \0���ӵ��߼��ں������
    value = LlvmDataMemGetValue(&llvmLen);

    //��û��ֵ�����
    if (llvmLen == 0)
    {
        return SetElementOriginalValue(pElement);
    }

    int isInsert = RAND_32() % 3;

    // 0 Insert 1 Overwrite 2 replace
    MagicGetValue(pElement, value, llvmLen, isInsert);

    return pElement->para.value;
}

static int StringMagicGetIsSupport(SElement *pElement)
{
    ASSERT_NULL(pElement);
    // �ַ���
    if (pElement->para.type == ENUM_STRING)
    {
        return ENUM_YES;
    }

    return ENUM_NO;
}

const struct MutaterGroup g_stringMagicGroup = {
    .name 			= "StringMagic",
    .getCount 		= StringMagicGetCount,
    .getValue 		= StringMagicGetValue,
    .getIsSupport 	= StringMagicGetIsSupport,
};

void InitStringMagic(void)
{	
    if (LlvmHookIsSupport() == 0)
    {
        return;
    }

    RegisterMutater(&g_stringMagicGroup, ENUM_STRING_MAGIC);
}

#ifdef __cplusplus
}
#endif

