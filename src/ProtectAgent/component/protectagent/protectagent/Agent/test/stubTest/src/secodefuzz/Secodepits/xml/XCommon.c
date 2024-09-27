/*
版权所有 (c) 华为技术有限公司 2012-2018


对外接口在这个文件里封装
*/
#include "XML.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef Open_source
static STestElement* g_testElement;
#endif

// 存放临时doc内存，整个测视例后释放
hw_xmlDocPtr g_doc[100];
int g_docNum = 0;

// 存放临时内存，整个测视例后释放
char* g_testcaseMemory[200000];
int g_testcaseMemoryCount = 0;

// 存放临时内存，一次运行后释放
char* g_onerunMemory[200000];
int g_onerunMemoryCount = 0;

SPitsMutator g_pitsMutator = {0};
int g_mutatorFrequency = 100;
int g_algorithmFrequency = 20;

char* g_binName = NULL;
char* g_binBuf = NULL;
int g_binBufLength = 0;

int g_maxOutOfMutator = 4000;  // 配置单个字段变异最大长度,仅2.0代码使用

int g_isSwap = 0;

int g_configNum = 0;
char *g_configValue[MAX_PITS_PARA_NUMBER*2] = {0};
char *g_configKey[MAX_PITS_PARA_NUMBER*2] = {0};

int g_tempElementCount = 0;


// 临时buf，牺牲内存提升速度
char g_outBuf1[OUT_BUF_MAX_LENGTH] = {0};
char g_outBuf2[OUT_BUF_MAX_LENGTH] = {0};

char* g_publisherBuf = NULL;
int g_publisherBufLen = 0;

void DT_Pits_SetBinFile(char* binName)
{
    g_binName = binName;
}

void DT_Pits_SetBinBuf(char* binBuf, int binBufLength)
{
    g_binBuf = binBuf;
    g_binBufLength = binBufLength;
}

#ifndef Open_source
 int DT_Pits_ParsePits(char* docName)
 {
    if (g_publisherCount == 0)
    {
        DoPublisherInit();
    }

    if (g_fixupCount == 0)
    {
        DoFixupInit();
    }

    if (g_transformerCount == 0)
    {
        DoTransformerInit();
    }

    g_tempElementCount = 0;

    hw_xmlNodePtr root;

    //解析数据模型整个文件
    root = ParseXml(docName, 1); 
    if (root)
    {
        g_testElement = ParseTestModel(root, "Default");
    }

    if (root == NULL)
    {
        return 0;
    }
    
    return 1;
}

void DT_Pits_DoState(void)
{
    // 状态模型只有一个，这里清0可以
    g_tempElementCount = 0;
    
    DoState(g_testElement->stateElement, g_testElement);
}
#endif

  int DT_Pits_ParseDataModel(char* docName, char* dataModelName, int isfirst)
 {

    if (g_fixupCount == 0)
    {
        DoFixupInit();
    }

    if (g_transformerCount == 0)
    {
        DoTransformerInit();
    }

    int id;

    //解析数据模型整个文件
    id = ParseXmlHW(docName, dataModelName, isfirst); 
    
    return id;
}

void DT_Pits_DoAction(int id)
{
    // action有多个，只能在第一个的地方清0
    if (id == 0)
    {
        g_tempElementCount = 0;
    }
    
    DoActionHW(id);
}

 void DT_Pits_ParseFree(void)
 {
    int i = 0;
    for (i = 0; i < g_docNum; i++)
    {
        if (g_doc[i])
        HW1xmlFreeDoc(g_doc[i]);
        g_doc[i] = NULL;
    }
    g_docNum = 0;
    HW1xmlCleanupParser();

    for (i = 0; i < g_testcaseMemoryCount; i++)
    {
        if(g_testcaseMemory[i])
        free(g_testcaseMemory[i]);
        g_testcaseMemory[i] = NULL;
    }
    g_testcaseMemoryCount = 0;

    // 清除config
    g_configNum = 0;

    // 样本变量
    g_binName = NULL;
    g_binBuf = NULL;
    g_binBufLength = 0;

    // debug
    g_onOffDebugMutatorElement = 0;
    g_onOffDebugParseAssociated = 0;
    g_onOffDebugGetBinBuf = 0;
    g_onOffDebugParseBin = 0;
    g_onOffDebugParseDataModel = 0;
    g_onOffDebugParseStateModel = 0;
    g_onOffDebugParseTestModel = 0;
    g_onOffDebugParseXml = 0;
    g_onOffDebugPublisher = 0;
    g_onOffDebugDoRelation = 0;
    g_onOffDebugDoFixup = 0;
    g_onOffDebugDoTransformer = 0;
    g_onOffDebugMutatorPits = 0;

    //大小端
    g_isSwap = 0;

    AnalyzerClean();
  }

 void DT_Pits_OneRunFree(void)
{
    int i = 0;
    for(i = 0; i < g_onerunMemoryCount; i++)
    {
        if (g_onerunMemory[i])
        free (g_onerunMemory[i]);
        g_onerunMemory[i] = NULL;
    }
    g_onerunMemoryCount = 0;
}

