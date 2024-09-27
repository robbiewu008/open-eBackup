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

int DataElementMagicChangeGetCount(SElement *pElement)
{
    if ((LlvmDataMemGetCount() == 0) && (LlvmDataNumberGetCount() == 0))
    {
        return 0;
    }
    
    return MAX_COUNT;
}

char* DataElementMagicChangeGetValue(SElement *pElement, int pos)
{
    int llvmLen;
    char* value =NULL;
    ASSERT_NULL(pElement);

    int temp = RAND_32() % 20;

    if (temp < 6)
    {
        s64 number = LlvmDataNumberGetValue();

        //��û��ֵ
        if (number == 0)
        {
            return SetElementOriginalValue(pElement);
        }
        
        value = (char*)(&number);
        llvmLen = 8;

        // �ҵ���ʼλ��
        int start = RAND_RANGE(0, llvmLen);
        // �ҵ��滻�ĳ���
        int len = RAND_RANGE(0, llvmLen - start);

        // 0 Insert 1 Overwrite 2 replace
        MagicGetValue(pElement, value + start, len, ENUM_CHANGE);
    }
    else
    {
        // \0���ӵ��߼��ں������
        value = LlvmDataMemGetValue(&llvmLen);

        //��û��ֵ�����߾�һ���ֽ�
        if (llvmLen <= 1)
        {
            return SetElementOriginalValue(pElement);
        }

        // �ҵ���ʼλ��
        int start = RAND_RANGE(0, llvmLen);
        // �ҵ��滻�ĳ���
        int len = RAND_RANGE(0, llvmLen - start);

        // 0 Insert 1 Overwrite 2 replace 3 change
        MagicGetValue(pElement, value + start, len, ENUM_CHANGE);
    }

    return (char *)pElement->para.value;
}

int DataElementMagicChangeGetIsSupport(SElement *pElement)
{
    ASSERT_NULL(pElement);

    // ö�ٲ�֧��
    if (InGetTypeIsEnumOrRange(pElement->para.type) == ENUM_YES)
    {
        return ENUM_NO;
    }

    if (pElement->para.type == ENUM_STRING_NUM)
    {
        return ENUM_NO;
    }

    // self�����㷨�����Σ���Ҫ���Լ���
    if (pElement->para.type == ENUM_TSELF)
    {
        return ENUM_NO;
    }
    
    // ֻҪ�г�ʼֵ����֧�֣��������������Ϳ����������?
    if (pElement->isHasInitValue == ENUM_YES)
    {
        return ENUM_YES;
    }
    
    return ENUM_NO;
}


const struct MutaterGroup g_dataElementMagicChangeGroup = {
    "DataElementMagicChange",
    DataElementMagicChangeGetCount,
    DataElementMagicChangeGetValue,
    DataElementMagicChangeGetIsSupport,
    1
};

void InitDataElementMagicChange(void)
{
    if (LlvmHookIsSupport() == 0)
    {
        return;
    }
    
    RegisterMutater(&g_dataElementMagicChangeGroup, ENUM_DATAELEMENT_MAGIC_CHANGE);
}

#ifdef __cplusplus
}
#endif

