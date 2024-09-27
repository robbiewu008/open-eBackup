/*
��Ȩ���� (c) ��Ϊ�������޹�˾ 2012-2018

ԭ��:					�������ݳ���  ��
						�����㷨Ϊ����ٽ�ֵ���죬
						ʹ�����ݱ������


����:					���������ֵ��1֮��

����������:		����������Ϊ���������ȵ�bit���

֧����������: 	�г�ʼֵ�Ŀɱ�����
*/
#include "../common/PCommon.h"

#ifdef __cplusplus
extern "C" {
#endif

static int DataElementLengthEdgeCaseGetCount(SElement *pElement)
{
    u32 count;

    ASSERT_NULL(pElement);
    
    count = InGetBitNumber(pElement->para.maxLen);

    if (count == 0)
    {
        return 0;
    }

    return count * 2 - 1;
}

static char* DataElementLengthEdgeCaseGetValue(SElement *pElement, int pos)
{
    int i;
    int inLen;
    int outLen;
    ASSERT_NULL(pElement);
    
    inLen = (int)(pElement->inLen / 8);
    outLen = g_edgeCaseTable[pos];

    if ((outLen > pElement->para.maxLen) || (outLen == 0))
    {
        return SetElementOriginalValue(pElement);
    }

    SetElementInitoutBufEx(pElement, outLen);

    for (i = 0; i < outLen / inLen; i++)
    { 
        HwMemcpy(pElement->para.value + i * inLen, pElement->inBuf, inLen); 
    }

    // ����ʣ�µ�
    HwMemcpy(pElement->para.value + i * inLen, pElement->inBuf, outLen - (outLen / inLen) * inLen); 

    return pElement->para.value;
}

static int DataElementLengthEdgeCaseGetIsSupport(SElement *pElement)
{
    ASSERT_NULL(pElement);
    // �г�ʼֵ�Ŀɱ�����
    if (((pElement->para.type == ENUM_STRING)
        || (pElement->para.type == ENUM_BLOB))
        && (pElement->isHasInitValue == ENUM_YES))
    {
        return ENUM_YES;
    }

    return ENUM_NO;
}

const struct MutaterGroup g_dataElementLengthEdgeCaseGroup = {
    "DataElementLengthEdgeCase",
    DataElementLengthEdgeCaseGetCount,
    DataElementLengthEdgeCaseGetValue,
    DataElementLengthEdgeCaseGetIsSupport,
    1
};

void InitDataElementLengthEdgeCase(void)
{
    RegisterMutater(&g_dataElementLengthEdgeCaseGroup, ENUM_DATAELEMENT_LENGTH_EDGE_CASE);
}

#ifdef __cplusplus
}
#endif