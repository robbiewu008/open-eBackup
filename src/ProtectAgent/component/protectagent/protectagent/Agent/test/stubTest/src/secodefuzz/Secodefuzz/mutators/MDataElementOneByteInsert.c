/*
��Ȩ���� (c) ��Ϊ�������޹�˾ 2012-2018


ԭ��:					ԭʼ���ݵ����λ�ñ��������byte

����:					ԭʼ����+1

����:					MAX_COUNT

֧����������: 	�ɱ䳤��������

*/
#include "../common/PCommon.h"

#ifdef __cplusplus
extern "C" {
#endif

int DataElementOneByteInsertGetCount(SElement *pElement)
{
    ASSERT_NULL(pElement);

    // ����֧�����ֶΣ���˱���������������
    return MAX_COUNT*2;
}

char *DataElementOneByteInsertGetValue(SElement *pElement, int pos)
{
    int inLen;
    int start;
    ASSERT_NULL(pElement);

    inLen = (int)(pElement->inLen / 8);

    // ��󳤶��㷨Ҫ�����㷨��������
    if ((inLen + 1) > pElement->para.maxLen)
    {
        return SetElementOriginalValue(pElement);
    }

    // ���㷨֧���޳�ʼֵԪ��

    start = RAND_RANGE(0, inLen);

    SetElementInitoutBufEx(pElement, inLen + 1);

    HwMemcpy(pElement->para.value, pElement->inBuf, start);

    //һ������ר��byte����������ĸ��ʸ���һЩ
    if (RAND_BOOL())
    {
        pElement->para.value[start] = RAND_BYTE();
    }
    else
    {
        pElement->para.value[start] = g_specialByte[RAND_RANGE(0, g_specialByteLen - 1)];
    }
    HwMemcpy(pElement->para.value + start + 1, pElement->inBuf + start, inLen - start);

    return pElement->para.value;
}

int DataElementOneByteInsertGetIsSupport(SElement *pElement)
{
    ASSERT_NULL(pElement);
    // �ɱ�����
    if ((pElement->para.type == ENUM_STRING) 
        || (pElement->para.type == ENUM_BLOB) 
        || (pElement->para.type == ENUM_FIXBLOB))
    {
        return ENUM_YES;
    }

    return ENUM_NO;
}

const struct MutaterGroup g_dataElementOneByteInsertGroup = {
    "DataElementOneByteInsert",
    DataElementOneByteInsertGetCount,
    DataElementOneByteInsertGetValue,
    DataElementOneByteInsertGetIsSupport,
    1
};

void InitDataElementOneByteInsert(void)
{
    RegisterMutater(&g_dataElementOneByteInsertGroup, ENUM_DATAELEMENT_ONE_BYTE_INSERT);
}

#ifdef __cplusplus
}
#endif

