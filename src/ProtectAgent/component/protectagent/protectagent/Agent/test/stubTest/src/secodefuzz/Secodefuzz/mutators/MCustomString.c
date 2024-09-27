/*
��Ȩ���� (c) ��Ϊ�������޹�˾ 2012-2018

�򵥵Ķ��ƻ�string���ͱ����㷨��
ֻҪ�ڱ����������ݣ�������ݾͻ������string����Ԫ�صĲ�������


*/
#include "../common/PCommon.h"

#ifdef __cplusplus
extern "C" {
#endif

int CustomStringGetCount(SElement *pElement)
{
    ASSERT_NULL(pElement);
    int count = g_customStringTableCount;

    if (count == 0)
    {
        return 0;
    }
    
    return MAX_COUNT / 2;
}

char* CustomStringGetValue(SElement *pElement, int pos)
{
    int count = g_customStringTableCount;
    int llvmLen;

    ASSERT_NULL(pElement);

    // ��û��ֵ�����
    if (count == 0)
    {
        return SetElementOriginalValue(pElement);
    }

    // ���������ȡһ��
    size_t idx = RAND_32() % count;

    llvmLen = Instrlen(g_customStringTable[idx]);

    // һ�����/0
    if (RAND_32() % 2)
    {
        llvmLen = llvmLen + 1;
    }
    
    int isInsert = RAND_32() % 3;

    // 0 Insert 1 Overwrite 2 replace
    MagicGetValue(pElement, (char*)g_customStringTable[idx], llvmLen, isInsert);

    return pElement->para.value;
}

int CustomStringGetIsSupport(SElement *pElement)
{
    ASSERT_NULL(pElement);
    // ֻҪ���ַ�����֧��
    if (pElement->para.type == ENUM_STRING)
    {
        return ENUM_YES;
    }

    return ENUM_NO;
}

const struct MutaterGroup g_customStringGroup = {
    "CustomString",
    CustomStringGetCount,
    CustomStringGetValue,
    CustomStringGetIsSupport,
    1
};

void InitCustomString(void)
{
    RegisterMutater(&g_customStringGroup, ENUM_CUSTOM_STRING);
}

#ifdef __cplusplus
}
#endif