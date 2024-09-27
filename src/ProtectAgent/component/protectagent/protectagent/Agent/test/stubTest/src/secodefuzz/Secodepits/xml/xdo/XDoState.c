/*
��Ȩ���� (c) ��Ϊ�������޹�˾ 2012-2018

Stateʵ��
*/
#ifndef Open_source

#include "../XML.h"

#ifdef __cplusplus
extern "C" {
#endif

void DoState(SStateElement* tempAction, STestElement* tempTestElement)
{
    SStateElement *tempStateElement = tempAction->children;
    SStateElement *tempActionElement = tempStateElement->children;

    while (tempActionElement != NULL) 
    { 
        SXMLElement * tempXml = tempActionElement->xmlElement;

        tempActionElement->testElement =tempTestElement;

        if (strcmp(tempXml->actionType, "open") == 0)
        {
            DoActionOpen(tempActionElement);
        }

        if (strcmp(tempXml->actionType, "output") == 0)
        {
            DoActionOpen(tempActionElement);
            DoActionOutput(tempActionElement);
        }

        if (strcmp(tempXml->actionType, "close") == 0)
        {
            DoActionClose(tempActionElement);
        }
        tempActionElement = tempActionElement->next;
    }


    //���Ҫ������δ�رյ�publisher���ر�
    tempStateElement = tempAction->children;
    tempActionElement = tempStateElement->children;

    while (tempActionElement != NULL) 
    { 
        SXMLElement * temp_xml = tempActionElement->xmlElement;

        tempActionElement->testElement = tempTestElement;

        if (strcmp(temp_xml->actionType, "open") == 0)
        {
            DoActionClose(tempActionElement);
        }

        if (strcmp(temp_xml->actionType, "output") == 0)
        {
            DoActionClose(tempActionElement);
        }

        if (strcmp(temp_xml->actionType, "close") == 0)
        {
            DoActionClose(tempActionElement);
        }
        tempActionElement = tempActionElement->next;
    }
}

#ifdef __cplusplus
}
#endif

#endif // Open_source
