/*
��Ȩ���� (c) ��Ϊ�������޹�˾ 2012-2018


������������ģ�ͣ�����xml�ṹ�壬ÿ���ڵ�һһ��Ӧ

// �õ�xmlԪ�أ� ������������ģ�ͣ�   ����xmlNodePtr�����ṹ��������g_xml_element����ֵ��_private

// ���е�xml�﷨�������ֹ��ȫ���⿪����g_xml_element

// ֮��Ĵ����������xml�﷨���Ա��Ժ��滻�﷨�滻����ģ�ͽṹ����
*/
#ifndef Open_source

#include "../XML.h"

#ifdef __cplusplus
extern "C" {
#endif

static int g_nameCountT = 1;

void ParseXmlTestModel(hw_xmlNodePtr root, hw_xmlNodePtr xmlParent)
{
    hw_xmlNodePtr child; 
    child = xmlParent->children;

    SXMLElement* temp;

    while (child != NULL) 
    {  
        // test����ֱ�Ӻ��ԣ���ʱû������ɶ�ô�
        if (strcmp((char*)child->name, "text") == 0)
        {
            child = child->next; 
            continue;
        }

        // ����xmlԪ���ڴ棬ֱ�ӹҽ���node����
        temp = malloc(sizeof(SXMLElement));
        g_testcaseMemory[g_testcaseMemoryCount++] = (char*)temp;
        Hw1Memset((char*)temp, 0, sizeof(SXMLElement));

        // ָ�뻥ָ��������������
        temp->xmlNode = child;
        child->_private = temp;

        /*******************************
        ���ԵĶ��������ϱ�
        ********************************/
        temp->name = XmlGetGetProp(child, "name");
        temp->typeName = (char*)child->name;

        // û�����֣�����Ĭ�ϵ����֣����ܺ��ֹ������ĳ�ͻ��������
        // Ĭ�����ֲ�ȡԪ�����ͼ����ֵķ�ʽ
        if (temp->name == NULL)
        {
            temp->name = malloc(40);
            g_testcaseMemory[g_testcaseMemoryCount++] = temp->name;
            sprintf(temp->name, "%s_%03d", temp->typeName, g_nameCountT++);
        }

        if (g_onOffDebugParseXml)
        {      
            printf("	----Start Parse type=%s ----name=\"%s\"  \n", temp->typeName, temp->name);  
        }

        /*******************************
        ���ԵĶ����������
        *******************************/
        if (strcmp((char*)child->name, "StateModel") == 0)
        {
            temp->stateModelRef = XmlGetProperty(child, "ref");

            if (g_onOffDebugParseXml)
            {         
                if (temp->stateModelRef)
                {            
                    printf("		StateModel_ref	= %s \n", temp->stateModelRef);
                }
            }
        }

        if (strcmp((char*)child->name, "Publisher") == 0)
        {
            temp->publisherName = XmlGetProperty(child, "name");
            temp->publisherClass = XmlGetProperty(child, "class");

            if (g_onOffDebugParseXml)
            {         
                if (temp->publisherName)
                {            
                    printf("		Publisher_name	= %s \n", temp->publisherName);
                }
            }

            if (g_onOffDebugParseXml)
            {         
                if (temp->publisherClass)
                {            
                    printf("		Publisher_class	= %s \n", temp->publisherClass);
                }
            }


            hw_xmlNodePtr sunzi = child->children;
            int i = 0;
            while (sunzi != NULL)
            {
                if (strcmp((char*)sunzi->name, "Param") != 0)
                {
                    sunzi = sunzi->next; 
                    continue;
                }

                temp->publisherParamName[i] = XmlGetGetProp(sunzi, "name");
                temp->publisherParamValue[i] = XmlGetGetProp(sunzi, "value");

                i++;
                sunzi = sunzi->next;
            }
        }

        if (g_onOffDebugParseXml)
        {      
            printf("	-----------End  type=%s ----name=\"%s\"  \n", temp->typeName, temp->name);  
        }

        child = child->next;  
    }
}

#ifdef __cplusplus
}
#endif

#endif // Open_source
