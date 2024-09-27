/*
��Ȩ���� (c) ��Ϊ�������޹�˾ 2012-2018

�򵥵Ķ��ƻ�blob���ͱ����㷨��
ֻҪ�����������������ݣ�������ݾͻ������blob�Ĳ�������

*/
#include "../common/PCommon.h"

#ifdef __cplusplus
extern "C" {
#endif

int CustomBlobGetCount(SElement *pElement)
{
    ASSERT_NULL(pElement);
    int count = g_customBlobTableCount;

    if (count == 0)
    {
        return 0;
    }
        
    return MAX_COUNT / 2;
}

char* CustomBlobGetValue(SElement *pElement, int pos)
{
    int count = g_customBlobTableCount;
    int llvmLen;

    ASSERT_NULL(pElement);

    // ��û��ֵ��������
    if (count == 0)
    {
        return SetElementOriginalValue(pElement);
    }

    // ���������ȡһ��
    size_t idx = RAND_32() % count;

    llvmLen = g_customBlobTableLen[idx];

    // ���룬���ǣ����������滻
    int isInsert = RAND_32() % 3;

    // 0 Insert 1 Overwrite 2 replace
    MagicGetValue(pElement, (char*)g_customBlobTable[idx], llvmLen, isInsert);
    
    return pElement->para.value;
}

int CustomBlobGetIsSupport(SElement *pElement)
{
    ASSERT_NULL(pElement);
    // ֻҪ��Blob��֧��
    if ((pElement->para.type == ENUM_BLOB) 
        || (pElement->para.type == ENUM_FIXBLOB))
    {
        return ENUM_YES;
    }

    return ENUM_NO;
}

const struct MutaterGroup g_customBlobGroup = {
    "CustomBlob",
    CustomBlobGetCount,
    CustomBlobGetValue,
    CustomBlobGetIsSupport,
    1
};

void InitCustomBlob(void)
{
    RegisterMutater(&g_customBlobGroup, ENUM_CUSTOM_BLOB);
}

#ifdef __cplusplus
}
#endif