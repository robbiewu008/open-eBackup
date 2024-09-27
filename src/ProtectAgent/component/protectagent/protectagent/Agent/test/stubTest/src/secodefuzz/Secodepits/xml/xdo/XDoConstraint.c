/*
版权所有 (c) 华为技术有限公司 2012-2018


constraint主要用于解析bin的时候检验，与token作用差不多

*/

#include "../XML.h"

#ifdef __cplusplus
extern "C" {
#endif

//constraint
extern int DoNoMaoHao(SXMLElement *tempXml);

int DoConstraint(SXMLElement *tempXml)
{
    if (strcmp(tempXml->constraint, "nomaohao") == 0)
    {
        return DoNoMaoHao(tempXml);
    }
    return R_Yes;
} 

#ifdef __cplusplus
}
#endif
