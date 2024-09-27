/*
版权所有 (c) 华为技术有限公司 2012-2018

使用这个constraint标识的元素不能带有"
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
