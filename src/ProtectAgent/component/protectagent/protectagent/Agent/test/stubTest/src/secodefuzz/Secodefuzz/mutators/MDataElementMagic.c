/*
��Ȩ���� (c) ��Ϊ�������޹�˾ 2012-2018


ԭ��:					����hook��Դ�����滻

����:					

����:					MAX_COUNT*2

֧����������: 	�ɱ䳤��������
*/
#include "../common/PCommon.h"

#ifdef __cplusplus
extern "C" {
#endif

int DataElementMagicGetCount(SElement *pElement)
{
    if (LlvmDataMemGetCount() == 0)
    {
        return 0;
    }
    
    return MAX_COUNT * 2;
}

char* DataElementMagicGetValue(SElement *pElement, int pos)
{
    int llvmLen;
    char* value =NULL;
    ASSERT_NULL(pElement);

    value = LlvmDataMemGetValue(&llvmLen);

    // ��û��ֵ�����
    if (llvmLen == 0)
    {
        return SetElementOriginalValue(pElement);
    }

    if (llvmLen > pElement->para.maxLen)
    {
        llvmLen = pElement->para.maxLen;
    }

    SetElementInitoutBufEx(pElement, llvmLen);
    HwMemcpy(pElement->para.value, value, llvmLen); 

    return (char *)pElement->para.value;
}

int DataElementMagicGetIsSupport(SElement *pElement)
{
    if ((pElement->para.type == ENUM_STRING)
        || (pElement->para.type == ENUM_BLOB)
        || (pElement->para.type == ENUM_FIXBLOB))
    {
        return ENUM_YES;
    }

    return ENUM_NO;
}

const struct MutaterGroup g_dataElementMagicGroup = {
    "DataElementMagic",
    DataElementMagicGetCount,
    DataElementMagicGetValue,
    DataElementMagicGetIsSupport,
    1
};

void InitDataElementMagic(void)
{
    if (LlvmHookIsSupport() == 0)
    {
        return;
    }
    
    RegisterMutater(&g_dataElementMagicGroup, ENUM_DATAELEMENT_MAGIC);
}

#ifdef __cplusplus
}
#endif

