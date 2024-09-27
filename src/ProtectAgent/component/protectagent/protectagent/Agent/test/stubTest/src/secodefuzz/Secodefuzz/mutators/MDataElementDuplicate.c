/*
��Ȩ���� (c) ��Ϊ�������޹�˾ 2012-2018


ԭ��:					ͨ������Ԫ�صĸ��������ɲ�����
						��������ʹ�ø�˹����
						

����:					ԭʼ���ݳ��ȵ�������	

����:					���������ȿ�ƽ����MAXCOUNT/5����Сֵ

֧����������: 	�г�ʼֵ�Ŀɱ�����

*/
#include "../common/PCommon.h"

#ifdef __cplusplus
extern "C" {
#endif

int DataElementDuplicateGetCount(SElement *pElement)
{
    ASSERT_NULL(pElement);
    return MIN(Insqrt(pElement->para.maxLen), MAX_COUNT / 100);
}

char *DataElementDuplicateGetValue(SElement *pElement, int pos)
{
    int i;
    int inLen;
    int times; 
    int maxTimes;
    ASSERT_NULL(pElement);
    
    inLen = (int)(pElement->inLen / 8);
    maxTimes = pElement->para.maxLen / inLen;

    // �õ��ظ�����
    times = GaussRandU32(pos % InGetBitNumber(maxTimes));

    SetElementInitoutBufEx(pElement, inLen * times);

    for (i = 0; i < times; i++)
    { 
        HwMemcpy(pElement->para.value + i * inLen, pElement->inBuf, inLen); 
    }

    return pElement->para.value;
}

int DataElementDuplicateGetIsSupport(SElement *pElement)
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

const struct MutaterGroup g_dataElementDuplicateGroup = {
    "DataElementDuplicate",
    DataElementDuplicateGetCount,
    DataElementDuplicateGetValue,
    DataElementDuplicateGetIsSupport,
    1
};

void InitDataElementDuplicate(void)
{
    RegisterMutater(&g_dataElementDuplicateGroup, ENUM_DATAELEMENT_DUPLICATE);
}

#ifdef __cplusplus
}
#endif

