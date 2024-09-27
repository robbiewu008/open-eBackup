/*
��Ȩ���� (c) ��Ϊ�������޹�˾ 2012-2018


ԭ��:					ԭʼ���ݵ�ĳ��byte��ֵ�����

����:					���Ȳ���

����:					byte������256��MAXCOUNT*4����Сֵ

֧����������: 	�г�ֵ,ö�������

*/
#include "../common/PCommon.h"

#ifdef __cplusplus
extern "C" {
#endif

int DataElementByteRandomGetCount(SElement *pElement)
{
    ASSERT_NULL(pElement);

    if (pElement->inLen == 0)
    {
        return 0;
    }

    // ����֧��õı����ֶΣ���˱���������������
    return MAX_COUNT * 4;
}

char *DataElementByteRandomGetValue(SElement *pElement, int pos)
{ 
    int inLen;
    int start;  

    ASSERT_NULL(pElement);

    inLen = (int)(pElement->inLen / 8);

    // ����õ� λ��,��Ϊ�г�ֵ�����Բ��ÿ���in_len=0
    start = RAND_RANGE(0, inLen - 1);

    SetElementInitoutBufEx(pElement, inLen);

    HwMemcpy(pElement->para.value, pElement->inBuf, inLen); 

    //һ������ר��byte����������ĸ��ʸ���һЩ
    if (RAND_BOOL())
    {
        pElement->para.value[start] = RAND_BYTE();
    }
    else
    {
        pElement->para.value[start] = g_specialByte[RAND_RANGE(0, g_specialByteLen - 1)];
    }

    return pElement->para.value;
}

int DataElementByteRandomGetIsSupport(SElement *pElement)
{
    ASSERT_NULL(pElement);

    // ö�ٲ�֧��
    if (InGetTypeIsEnumOrRange(pElement->para.type) == ENUM_YES)
    {
        return ENUM_NO;
    }

    if (pElement->para.type == ENUM_STRING_NUM)
    {
        return ENUM_NO;
    }

    // self�����㷨�����Σ���Ҫ���Լ���
    if (pElement->para.type == ENUM_TSELF)
    {
        return ENUM_NO;
    }
    
    // �г�ʼֵ�Ŀɱ�����
    if (pElement->isHasInitValue == ENUM_YES)
    {
        return ENUM_YES;
    }

    return ENUM_NO;
}

const struct MutaterGroup g_dataElementByteRandomGroup = {
    "DataElementByteRandom",
    DataElementByteRandomGetCount,
    DataElementByteRandomGetValue,
    DataElementByteRandomGetIsSupport,
    1
};

void InitDataElementByteRandom(void)
{
    RegisterMutater(&g_dataElementByteRandomGroup, ENUM_DATAELEMENT_BYTE_RANDOM);
}

#ifdef __cplusplus
}
#endif

