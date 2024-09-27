/*
��Ȩ���� (c) ��Ϊ�������޹�˾ 2012-2018

ԭ��:					�������ĳ�������ݣ�
						���Ƹ������,����1�εĸ���Ϊ25%
						

����:					���ֵ��ԭ����֮����죬

����:					MAX_COUNT

֧����������: 	�г�ʼֵ�Ŀɱ�����
*/
#include "../common/PCommon.h"

#ifdef __cplusplus
extern "C" {
#endif

static int DataElementLengthRepeatPartGetCount(SElement *pElement)
{
    ASSERT_NULL(pElement);
    return MAX_COUNT/10;
}

static char* DataElementLengthRepeatPartGetValue(SElement *pElement, int pos)
{
    int i;
    int inLen;
    int repeatLen;
    int count;
    int start;
    ASSERT_NULL(pElement);

    inLen = (int)(pElement->inLen / 8);

    // �õ�Ҫ���Ƶ�byte����
    repeatLen = GaussRandU32(RAND_32() % InGetBitNumber(inLen));

    // ���ٸ���1byte�ĸ�������Ϊ����Ƚ�С��������ĸ�����̫��
    if (repeatLen == 1)
    {
        repeatLen = GaussRandU32(RAND_32() % InGetBitNumber(inLen));
    }

    // ����õ�Ҫ�������ʼλ��
    start = RAND_RANGE(0, inLen - repeatLen);

    // ����õ�Ҫ���Ƶĸ���
    count = GaussRandU32(RAND_32() % InGetBitNumber(pElement->para.maxLen / repeatLen)); 

    // ���Ӹ���1�εĸ���Ϊ%25
    if (RAND_32() % 4 == 0)
    {
        count = 1;
    }

    if ((inLen + count * repeatLen) > pElement->para.maxLen)
    {
        return SetElementOriginalValue(pElement);
    }

    SetElementInitoutBufEx(pElement, (inLen + count * repeatLen));

    // ������ʼ
    HwMemcpy(pElement->para.value , pElement->inBuf, start); 

    for (i = 0; i < count; i++)
    { 
        HwMemcpy(pElement->para.value + start + i * repeatLen, pElement->inBuf + start, repeatLen); 
    }

    // ����ʣ�µ�
    HwMemcpy(pElement->para.value + start + i * repeatLen, pElement->inBuf + start, inLen - start); 

    return pElement->para.value;
}

static int DataElementLengthRepeatPartGetIsSupport(SElement *pElement)
{
    ASSERT_NULL(pElement);
    // �г�ʼֵ�Ŀɱ�����
    if (((pElement->para.type == ENUM_STRING)
        || (pElement->para.type == ENUM_BLOB) 
        || (pElement->para.type == ENUM_FIXBLOB))
        &&(pElement->isHasInitValue == ENUM_YES))
    {
        return ENUM_YES;
    }

    return ENUM_NO;
}

const struct MutaterGroup g_dataElementLengthRepeatPartGroup = {
    .name           = "DataElementLengthRepeatPart",
    .getCount       = DataElementLengthRepeatPartGetCount,
    .getValue       = DataElementLengthRepeatPartGetValue,
    .getIsSupport   = DataElementLengthRepeatPartGetIsSupport,
};

void InitDataElementLengthRepeatPart(void)
{
    RegisterMutater(&g_dataElementLengthRepeatPartGroup, ENUM_DATAELEMENT_LENGTH_REPEAT_PART);
}

#ifdef __cplusplus
}
#endif