/*
��Ȩ���� (c) ��Ϊ�������޹�˾ 2012-2018


ԭ��:						������������������ַ����������룬���ǣ��滻Ԫ��
							ֻҪ�ڱ����������ݣ�������ݾͻ�����ڲ�������						

����:						0����󳤶�֮��

����:						MAX_COUNT

֧����������: 		string,Blob,FixBlob


*/
#include "../common/PCommon.h"

#ifdef __cplusplus
extern "C" {
#endif

int DataElementStringStaticGetCount(SElement *pElement)
{
    ASSERT_NULL(pElement);
    
    int count = g_stringStaticTableLen;
    if (count == 0)
    {
        return 0;
    }
    
    return MAX_COUNT;
}

char* DataElementStringStaticGetValue(SElement *pElement, int pos)
{
    int count = g_stringStaticTableLen;
    int llvmLen;

    ASSERT_NULL(pElement);

    // ��û��ֵ�����
    if (count == 0)
    {
        return SetElementOriginalValue(pElement);
    }

    // ���������ȡһ��
    size_t idx  = RAND_32() % count;

    llvmLen = Instrlen(g_stringStaticTable[idx]);
    if (llvmLen == 0)
    {
        return SetElementOriginalValue(pElement);
    }

    // һ�����/0
    if (RAND_32() % 2)
    {
        llvmLen = llvmLen + 1;
    }
    
    int isInsert = RAND_32() % 3;
    
    // 0 Insert 1 Overwrite 2 replace
    MagicGetValue(pElement, (char*)g_stringStaticTable[idx], llvmLen, isInsert);

    return pElement->para.value;
}

int DataElementStringStaticGetIsSupport(SElement *pElement)
{
    ASSERT_NULL(pElement);
    // ����fixblob,��Ϊ�㷨�����С���ȣ�����󳤶Ȼ����жϣ����Կ���֧��
    if ((pElement->para.type == ENUM_STRING)
        || (pElement->para.type == ENUM_BLOB)
        || (pElement->para.type == ENUM_FIXBLOB))
    {
        return ENUM_YES;
    }

    return ENUM_NO;
}

const struct MutaterGroup g_dataElementStringStaticGroup = {
    "DataElementStringStatic",
    DataElementStringStaticGetCount,
    DataElementStringStaticGetValue,
    DataElementStringStaticGetIsSupport,
    1
};

void InitDataElementStringStatic(void)
{
    RegisterMutater(&g_dataElementStringStaticGroup, ENUM_DATAELEMENT_STRING_STATIC);
}

#ifdef __cplusplus
}
#endif

