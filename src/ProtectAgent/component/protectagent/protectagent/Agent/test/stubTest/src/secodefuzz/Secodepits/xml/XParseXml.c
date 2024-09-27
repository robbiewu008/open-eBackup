/*
��Ȩ���� (c) ��Ϊ�������޹�˾ 2012-2018


������������ģ�ͣ�����xml�ṹ�壬ÿ���ڵ�һһ��Ӧ

//�õ�xmlԪ�أ� ������������ģ�ͣ�   ����xmlNodePtr�����ṹ��������g_xml_element����ֵ��_private

//���е�xml�﷨�������ֹ��ȫ���⿪����g_xml_element

//֮��Ĵ����������xml�﷨���Ա��Ժ��滻�﷨�滻����ģ�ͽṹ����
*/
#ifndef Open_source

#include "XML.h"

#ifdef __cplusplus
extern "C" {
#endif

static int GetmaxOutputSize(hw_xmlNodePtr node)
{
    char* tempValue = XmlGetGetProp(node, "maxOutputSize");

    // Ĭ��Ϊ0���Ǿ���Ҫ�Ժ�ֵ��
    if (tempValue == NULL)
    {
        return 0;
    }

    int tempNumber = atol(tempValue);

    return tempNumber;
}

static void GetAllXmlElement(hw_xmlNodePtr root)
{
    hw_xmlNodePtr child = NULL;
    child = root->children;  

    SXMLElement *temp;

    while (child != NULL) 
    {  
        // ��������DataModel��һ���Խ�����ɣ��Ժ�Ͳ���Ҫ��
        if (strcmp((char*)child->name, "DataModel") == 0)
        {
            temp = malloc(sizeof(SXMLElement));
            g_testcaseMemory[g_testcaseMemoryCount++] = (char*)temp;
            Hw1Memset(temp, 0, sizeof(SXMLElement));

            //ָ�뻥ָ��������������
            temp->xmlNode = child;
            child->_private = temp;

            temp->name = XmlGetGetProp(child, "name");
            temp->typeName = (char*)child->name;

            if (g_onOffDebugParseXml)
            {
                printf("**************************************\r\n");  
                printf("****Parse DataModel %s  Start\r\n", temp->name);  
                printf("* * * * * * * * * * * * * *\r\n"); 
            }

            // �õ�ref��Ϣ,����ģ�ͽ�֧��ref����ò��
            if (XmlGetGetProp(child, "ref") != NULL)
            {
                temp->isHasRef = 1;
                temp->xmlRef = XmlGetDataModelByName(root, XmlGetGetProp(child, "ref"));

                if (temp->xmlRef == NULL) 
                {  
                    xml_assert(0, "ref DateMode not found");
                    return ;  
                } 
            }
            else
            {         
                temp->isHasRef = 0;
            }

            ParseXmlDataModel(root, child);

            if (g_onOffDebugParseXml)
            {
                printf("* * * * * * * * * * * * * *\r\n");  
                printf("****Parse DataModel %s  End\r\n", temp->name);  
                printf("**************************************\r\n");  
            }
        }

        // ��������StateModel��һ���Խ�����ɣ��Ժ�Ͳ���Ҫ��
        if (strcmp((char*)child->name, "StateModel") == 0)
        {
            temp = malloc(sizeof(SXMLElement));
            g_testcaseMemory[g_testcaseMemoryCount++] = (char*)temp;
            Hw1Memset(temp, 0, sizeof(SXMLElement));

            // ָ�뻥ָ��������������
            temp->xmlNode = child;
            child->_private = temp;

            temp->name = XmlGetGetProp(child, "name");
            temp->typeName = (char*)child->name;
            temp->initialState = XmlGetProperty(child, "initialState");

            if (g_onOffDebugParseXml)
            {
                printf("**************************************\r\n");  
                printf("****Parse StateModel %s  Start\r\n", temp->name);  
                printf("* * * * * * * * * * * * * *\r\n"); 
            }

            ParseXmlStateModel(root, child);

            if (g_onOffDebugParseXml)
            {
                printf("* * * * * * * * * * * * * *\r\n");  
                printf("****Parse StateModel %s  End\r\n", temp->name);  
                printf("**************************************\r\n");  
            }
        }

        // ��������TestModel��һ���Խ�����ɣ��Ժ�Ͳ���Ҫ��
        if (strcmp((char*)child->name, "Test") == 0)
        {
            temp = malloc(sizeof(SXMLElement));
            g_testcaseMemory[g_testcaseMemoryCount++] = (char*)temp;
            Hw1Memset(temp, 0, sizeof(SXMLElement));

            // ָ�뻥ָ��������������
            temp->xmlNode = child;
            child->_private = temp;

            temp->name = XmlGetGetProp(child,"name");
            temp->typeName = (char*)child->name;
            temp->maxOutputSize = GetmaxOutputSize(child);

            if (g_onOffDebugParseXml)
            {
                printf("**************************************\r\n");  
                printf("****Parse TestModel %s  Start\r\n", temp->name);  
                printf("* * * * * * * * * * * * * *\r\n"); 
            }

            ParseXmlTestModel(root, child);

            if (g_onOffDebugParseXml)
            {
                printf("* * * * * * * * * * * * * *\r\n");  
                printf("****Parse TestModel %s  End\r\n", temp->name);  
                printf("**************************************\r\n");  
            }
        }

        // ��������DataModel��һ���Խ�����ɣ��Ժ�Ͳ���Ҫ��
        if (strcmp((char*)child->name, "Include") == 0)
        {
            temp = malloc(sizeof(SXMLElement));
            g_testcaseMemory[g_testcaseMemoryCount++] = (char*)temp;
            Hw1Memset(temp, 0, sizeof(SXMLElement));

            // ָ�뻥ָ��������������
            temp->xmlNode = child;
            child->_private = temp;

            temp->name = XmlGetGetProp(child, "name");
            temp->typeName = (char*)child->name;


            temp->includeNs = XmlGetGetProp(child, "ns");
            temp->includeSrc = XmlGetGetProp(child, "src");

            if (g_onOffDebugParseXml)
            {
                printf("**************************************\r\n");  
                printf("****Parse include file %s  Start\r\n", temp->name);  
                printf("* * * * * * * * * * * * * *\r\n"); 
            }

            // ȥ��file:
            char* temp_src = temp->includeSrc;
            if (MyMemmem(temp->includeSrc, strlen(temp->includeSrc), "file:", 5))
            {
                temp_src = temp_src + 5;
            }

            temp->includeRootDoc = ParseXml(temp_src, 0);

            if (g_onOffDebugParseXml)
            {
                printf("* * * * * * * * * * * * * *\r\n");  
                printf("****Parse include file %s  End\r\n", temp->name);  
                printf("**************************************\r\n");  
            }
        }
        child = child->next;  
    }

    return;
}

 hw_xmlNodePtr ParseXml(char* docName, int needParseConfig) 
 {  
    hw_xmlDocPtr doc;  
    hw_xmlNodePtr root; 

    if (needParseConfig == 1)
    {
        // hw_xmlNodePtr root_config;
        // Ҫ�Ƚ��������ļ�
        // root_config = 
        ParseXmlConfig(docName);
    }

    if (g_onOffDebugParseXml)
    {
        printf("**************************************\r\n");  
        printf("!!!!!!!!!!!!!!!~~~~~~Parse xml file %s  ~~~~~~!!!!!!!!!!!!!!\r\n", docName);  
        printf("* * * * * * * * * * * * * *\r\n"); 
    }

    // ����xml�﷨
    // doc = xmlParseFile(docName);  
    doc = HW1xmlReadFile(docName, "utf-8", XML_PARSE_RECOVER);
    g_doc[g_docNum++] = doc;
    if (doc == NULL) 
    {  
        xml_assert(0, "doc parse erroe");
        return 0;  
    }  

    root = HW1xmlDocGetRootElement(doc);  
    if (root == NULL) 
    {  
        xml_assert(0, "root parse erroe");
        return 0;  
    }  

    // ��������ģ���﷨
    GetAllXmlElement(root);

    return root;  
}  

#ifdef __cplusplus
}
#endif

#endif // Open_source
