/*
��Ȩ���� (c) ��Ϊ�������޹�˾ 2012-2018


constraint��Ҫ���ڽ���bin��ʱ����飬��token���ò��

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
