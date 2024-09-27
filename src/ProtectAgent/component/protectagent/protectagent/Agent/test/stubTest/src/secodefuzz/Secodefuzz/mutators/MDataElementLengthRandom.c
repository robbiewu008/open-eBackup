/*
��Ȩ���� (c) ��Ϊ�������޹�˾ 2012-2018

ԭ��:					�������ݳ���  ��
						�����㷨���
						ʹ�����ݱ������
						

����:					���������ֵ��1֮��

����:					���������ȿ�ƽ����MAXCOUNT/5����Сֵ

֧����������: 	�г�ʼֵ�Ŀɱ�����
*/
#include "../common/PCommon.h"

#ifdef __cplusplus
extern "C" {
#endif

static int DataElementLengthRandomGetCount(SElement *pElement)
{
    ASSERT_NULL(pElement);
    return MIN(Insqrt(pElement->para.maxLen), MAX_COUNT / 100);
}

static char* DataElementLengthRandomGetValue(SElement *pElement, int pos)
{
    int i;
    int outLen;
    int inLen;
    ASSERT_NULL(pElement);

    inLen = (int)(pElement->inLen / 8);
    outLen = RAND_RANGE(1, pElement->para.maxLen);

    SetElementInitoutBufEx(pElement, outLen);

    for (i = 0; i < outLen / inLen; i++)
    { 
        HwMemcpy(pElement->para.value + i * inLen, pElement->inBuf, inLen); 
    }

    // ����ʣ�µ�
    HwMemcpy(pElement->para.value + i * inLen, pElement->inBuf, outLen - (outLen / inLen) * inLen); 

    return pElement->para.value;
}

static int DataElementLengthRandomGetIsSupport(SElement *pElement)
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

const struct MutaterGroup g_dataElementLengthRandomGroup = {
    .name           = "DataElementLengthRandom",
    .getCount       = DataElementLengthRandomGetCount,
    .getValue       = DataElementLengthRandomGetValue,
    .getIsSupport   = DataElementLengthRandomGetIsSupport,
};

void InitDataElementLengthRandom(void)
{
    RegisterMutater(&g_dataElementLengthRandomGroup, ENUM_DATAELEMENT_LENGTH_RANDOM);
}

#ifdef __cplusplus
}
#endif