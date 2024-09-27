/*
版权所有 (c) 华为技术有限公司 2012-2018


解析所有数据模型，生成xml结构体，每个节点一一对应

//得到xml元素， 遍历整个数据模型，   保留xmlNodePtr完整结构，并生成g_xml_element，赋值给_private

//所有的xml语法到这里截止，全部解开存入g_xml_element

//之后的处理不允许出现xml语法，以便以后替换语法替换数据模型结构方便
*/
#ifndef Open_source

#include "XML.h"

#ifdef __cplusplus
extern "C" {
#endif

static int GetmaxOutputSize(hw_xmlNodePtr node)
{
    char* tempValue = XmlGetGetProp(node, "maxOutputSize");

    // 默认为0，那就需要以后赋值了
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
        // 遍历所有DataModel，一次性解析完成，以后就不需要了
        if (strcmp((char*)child->name, "DataModel") == 0)
        {
            temp = malloc(sizeof(SXMLElement));
            g_testcaseMemory[g_testcaseMemoryCount++] = (char*)temp;
            Hw1Memset(temp, 0, sizeof(SXMLElement));

            //指针互指，查找起来方便
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

            // 得到ref信息,数据模型仅支持ref属性貌似
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

        // 遍历所有StateModel，一次性解析完成，以后就不需要了
        if (strcmp((char*)child->name, "StateModel") == 0)
        {
            temp = malloc(sizeof(SXMLElement));
            g_testcaseMemory[g_testcaseMemoryCount++] = (char*)temp;
            Hw1Memset(temp, 0, sizeof(SXMLElement));

            // 指针互指，查找起来方便
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

        // 遍历所有TestModel，一次性解析完成，以后就不需要了
        if (strcmp((char*)child->name, "Test") == 0)
        {
            temp = malloc(sizeof(SXMLElement));
            g_testcaseMemory[g_testcaseMemoryCount++] = (char*)temp;
            Hw1Memset(temp, 0, sizeof(SXMLElement));

            // 指针互指，查找起来方便
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

        // 遍历所有DataModel，一次性解析完成，以后就不需要了
        if (strcmp((char*)child->name, "Include") == 0)
        {
            temp = malloc(sizeof(SXMLElement));
            g_testcaseMemory[g_testcaseMemoryCount++] = (char*)temp;
            Hw1Memset(temp, 0, sizeof(SXMLElement));

            // 指针互指，查找起来方便
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

            // 去掉file:
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
        // 要先解析配置文件
        // root_config = 
        ParseXmlConfig(docName);
    }

    if (g_onOffDebugParseXml)
    {
        printf("**************************************\r\n");  
        printf("!!!!!!!!!!!!!!!~~~~~~Parse xml file %s  ~~~~~~!!!!!!!!!!!!!!\r\n", docName);  
        printf("* * * * * * * * * * * * * *\r\n"); 
    }

    // 解析xml语法
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

    // 解析数据模型语法
    GetAllXmlElement(root);

    return root;  
}  

#ifdef __cplusplus
}
#endif

#endif // Open_source
