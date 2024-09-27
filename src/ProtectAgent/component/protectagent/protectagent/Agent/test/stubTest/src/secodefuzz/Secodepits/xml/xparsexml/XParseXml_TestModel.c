/*
版权所有 (c) 华为技术有限公司 2012-2018


解析所有数据模型，生成xml结构体，每个节点一一对应

// 得到xml元素， 遍历整个数据模型，   保留xmlNodePtr完整结构，并生成g_xml_element，赋值给_private

// 所有的xml语法到这里截止，全部解开存入g_xml_element

// 之后的处理不允许出现xml语法，以便以后替换语法替换数据模型结构方便
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
        // test类型直接忽略，暂时没看到有啥用处
        if (strcmp((char*)child->name, "text") == 0)
        {
            child = child->next; 
            continue;
        }

        // 分配xml元素内存，直接挂接在node树上
        temp = malloc(sizeof(SXMLElement));
        g_testcaseMemory[g_testcaseMemoryCount++] = (char*)temp;
        Hw1Memset((char*)temp, 0, sizeof(SXMLElement));

        // 指针互指，查找起来方便
        temp->xmlNode = child;
        child->_private = temp;

        /*******************************
        共性的东西放在上边
        ********************************/
        temp->name = XmlGetGetProp(child, "name");
        temp->typeName = (char*)child->name;

        // 没有名字，给个默认的名字，可能和手工命名的冲突，不管了
        // 默认名字采取元素类型加数字的方式
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
        特性的东西放在这边
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
