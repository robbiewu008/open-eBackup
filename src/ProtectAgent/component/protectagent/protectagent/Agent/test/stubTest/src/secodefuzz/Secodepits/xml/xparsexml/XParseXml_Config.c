/*
版权所有 (c) 华为技术有限公司 2012-2018


解析所有数据模型，生成xml结构体，每个节点一一对应

//得到xml元素， 遍历整个数据模型，   保留xmlNodePtr完整结构，并生成g_xml_element，赋值给_private

//所有的xml语法到这里截止，全部解开存入g_xml_element

//之后的处理不允许出现xml语法，以便以后替换语法替换数据模型结构方便
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

        //遍历所有StateModel，一次性解析完成，以后就不需要了
        if ((strcmp((char*)child->name, "String") == 0)
            || (strcmp((char*)child->name, "Hwaddr") == 0)
            || (strcmp((char*)child->name, "Ipv4") == 0)
            || (strcmp((char*)child->name, "Iface") == 0)
            || (strcmp((char*)child->name, "Range") == 0))
        {
            temp = malloc(sizeof(SXMLElement));
            g_testcaseMemory[g_testcaseMemoryCount++] = (char*)temp;
            Hw1Memset(temp, 0, sizeof(SXMLElement));

            //指针互指，查找起来方便
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

    //解析xml语法,文件解析失败就当做没有就行了
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

    //解析数据模型语法
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
