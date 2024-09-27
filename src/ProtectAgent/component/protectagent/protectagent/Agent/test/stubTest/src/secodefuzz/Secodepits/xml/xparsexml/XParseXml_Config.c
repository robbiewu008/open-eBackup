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

static void GetAllXmlElement_config(hw_xmlNodePtr root)
{
    hw_xmlNodePtr child = NULL;
    child = root->children;  

    SXMLElement *temp;

    while (child != NULL) 
    {  
        //
        if (strcmp((char*)child->name, "PitDefines") == 0)
        {
            child = child->children; 
            continue;
        }

        if (strcmp((char*)child->name, "All") == 0)
        {
            child = child->children; 
            continue;
        }

        if (strcmp((char*)child->name, "Group") == 0)
        {
            child = child->children; 
            continue;
        }

        //��������StateModel��һ���Խ�����ɣ��Ժ�Ͳ���Ҫ��
        if ((strcmp((char*)child->name, "String") == 0)
            || (strcmp((char*)child->name, "Hwaddr") == 0)
            || (strcmp((char*)child->name, "Ipv4") == 0)
            || (strcmp((char*)child->name, "Iface") == 0)
            || (strcmp((char*)child->name, "Range") == 0))
        {
            temp = malloc(sizeof(SXMLElement));
            g_testcaseMemory[g_testcaseMemoryCount++] = (char*)temp;
            Hw1Memset(temp, 0, sizeof(SXMLElement));

            //ָ�뻥ָ��������������
            temp->xmlNode = child;
            child->_private = temp;

            g_configValue[g_configNum] = XmlGetGetProp(child, "value");
            g_configKey[g_configNum] = XmlGetGetProp(child, "key");
            
            if (g_onOffDebugParseXml)
            {
                printf("	get config key is %s value is %s\r\n", g_configKey[g_configNum], g_configValue[g_configNum]);  
            }

            g_configNum++;
        }

        child = child->next;  
    }

    return;
}

void ParseXmlConfig(char* docname) 
{  
    hw_xmlDocPtr doc;  
    char configName[512];

    sprintf(configName, "%s.config", docname);

    //����xml�﷨,�ļ�����ʧ�ܾ͵���û�о�����
    //doc = xmlParseFile(configName);  
    doc = HW1xmlReadFile(configName, "utf-8", XML_PARSE_RECOVER);
    g_doc[g_docNum++] = doc;
    if (doc == NULL) {  
        printf("%s parse error or not found, please donot worry about it\r\n", configName);
        return;  
    }  

    if (g_onOffDebugParseXml)
    {
        printf("**************************************\r\n");  
        printf("!!!!!!!!!!!!!!!~~~~~~Parse xml.config file %s  start~~~~~~!!!!!!!!!!!!!!\r\n", configName);  
        printf("* * * * * * * * * * * * * *\r\n"); 
    }

    hw_xmlNodePtr root_config = HW1xmlDocGetRootElement(doc);  
    if (root_config == NULL) 
    {  
        xml_assert(0, "root parse erroe");
        return;  
    }  

    //��������ģ���﷨
    GetAllXmlElement_config(root_config);

    if (g_onOffDebugParseXml)
    {
        printf("* * * * * * * * * * * * * *\r\n");  
        printf("!!!!!!!!!!!!!!!~~~~~~Parse xml.config file %s  end~~~~~~!!!!!!!!!!!!!!\r\n", configName);  
        printf("**************************************\r\n");  
    }

    return;  
}  

#ifdef __cplusplus
}
#endif

#endif // Open_source
