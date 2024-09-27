/*
��Ȩ���� (c) ��Ϊ�������޹�˾ 2012-2018


������������ģ�ͣ�����xml�ṹ�壬ÿ���ڵ�һһ��Ӧ

//�õ�xmlԪ�أ� ������������ģ�ͣ�   ����xmlNodePtr�����ṹ��������g_xml_element����ֵ��_private

//���е�xml�﷨�������ֹ��ȫ���⿪����g_xml_element

//֮��Ĵ����������xml�﷨���Ա��Ժ��滻�﷨�滻����ģ�ͽṹ����
*/
#ifndef Open_source

#include "../XML.h"

#ifdef __cplusplus
extern "C" {
#endif

static int g_nameCountS = 1;

void ParseXmlStateModel(hw_xmlNodePtr root, hw_xmlNodePtr xmlParent)
{
    hw_xmlNodePtr child; 
    child = xmlParent->children;

    SXMLElement* temp;

    while (child != NULL) 
    {  
        //test����ֱ�Ӻ��ԣ���ʱû������ɶ�ô�
        if (strcmp((char*)child->name, "text") == 0)
        {
            child = child->next; 
            continue;
        }

        //����xmlԪ���ڴ棬ֱ�ӹҽ���node����
        temp = malloc(sizeof(SXMLElement));
        g_testcaseMemory[g_testcaseMemoryCount++] = (char*)temp;
        Hw1Memset(temp, 0, sizeof(SXMLElement));

        //ָ�뻥ָ��������������
        temp->xmlNode = child;
        child->_private = temp;

        /*******************************
        ���ԵĶ��������ϱ�
        ********************************/
        temp->name =  XmlGetGetProp(child, "name");
        temp->typeName = (char*)child->name;

        //û�����֣�����Ĭ�ϵ����֣����ܺ��ֹ������ĳ�ͻ��������
        //Ĭ�����ֲ�ȡԪ�����ͼ����ֵķ�ʽ
        if (temp->name == NULL)
        {
            temp->name = malloc(40);
            g_testcaseMemory[g_testcaseMemoryCount++] = temp->name;
            sprintf(temp->name, "%s_%03d" , temp->typeName, g_nameCountS++);
        }

        if (g_onOffDebugParseXml)
        {      
            printf("	----Start Parse type=%s ----name=\"%s\"  \n", temp->typeName, temp->name );  
        }

        /*******************************
        ���ԵĶ����������
        *******************************/
        if (strcmp((char*)child->name, "State") == 0)
        {
            ParseXmlStateModel(root, child);
        }

        if (strcmp((char*)child->name ,"Action") == 0)
        {
            temp->actionType = XmlGetProperty(child, "type");
            temp->publisher = XmlGetProperty(child, "publisher");
            temp->method = XmlGetProperty(child, "method");

            if (g_onOffDebugParseXml)
            {         
                if (temp->actionType)
                {            
                    printf("		action_type	= %s \n", temp->actionType);
                }
            }

            if (g_onOffDebugParseXml)
            {         
                if (temp->publisher)
                {            
                    printf("		publisher	= %s \n", temp->publisher);
                }
            }

            if (g_onOffDebugParseXml)
            {         
                if (temp->method)
                {            
                    printf("		method		= %s \n", temp->method);
                }
            }
            ParseXmlStateModel(root, child);
        }


        if (strcmp((char*)child->name, "DataModel") == 0)
        {
            temp->dataModelRef = XmlGetProperty(child, "ref");

            if (g_onOffDebugParseXml)
            {         
                if (temp->dataModelRef)
                {            
                    printf("		datamodel_ref	= %s \n", temp->dataModelRef);
                }
            }
        }

        if (strcmp((char*)child->name, "Data") == 0)
        {
            temp->fileName = XmlGetProperty(child, "fileName");

            if (g_onOffDebugParseXml)
            {         
                if (temp->fileName)
                {            
                    printf("		fileName	= %s \n", temp->fileName);
                }
            }
        }

        if (strcmp((char*)child->name, "Param") == 0)
        {
            temp->isclose = XmlGetProperty(child, "isclose");

            if (g_onOffDebugParseXml)
            {         
                if (temp->isclose)
                {            
                    printf("		isclose	= %s \n", temp->isclose);
                }
            }
        }

        if(g_onOffDebugParseXml)
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
