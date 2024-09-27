/*
版权所有 (c) 华为技术有限公司 2012-2018


解析指定数据模型，生成需要变异的元素二叉树，并解决ref问题

数组与choice仅解析，不解决
*/
#ifndef Open_source

#include "../XML.h"

#ifdef __cplusplus
extern "C" {
#endif

//得到变异元素二叉树
static void GetTestElement(hw_xmlNodePtr dataModel, char* xpath, STestElement* tempTestElement)
{
    SXMLElement* tempXml;

    hw_xmlNodePtr child; 
    child = dataModel->children;

    //test只有一个
    STestElement* temp = tempTestElement;

    while (child != NULL) 
    { 
        char* name = (char*)child->name;

        //不在这里，就不支持
        if ((strcmp(name, "StateModel") != 0)
            && (strcmp(name, "Publisher") != 0))
        {
            child = child->next;  
            continue;
        }

        tempXml = (SXMLElement *)child->_private;

        if (g_onOffDebugParseTestModel)
        {      
            printf("	Parse name =  %s  \n", name);
        }

        if (strcmp((char*)child->name, "StateModel") == 0)
        {
            temp->stateModelRef = tempXml->stateModelRef;

            //如果有:,当做include，必须要处理
            temp->stateElement = ParseStateModel(temp->dataModeRoot, temp->stateModelRef ,temp);
        }

        if (strcmp((char*)child->name, "Publisher") == 0)
        {
            int i = 0;
            temp->publisherName[temp->publisherCount] = tempXml->publisherName;
            temp->publisherClass[temp->publisherCount] = tempXml->publisherClass;

            for (i = 0; i < 10; i++)
            {
                temp->publisherParamName[temp->publisherCount][i]  = tempXml->publisherParamName[i];
                temp->publisherParamValue[temp->publisherCount][i]  = tempXml->publisherParamValue[i];
            }

            temp->publisherCount++;
        }

        child = child->next;  
    }
}

STestElement* ParseTestModel(hw_xmlNodePtr root, char* testModelName)
{  
    if (g_onOffDebugParseTestModel)
    {
        printf("||||||||||||||||||||||||||||||||||||||\r\n");  
        printf("****Parse TestModel Start\r\n");  
        printf("* * * * * * * * * * * * * *\r\n");  
    }

    hw_xmlNodePtr testModel;
    SXMLElement* tempXml;
    STestElement* testElement;

    testModel = XmlGetTestModelByName(root, testModelName);
    if (testModel == NULL) 
    {  
        xml_assert(0, "testModel not fount");
        return 0;  
    }  

    testElement = malloc(sizeof(STestElement));
    g_testcaseMemory[g_testcaseMemoryCount++] = (char*)testElement;
    Hw1Memset(testElement, 0, sizeof(STestElement ));

    tempXml = (SXMLElement *)testModel->_private;
    testElement->xmlElement = tempXml;
    testElement->dataModeRoot = root;

    sprintf(testElement->xpathName, "%s", testElement->xmlElement->name);

    //得到指定数据模型的元素，解析ref
    GetTestElement(testModel, testModelName, testElement);

    if (g_onOffDebugParseTestModel)
    {
        printf("* * * * * * * * * * * * * *\r\n");  
        printf("****Parse testModel  End\r\n");  
        printf("||||||||||||||||||||||||||||||||||||||\r\n");  
    }

    return testElement;  
} 

#ifdef __cplusplus
}
#endif

#endif // Open_source
