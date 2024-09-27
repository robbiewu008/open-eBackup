/*
版权所有 (c) 华为技术有限公司 2012-2018


json
参考
https://www.cnblogs.com/secondtononewe/p/6047029.html
*/

#include "../../XML.h"
#include "cJSON.h"


#ifdef _WIN32
#include<ws2tcpip.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

// 单纯为了内存释放
cJSON* g_tempJson[1000];
int g_tempJsonNumber = 0;

static void DoJsonAnalyzerMutator(cJSON* pJsonRoot,cJSON* pJsonMutator)
{
    if ((pJsonRoot->type == cJSON_Object) || (pJsonRoot->type == cJSON_Array))
    {
        cJSON* child = pJsonRoot->child;
        cJSON* child_Mutator = pJsonMutator->child;
        for ( ;child; child = child->next,child_Mutator=child_Mutator->next)
        {
            DoJsonAnalyzerMutator(child,child_Mutator);
        }
        return;
    }

    // 拷贝内存有点浪费时间，以后再优化
    if (pJsonRoot->type == cJSON_String)
    {
        char* value  = DT_SetGetString(&g_Element[g_tempElementCount], strlen(pJsonRoot->valuestring) + 1, g_maxOutOfMutator, pJsonRoot->valuestring);

        if (pJsonMutator->valuestring)
        {
            free(pJsonMutator->valuestring);
        }

        pJsonMutator->valuestring = malloc(DT_GET_MutatedValueLen(&g_Element[g_tempElementCount]));
        memcpy(pJsonMutator->valuestring, value, DT_GET_MutatedValueLen(&g_Element[g_tempElementCount]));
        
        g_tempElementCount++;
    }

    if (pJsonRoot->type == cJSON_Number)
    {
        pJsonMutator->valueint = *(s32 *)DT_SetGetS32(&g_Element[g_tempElementCount], pJsonRoot->valueint);
        pJsonMutator->valuedouble = pJsonMutator->valueint;
        g_tempElementCount++;
    }

    return;
}

void DoJsonAnalyzer(SBinElement* temp)
{
    SXMLElement* tempXml = temp->mutatorElement->xmlElement;

    if(tempXml->analyzerIsInit == 0)
    {
        tempXml->analyzerIsInit = 1;
        tempXml->analyzerValue1= (char*)cJSON_Parse(temp->defaultStringValue);
        tempXml->analyzerValue2= (char*)cJSON_Parse(temp->defaultStringValue);
        g_tempJson[g_tempJsonNumber++] = (cJSON*)tempXml->analyzerValue1;
        g_tempJson[g_tempJsonNumber++] = (cJSON*)tempXml->analyzerValue2;
    }

    DoJsonAnalyzerMutator((cJSON*)tempXml->analyzerValue1, (cJSON*)tempXml->analyzerValue2);

    char * p = cJSON_Print((cJSON*)tempXml->analyzerValue2);
    int len = strlen(p) + 1;
    g_onerunMemory[g_onerunMemoryCount++] = p;
    
    temp->mutaterValue = p;
    temp->mutaterLength = len;
    
    if (tempXml->isnullTerminated != 1)
    {
        if (temp->mutaterLength > 0)
        {            
            temp->mutaterLength = temp->mutaterLength - 1;
        }
    }
} 

void JsonAnalyzerClean(void)
{
    int i = 0;
    for (i = 0; i < g_tempJsonNumber; i++)
    {
        cJSON_Delete(g_tempJson[i]); 
    }
    g_tempJsonNumber = 0;
}

#ifdef __cplusplus
}
#endif
