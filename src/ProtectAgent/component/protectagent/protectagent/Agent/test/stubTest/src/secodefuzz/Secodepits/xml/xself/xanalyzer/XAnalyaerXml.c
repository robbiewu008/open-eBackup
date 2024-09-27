/*
版权所有 (c) 华为技术有限公司 2012-2018


json
参考
https://www.cnblogs.com/secondtononewe/p/6047029.html
*/

#include "../../XML.h"


#ifdef _WIN32
#include<ws2tcpip.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

hw_xmlDocPtr g_tempXml[1000];
int g_tempXmlNumber = 0;

static void DoXmlAnalyzerMutator(hw_xmlNodePtr root, hw_xmlNodePtr rootMutator)
{

    hw_xmlNodePtr child = NULL;
    hw_xmlNodePtr childMutator = NULL;
    child = root->children;  
    childMutator = rootMutator->children;  

    while (child != NULL) 
    {  
        if (child->children != NULL)
        {
            DoXmlAnalyzerMutator(child, childMutator);
        }

        if (child->type == XML_TEXT_NODE)
        {
            char* value  = DT_SetGetString(&g_Element[g_tempElementCount], strlen((char*)child->content) + 1, g_maxOutOfMutator, (char*)child->content);

            if (childMutator->content)
            {
                free(childMutator->content);
            }

            childMutator->content = malloc(DT_GET_MutatedValueLen(&g_Element[g_tempElementCount]));
            memcpy(childMutator->content, value, DT_GET_MutatedValueLen(&g_Element[g_tempElementCount]));

            g_tempElementCount++;
        }

        child = child->next;  
        childMutator = childMutator->next;  
    }

    return;
}

void DoXmlAnalyzer(SBinElement* temp)
{
    SXMLElement* tempXml = temp->mutatorElement->xmlElement;

    if(tempXml->analyzerIsInit == 0)
    {
        tempXml->analyzerIsInit = 1;
        tempXml->analyzerValue1= (char*)HW1xmlParseMemory(temp->defaultStringValue, temp->defaultLength);
        tempXml->analyzerValue2= (char*)HW1xmlParseMemory(temp->defaultStringValue, temp->defaultLength);
        g_tempXml[g_tempXmlNumber++] = (hw_xmlDocPtr)tempXml->analyzerValue1;
        g_tempXml[g_tempXmlNumber++] = (hw_xmlDocPtr)tempXml->analyzerValue2;
    }

    DoXmlAnalyzerMutator((hw_xmlNodePtr) tempXml->analyzerValue1, (hw_xmlNodePtr)tempXml->analyzerValue2);

    char* buf = NULL;
    int size = 0;

    HW1xmlDocDumpMemory((hw_xmlDocPtr)tempXml->analyzerValue2, (hw_xmlChar**)&buf, &size);

    int len = strlen(buf) + 1;
    g_onerunMemory[g_onerunMemoryCount++] = buf;

    temp->mutaterValue = buf;
    temp->mutaterLength = len;

    if (tempXml->isnullTerminated != 1)
    {
        if (temp->mutaterLength > 0)
        {            
            temp->mutaterLength = temp->mutaterLength - 1;
        }
    }
} 

void XmlAnalyzerClean(void)
{
    int i = 0;
    for (i = 0; i < g_tempXmlNumber; i++)
    {
        HW1xmlFreeDoc(g_tempXml[i]); 
    }
    g_tempXmlNumber = 0;
}

#ifdef __cplusplus
}
#endif
