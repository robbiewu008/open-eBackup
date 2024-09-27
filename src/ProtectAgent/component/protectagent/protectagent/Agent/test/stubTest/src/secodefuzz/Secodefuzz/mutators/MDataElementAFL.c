/*
��Ȩ���� (c) ��Ϊ�������޹�˾ 2012-2018

ԭ��:					�����㷨����AFL

����:					0��ԭʼ����֮��

����:					�����������byte������8

֧����������: 	�г�ʼֵ�Ŀɱ���������

*/
#include "../common/PCommon.h"

#ifdef __cplusplus
extern "C" {
#endif

int DataElementAFLGetCount(SElement *pElement)
{
    ASSERT_NULL(pElement);

    // ����֧�����ֶΣ���˱���������������
    return 0;
}

char *DataElementAFLGetValue(SElement *pElement, int pos)
{
    return NULL;
}

int DataElementAFLGetIsSupport(SElement *pElement)
{
    ASSERT_NULL(pElement);
    // �г�ʼֵ�Ŀɱ�����
    if (((pElement->para.type == ENUM_BLOB) 
        || (pElement->para.type == ENUM_FIXBLOB))
        && (pElement->isHasInitValue == ENUM_YES))
    {
        return ENUM_YES;
    }

    return ENUM_NO;
}

const struct MutaterGroup g_dataElementAFLGroup = {
    "DataElementAFL",
    DataElementAFLGetCount,
    DataElementAFLGetValue,
    DataElementAFLGetIsSupport,
    1
};

void InitDataElementAFL(void)
{
    RegisterMutater(&g_dataElementAFLGroup, ENUM_DATAELEMENT_AFL);
}

#ifdef __cplusplus
}
#endif