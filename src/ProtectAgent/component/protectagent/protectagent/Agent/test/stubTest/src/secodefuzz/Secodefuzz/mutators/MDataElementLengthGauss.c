/*
��Ȩ���� (c) ��Ϊ�������޹�˾ 2012-2018


ԭ��:					�������ݳ���  ��
						�����㷨Ϊ��˹���죬
						ʹ�����ݱ������

					
����:					���������ֵ��1֮��

����:					���������ȿ�ƽ����MAXCOUNT/5����Сֵ

֧����������: 	�г�ʼֵ�Ŀɱ�����
*/
#include "../common/PCommon.h"

#ifdef __cplusplus
extern "C" {
#endif

static int DataElementLengthGaussGetCount(SElement *pElement)
{
    ASSERT_NULL(pElement);
    return MIN(Insqrt(pElement->para.maxLen), MAX_COUNT / 100);
}

static char* DataElementLengthGaussGetValue(SElement *pElement, int pos)
{
    int i;
    int outLen;
    int inLen;
    ASSERT_NULL(pElement);

    inLen = (int)(pElement->inLen / 8);
    outLen = GaussRandU32(pos % InGetBitNumber(pElement->para.maxLen));

    if (outLen > pElement->para.maxLen)
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

static int DataElementLengthGaussGetIsSupport(SElement *pElement)
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

const struct MutaterGroup g_dataElementLengthGaussGroup = {
    .name           = "DataElementLengthGauss",
    .getCount       = DataElementLengthGaussGetCount,
    .getValue       = DataElementLengthGaussGetValue,
    .getIsSupport   = DataElementLengthGaussGetIsSupport,
};

void InitDataElementLengthGauss(void)
{
    RegisterMutater(&g_dataElementLengthGaussGroup, ENUM_DATAELEMENT_LENGTH_GAUSS);
}

#ifdef __cplusplus
}
#endif

