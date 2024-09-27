/*
版权所有 (c) 华为技术有限公司 2012-2018


解析指定数据模型，生成需要变异的元素二叉树，并解决ref问题

数组与choice仅解析，不解决
*/

#include "../XML.h"

#ifdef __cplusplus
extern "C" {
#endif

// 得到变异元素二叉树
static void GetMutatorElement(hw_xmlNodePtr dataModel, char* xpath, SMutatorElement* tempMutatorElement)
{
    // 有ref,先解析ref的datamode,增加到二叉树上

    // ref可以多重嵌套
    SXMLElement* tempXml;

    tempXml = (SXMLElement *)dataModel->_private;
    if (tempXml->isHasRef)
    {
        // xpath不一样
        GetMutatorElement(tempXml->xmlRef, xpath,tempMutatorElement);
    }


    hw_xmlNodePtr child; 
    child = dataModel->children;
    SMutatorElement* temp = NULL;

    while (child != NULL) 
    { 

        char* name = (char*)child->name;

        // 不在这里，就不支持
        if ((strcmp(name, "Number") != 0)
            && (strcmp(name, "String") != 0)
            && (strcmp(name, "Blob") != 0)
            && (strcmp(name, "Block") != 0)
            && (strcmp(name, "Choice") != 0)
            && (strcmp(name, "Flags") != 0)
            && (strcmp(name, "Flag") != 0)
            && (strcmp(name, "Padding") != 0))
        {
            child = child->next;
            continue;
        }

        temp = malloc(sizeof(SMutatorElement));
        g_testcaseMemory[g_testcaseMemoryCount++] = (char*)temp;

        Hw1Memset(temp, 0, sizeof(SMutatorElement));

        temp->xmlElement = (SXMLElement *)child->_private;
        sprintf(temp->xpathName, "%s.%s", xpath, temp->xmlElement->name);

        if (g_onOffDebugParseDataModel)
        {      
            printf("	Parse xpath_name =  %s  \n", temp->xpathName);
        }

        MutatorElementAddChildren(tempMutatorElement, temp);

        if (strcmp((char*)child->name, "Number") == 0)
        {

        }

        if (strcmp((char*)child->name, "Flag") == 0)
        {

        }

        if (strcmp((char*)child->name, "String") == 0)
        {

        }

        if (strcmp((char*)child->name, "Blob") == 0)
        {

        }

        if (strcmp((char*)child->name, "Block") == 0)
        {
            GetMutatorElement(child, temp->xpathName,temp);
        }

        if (strcmp((char*)child->name, "Flags") == 0)
        {
            GetMutatorElement(child, temp->xpathName,temp);
        }

        // choice本身不支持数组，ref,哈哈
        if (strcmp((char*)child->name, "Choice") == 0)
        {
            GetMutatorElement(child, temp->xpathName,temp);

            // 先选择大儿子继承王位  :) ,有bin的时候会被改写
            temp->seletedChildren = temp->children;
        }

        child = child->next;  
    }
}

 SMutatorElement * ParseDataModel(hw_xmlNodePtr root, char* dataModelName) 
 {  
    if (g_onOffDebugParseDataModel)
    {
        printf("||||||||||||||||||||||||||||||||||||||\r\n");  
        printf("****ParsePits DataModel %s Start\r\n", dataModelName);  
        printf("* * * * * * * * * * * * * *\r\n");  
    }

    hw_xmlNodePtr dataModel;
    SXMLElement* tempXml;

    SMutatorElement* mutatorElement;

    dataModel = XmlGetDataModelByName(root, dataModelName);
    if (dataModel == NULL) 
    {  
        printf("dataModel %s not fount\r\n", dataModelName);
        xml_assert(0, " please check!!!");
        return 0;  
    }  

    mutatorElement = malloc(sizeof(SMutatorElement));
    g_testcaseMemory[g_testcaseMemoryCount++] = (char*)mutatorElement;
    Hw1Memset(mutatorElement, 0, sizeof(SMutatorElement ));

    tempXml = (SXMLElement *)dataModel->_private;
    mutatorElement->xmlElement = tempXml;

    sprintf(mutatorElement->xpathName, "%s", mutatorElement->xmlElement->name);

    // 得到指定数据模型的元素，解析ref
    GetMutatorElement(dataModel, dataModelName, mutatorElement);

    SMutatorElement* temp = mutatorElement;

    // 解析结束，把最终的树打印出来
    if (g_onOffDebugParseDataModel)
    {
        printf("--------insert\r\n"); 
    }

    For_Tree_Start(temp)
    {
        if (g_onOffDebugParseDataModel)
        {      
            printf("		%s\r\n", temp->xpathName);
        }
    }
    For_Tree_End(temp)

    if (g_onOffDebugParseDataModel)
    {
        printf("* * * * * * * * * * * * * *\r\n");  
        printf("****ParsePits dataModel %s  End\r\n", dataModelName);  
        printf("||||||||||||||||||||||||||||||||||||||\r\n");  
    }

    return mutatorElement;  
} 

#ifdef __cplusplus
}
#endif
