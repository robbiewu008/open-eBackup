/*
��Ȩ���� (c) ��Ϊ�������޹�˾ 2012-2018


����ָ������ģ�ͣ�������Ҫ�����Ԫ�ض������������ref����

������choice�������������
*/
#ifndef Open_source

#include "../XML.h"

#ifdef __cplusplus
extern "C" {
#endif

static void AddStateElementChildren(SStateElement* tempParent, SStateElement* temp)
{
    // �������Ϊ0������ӵ�һ������
    if (tempParent->children == NULL)
    {
        tempParent->children = temp;
        tempParent->lastChildren = temp;
        temp->parent = tempParent;
    }
    else
    {
        // ������ӵ���β	
        SStateElement* temp1 = tempParent->lastChildren;
        temp1->next = temp;
        temp->prev = temp1;

        tempParent->lastChildren = temp;
        temp->parent = tempParent;
    }
}

// �õ�����Ԫ�ض�����
static void GetStateElement(hw_xmlNodePtr dataModel, char* xpath, SStateElement* tempMutatorElement, STestElement* tempTestElement)
{
    hw_xmlNodePtr child; 
    child = dataModel->children;
    SStateElement* temp = NULL;

    while (child != NULL) 
    { 
        char* name = (char*)child->name;

        // ��������Ͳ�֧��
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

            // ������Ҫ��֤publisher�ǲ��Ǵ���
            // �߸����
            GetStateElement(child, temp->xpathName, temp, tempTestElement);

            if (strcmp(temp->xmlElement->actionType, "output") == 0)
            {
                //
                temp->mutatorElement = ParseDataModel(temp->dataModeRoot, temp->dataModelRef);

                // �㶨relation, fixup�ȹ�ϵ�ֶ�֮�����ϵ
                ParseAssociated(temp->mutatorElement);

                // char * buf = NULL;
                // int length =0;

                // ���û����ýӿ���������
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

                // ����bin,�õ�����Ԫ�ص�Ĭ������	
                // ���binΪnull,������ģ�͵õ�����
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

    // �õ�ָ������ģ�͵�Ԫ�أ�����ref
    GetStateElement(stateModel, stateModelName, stateElement, tempTestElement);

    SStateElement* temp = stateElement;

    // ���������������յ�����ӡ����
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
