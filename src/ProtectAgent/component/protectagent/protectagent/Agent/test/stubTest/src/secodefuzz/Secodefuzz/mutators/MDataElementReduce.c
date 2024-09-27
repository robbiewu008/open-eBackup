/*
��Ȩ���� (c) ��Ϊ�������޹�˾ 2012-2018


ԭ��:					ԭʼ���ݵ����������������byte�ᱻɾ��.

����:					0��ԭʼ����֮��

����:					byte������8��MAXCOUNT����Сֵ

֧����������: 	�г�ʼֵ�Ŀɱ�����

*/
#include "../common/PCommon.h"

#ifdef __cplusplus
extern "C" {
#endif

int DataElementReduceGetCount(SElement *pElement)
{
    ASSERT_NULL(pElement);

    return MAX_COUNT/5;
}

char *DataElementReduceGetValue(SElement *pElement, int pos)
{
    int len;
    int start;
    int deleteLen;

    ASSERT_NULL(pElement);

    len = (int)(pElement->inLen / 8);
    
    // �õ�Ҫɾ����byte����
    deleteLen = GaussRandU32(pos % InGetBitNumber(len));
    if (len == deleteLen)
    {
        return SetElementOriginalValue(pElement);
    }

    // ����õ�Ҫɾ����λ��
    start = RAND_RANGE(0, len - deleteLen);

    SetElementInitoutBufEx(pElement, len - deleteLen);

    HwMemcpy(pElement->para.value, pElement->inBuf, start);
    HwMemcpy(pElement->para.value + start, pElement->inBuf + deleteLen + start, len - deleteLen - start);

    return pElement->para.value;
}

int DataElementReduceGetIsSupport(SElement *pElement)
{
    ASSERT_NULL(pElement);
    // �г�ʼֵ�Ŀɱ�����
    if (((pElement->para.type == ENUM_STRING)
        || (pElement->para.type == ENUM_BLOB)
        || (pElement->para.type == ENUM_FIXBLOB))
        && (pElement->isHasInitValue == ENUM_YES))
    {
        return ENUM_YES;
    }

    return ENUM_NO;
}

const struct MutaterGroup g_dataElementReduceGroup = {
    "DataElementReduce",
    DataElementReduceGetCount,
    DataElementReduceGetValue,
    DataElementReduceGetIsSupport,
    1
};

void InitDataElementReduce(void)
{
    RegisterMutater(&g_dataElementReduceGroup, ENUM_DATAELEMENT_REDUCE);
}

#ifdef __cplusplus
}
#endif

