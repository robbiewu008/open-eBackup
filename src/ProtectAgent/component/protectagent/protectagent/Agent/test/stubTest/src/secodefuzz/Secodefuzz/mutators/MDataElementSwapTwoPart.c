/*
��Ȩ���� (c) ��Ϊ�������޹�˾ 2012-2018


ԭ��:					���������������������
						�����������������������
						

����:					���Ȳ���

����:					MAX_COUNT

֧����������: 	�г�ʼֵ����������,ENUM_STRING,ENUM_BLOB,ENUM_FIXBLOB
*/
#include "../common/PCommon.h"

#ifdef __cplusplus
extern "C" {
#endif

static int DataElementSwapTwoPartGetCount(SElement *pElement)
{
    ASSERT_NULL(pElement);
    return MAX_COUNT;
}

// ����������������
static char* DataElementSwapNoNearTwoPartGetValue(SElement *pElement, int pos)
{
    int inLen;
    int partLen1;
    int partLen2;
    int start1;
    int start2;
    ASSERT_NULL(pElement);

    inLen = (int)(pElement->inLen / 8);

    // �õ���һ���ֳ���
    partLen1 = GaussRandU32(RAND_32() % InGetBitNumber(inLen));

    // �õ��ڶ����ֳ���
    partLen2 = GaussRandU32(RAND_32() % InGetBitNumber(inLen));

    if ((partLen1 + partLen2) > inLen)
    {
        return SetElementOriginalValue(pElement);
    }

    // �����һ���ֵ���ʼλ��
    start1 = RAND_RANGE(0, inLen  - (partLen1 + partLen2));

    // ����ڶ����ֵ���ʼλ��
    start2 = RAND_RANGE(0, inLen  - (partLen1 + partLen2 + start1)) + start1 + partLen1;
    

    SetElementInitoutBufEx(pElement, inLen);

    // ������ʼ
    HwMemcpy(pElement->para.value, pElement->inBuf, start1); 

    // �����ڶ�����
    HwMemcpy(pElement->para.value + start1, pElement->inBuf + start2, partLen2); 

    // �����м�
    HwMemcpy(pElement->para.value + start1 + partLen2, 
        pElement->inBuf + start1 + partLen1, start2 - start1 - partLen1); 

    // ������һ����
    HwMemcpy(pElement->para.value + start2 + partLen2 - partLen1, pElement->inBuf + start1, partLen1); 

    // ����ʣ�µ�
    HwMemcpy(pElement->para.value + start2 + partLen2, 
        pElement->inBuf + start2 + partLen2, inLen - (start2 + partLen2)); 

    return pElement->para.value;
}

// ��������������
static char* DataElementSwapNearTwoPartGetValue(SElement *pElement, int pos)
{
    int inLen;
    int partLen1;
    int partLen2;
    int start;
    ASSERT_NULL(pElement);

    inLen = (int)(pElement->inLen / 8);

    // �õ���һ���ֳ���
    partLen1 = GaussRandU32(RAND_32() % InGetBitNumber(inLen));

    // �õ��ڶ����ֳ���
    partLen2 = GaussRandU32(RAND_32() % InGetBitNumber(inLen));

    if ((partLen1 + partLen2) > inLen)
    {
        return SetElementOriginalValue(pElement);
    }

    // ����õ�Ҫ��������ʼλ��
    start = RAND_RANGE(0, inLen  - (partLen1 + partLen2));

    SetElementInitoutBufEx(pElement, inLen);

    // ������ʼ
    HwMemcpy(pElement->para.value, pElement->inBuf, start); 

    // �����ڶ�����
    HwMemcpy(pElement->para.value + start, pElement->inBuf + start + partLen1, partLen2); 

    // ������һ����
    HwMemcpy(pElement->para.value + start + partLen2, pElement->inBuf + start, partLen1); 

    // ����ʣ�µ�
    HwMemcpy(pElement->para.value + start + partLen1 + partLen2, 
        pElement->inBuf + start + partLen1 + partLen2, inLen - (start + partLen1 + partLen2)); 

    return pElement->para.value;
}

static char* DataElementSwapTwoPartGetValue(SElement *pElement, int pos)
{
    if (RAND_32() % 2 == 0)
    {
        return DataElementSwapNearTwoPartGetValue(pElement, pos);
    }
    else
    {
        return DataElementSwapNoNearTwoPartGetValue(pElement, pos);
    }
}

static int DataElementSwapTwoPartGetIsSupport(SElement *pElement)
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

const struct MutaterGroup g_dataElementSwapTwoPartGroup = {
    .name           = "DataElementSwapTwoPart",
    .getCount       = DataElementSwapTwoPartGetCount,
    .getValue       = DataElementSwapTwoPartGetValue,
    .getIsSupport   = DataElementSwapTwoPartGetIsSupport,
};

void InitDataElementSwapTwoPart(void)
{
    RegisterMutater(&g_dataElementSwapTwoPartGroup, ENUM_DATAELEMENT_SWAP_TWO_PART);
}

#ifdef __cplusplus
}
#endif

