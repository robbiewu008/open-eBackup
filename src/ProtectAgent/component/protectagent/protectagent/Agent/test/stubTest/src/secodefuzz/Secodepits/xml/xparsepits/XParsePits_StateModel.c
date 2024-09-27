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

static void AddStateElementChildren(SStateElement* tempParent, SStateElement* temp)
{
    // 如果儿子为0，则添加第一个儿子
    if (tempParent->children == NULL)
    {
        tempParent->children = temp;
        tempParent->lastChildren = temp;
        temp->parent = tempParent;
    }
    else
    {
        // 否则添加到结尾	
        SStateElement* temp1 = tempParent->lastChildren;
        temp1->next = temp;
        temp->prev = temp1;

        tempParent->lastChildren = temp;
        temp->parent = tempParent;
    }
}

// 得到变异元素二叉树
static void GetStateElement(hw_xmlNodePtr dataModel, char* xpath, SStateElement* tempMutatorElement, STestElement* tempTestElement)
{
    hw_xmlNodePtr child; 
    child = dataModel->children;
    SStateElement* temp = NULL;

    while (child != NULL) 
    { 
        char* name = (char*)child->name;

        // 不在这里，就不支持
        if ((strcmp(name, "State") != 0)
            && (strcmp(name, "Action") != 0)
            && (strcmp(name, "DataModel") != 0)
            && (strcmp(name, "Data") != 0)
            && (strcmp(name , "Param") != 0))
        {
            child = child->next;  
            continue;
        }

        temp = malloc(sizeof(SStateElement));
        g_testcaseMemory[g_testcaseMemoryCount++] = (char*)temp;
        Hw1Memset(temp, 0, sizeof(SStateElement));

        temp->xmlElement = (SXMLElement *)child->_private;
        temp->dataModeRoot = tempMutatorElement->dataModeRoot;
        sprintf(temp->xpathName, "%s.%s", xpath, temp->xmlElement->name);

        if (g_onOffDebugParseStateModel)
        {      
            printf("	Parse xpath_name =  %s  \n", temp->xpathName);
        }

        AddStateElementChildren(tempMutatorElement, temp);

        if (strcmp((char*)child->name, "DataModel") == 0)
        {
            temp->parent->dataModelRef = temp->xmlElement->dataModelRef;
        }

        if (strcmp((char*)child->name, "Data") == 0)
        {				
            temp->parent->binFileName = temp->xmlElement->fileName;
        }

        if (strcmp((char*)child->name, "Param") == 0)
        {				
            temp->parent->isClose = temp->xmlElement->isclose;
        }

        if (strcmp((char*)child->name, "Action") == 0)
        {
            temp->publihserName = temp->xmlElement->publisher;

            // 这里需要验证publisher是不是存在
            // 线稿儿子
            GetStateElement(child, temp->xpathName, temp, tempTestElement);

            if (strcmp(temp->xmlElement->actionType, "output") == 0)
            {
                //
                temp->mutatorElement = ParseDataModel(temp->dataModeRoot, temp->dataModelRef);

                // 搞定relation, fixup等关系字段之间的联系
                ParseAssociated(temp->mutatorElement);

                // char * buf = NULL;
                // int length =0;

                // 以用户调用接口设置优先
                if (g_binName)
                {            
                    GetBinBuf(g_binName, &(temp->binBuf), &temp->binLength);
                }
                else if (g_binBuf)
                {
                    temp->binBuf = g_binBuf;
                    temp->binLength = g_binBufLength;
                }
                else if (temp->binFileName)
                {
                    GetBinBuf(temp->binFileName, &(temp->binBuf), &temp->binLength);
                }

                // 解析bin,得到数据元素的默认数据	
                // 如果bin为null,从数据模型得到数据
                temp->binElement = ParseBin(temp->mutatorElement, temp->binBuf, temp->binLength);
            }

            temp->outBuf = malloc(OUT_BUF_MAX_LENGTH);
            g_testcaseMemory[g_testcaseMemoryCount++] = temp->outBuf;
        }

        if (strcmp((char*)child->name, "State") == 0)
        {
            GetStateElement(child, temp->xpathName, temp, tempTestElement);
        }

        child = child->next;  
    }
}

SStateElement* ParseStateModel(hw_xmlNodePtr root, char* stateModelName, STestElement* tempTestElement)
{  
    if (g_onOffDebugParseStateModel)
    {
        printf("||||||||||||||||||||||||||||||||||||||\r\n");  
        printf("****Parse StateModel (%s) Start\r\n", stateModelName);  
        printf("* * * * * * * * * * * * * *\r\n");  
    }

    hw_xmlNodePtr stateModel;
    SXMLElement* tempXml;
    SStateElement* stateElement;

    stateModel = XmlGetStateModelNodeByName(root, stateModelName);
    if (stateModel == NULL) 
    {  
        printf("stateModel (%s) \r\n", stateModelName);  
        xml_assert(0, "stateModel not fount");
        return 0;  
    }  

    stateElement = malloc(sizeof(SStateElement));
    g_testcaseMemory[g_testcaseMemoryCount++] = (char*)stateElement;
    Hw1Memset(stateElement, 0, sizeof(SStateElement ));

    tempXml = (SXMLElement *)stateModel->_private;
    stateElement->xmlElement = tempXml;
    stateElement->dataModeRoot = XmlGetStateModelDocByName(root, stateModelName);

    sprintf(stateElement->xpathName, "%s", stateElement->xmlElement->name);

    // 得到指定数据模型的元素，解析ref
    GetStateElement(stateModel, stateModelName, stateElement, tempTestElement);

    SStateElement* temp = stateElement;

    // 解析结束，把最终的树打印出来
    if (g_onOffDebugParseStateModel)
    {
        printf("--------insert\r\n"); 
    }

    For_Tree_Start(temp)
    {
        if (g_onOffDebugParseStateModel)
        {      
            printf("		%s\r\n", temp->xpathName);
        }
    }
    For_Tree_End(temp)

    if (g_onOffDebugParseStateModel)
    {
        printf("* * * * * * * * * * * * * *\r\n");  
        printf("****Parse stateModel (%s) End\r\n", stateModelName);  
        printf("||||||||||||||||||||||||||||||||||||||\r\n");  
    }

    return stateElement;  
} 

#ifdef __cplusplus
}
#endif

#endif // Open_source