void DT_Pits_GetMutatorBuf(char** buf, int* len)
{
    *buf = g_publisherBuf;
    *len = g_publisherBufLen;
}

void	DT_Pits_Enable_AllMutater(int isEnable)
{
    if(isEnable)
    {
        g_pitsMutator.mutatorBufCut = 1;
        g_pitsMutator.mutatorBlockDelete = 1;
        g_pitsMutator.mutatorBlockCopy = 1;
        g_pitsMutator.mutatorBlockChange = 1;
        g_pitsMutator.mutatorElementDelete = 1;
        g_pitsMutator.mutatorElementCopy = 1;
        g_pitsMutator.mutatorElementChange = 1;
        g_pitsMutator.mutatorRelationsize = 1;
    }
    else
    {
        g_pitsMutator.mutatorBufCut = 0;
        g_pitsMutator.mutatorBlockDelete = 0;
        g_pitsMutator.mutatorBlockCopy = 0;
        g_pitsMutator.mutatorBlockChange = 0;
        g_pitsMutator.mutatorElementDelete = 0;
        g_pitsMutator.mutatorElementCopy = 0;
        g_pitsMutator.mutatorElementChange = 0;
        g_pitsMutator.mutatorRelationsize = 0;
    }
}

void	DT_Pits_Enable_OneMutater(enum Enum_Pits_Type  mutatedNum, int isEnable)
{
    if (mutatedNum == ENUM_BUF_CUT)
    {
        if (isEnable)
        {
            g_pitsMutator.mutatorBufCut = 1;
        }
        else
        {
            g_pitsMutator.mutatorBufCut = 0;
        }
    }
    else if (mutatedNum == ENUM_BLOCK_DELETE)
    {
        if (isEnable)
        {
            g_pitsMutator.mutatorBlockDelete = 1;
        }
        else
        {
            g_pitsMutator.mutatorBlockDelete = 0;
        }
    }
    else if (mutatedNum == ENUM_BLOCK_COPY)
    {
        if (isEnable)
        {
            g_pitsMutator.mutatorBlockCopy = 1;
        }
        else
        {
            g_pitsMutator.mutatorBlockCopy = 0;
        }
    }
    else if (mutatedNum == ENUM_BLOCK_CHANGE)
    {
        if (isEnable)
        {
            g_pitsMutator.mutatorBlockChange = 1;
        }
        else
        {
            g_pitsMutator.mutatorBlockChange = 0;
        }
    }
    else if (mutatedNum == ENUM_ELEMENT_DELETE)
    {
        if (isEnable)
        {
            g_pitsMutator.mutatorElementDelete = 1;
        }
        else
        {
            g_pitsMutator.mutatorElementDelete = 0;
        }
    }
    else if (mutatedNum == ENUM_ELEMENT_COPY)
    {
        if (isEnable)
        {
            g_pitsMutator.mutatorElementCopy = 1;
        }
        else
        {
            g_pitsMutator.mutatorElementCopy = 0;
        }
    }
    else if (mutatedNum == ENUM_ELEMENT_CHANGE)
    {
        if (isEnable)
        {
            g_pitsMutator.mutatorElementChange = 1;
        }
        else
        {
            g_pitsMutator.mutatorElementChange = 0;
        }
    }
    else if (mutatedNum == ENUM_RELATION_SIZE)
    {
        if (isEnable)
        {
            g_pitsMutator.mutatorRelationsize = 1;
        }
        else
        {
            g_pitsMutator.mutatorRelationsize = 0;
        }
    }
}

void	DT_Pits_Set_Mutator_Frequency(int frequency)
{
    g_mutatorFrequency = frequency;
}

void	DT_Pits_Set_Algorithm_Frequency(int frequency)
{
    g_algorithmFrequency = frequency;
}

void DT_Pits_Set_MaxOutput(int value)
{
    g_maxOutOfMutator = value;
}

void DT_Pits_Set_DebugOnOff(char* debug)
{
    DoDebugOpen(debug);
}

void DT_Pits_Set_BigOrLittleSwap(int swap)
{
    g_isSwap = swap;
}

void DT_Pits_Set_Config(char* configKey, char* configValue)
{
    if (g_configNum < 10)
    {
        g_configValue[g_configNum] = configValue;
        g_configKey[g_configNum] = configKey;
        g_configNum++;
    }
}

#ifdef __cplusplus
}
#endif
