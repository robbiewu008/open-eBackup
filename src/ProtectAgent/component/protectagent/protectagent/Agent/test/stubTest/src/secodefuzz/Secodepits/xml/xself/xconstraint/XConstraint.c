/*
��Ȩ���� (c) ��Ϊ�������޹�˾ 2012-2018

ʹ�����constraint��ʶ��Ԫ�ز��ܴ���"
*/
#include "../../XML.h"

#ifdef __cplusplus
extern "C" {
#endif

int DoNoMaoHao(SXMLElement *tempXml)
{
    char *rel = MyMemmem("\"", 1 ,(char*)tempXml->stringValue , tempXml->length); 
    if (rel ==NULL)
    {
        return R_Yes;
    }
    return R_No;
}

#ifdef __cplusplus
}
#endif
