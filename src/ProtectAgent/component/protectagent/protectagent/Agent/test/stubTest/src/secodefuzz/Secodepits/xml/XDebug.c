/*
版权所有 (c) 华为技术有限公司 2012-2018


debug控制文件
*/
#include "XML.h"

#ifdef __cplusplus
extern "C" {
#endif

int g_onOffDebugParseAssociated = 0;
int g_onOffDebugGetBinBuf = 0;
int g_onOffDebugParseBin = 0;
int g_onOffDebugParseDataModel=0;
int g_onOffDebugParseStateModel=0;
int g_onOffDebugParseTestModel=0;
int g_onOffDebugParseXml =0;
int g_onOffDebugPublisher = 0;
int g_onOffDebugDoRelation = 0;
int g_onOffDebugDoFixup = 0;
int g_onOffDebugDoTransformer = 0;
int g_onOffDebugMutatorElement = 0;
int g_onOffDebugMutatorPits = 0;

void DoDebugOpen(char* debug)
{
    if (debug == NULL)
    {
        return;
    }

    if (MyMemmem(debug, strlen(debug), "g_onOffDebugParseXml", strlen("g_onOffDebugParseXml")))
    {
        g_onOffDebugParseXml = 1;
    }
    
    if (MyMemmem(debug, strlen(debug), "g_onOffDebugParseTestModel", strlen("g_onOffDebugParseTestModel")))
    {
        g_onOffDebugParseTestModel = 1;
    }

    if (MyMemmem(debug, strlen(debug), "g_onOffDebugParseStateModel", strlen("g_onOffDebugParseStateModel")))
    {
        g_onOffDebugParseStateModel =1;
    }

    if (MyMemmem(debug, strlen(debug), "g_onOffDebugParseDataModel", strlen("g_onOffDebugParseDataModel")))
    {
        g_onOffDebugParseDataModel = 1;
    }

    if (MyMemmem(debug, strlen(debug), "g_onOffDebugParseAssociated", strlen("g_onOffDebugParseAssociated")))
    {
        g_onOffDebugParseAssociated = 1;
    }

    if (MyMemmem(debug, strlen(debug), "g_onOffDebugGetBinBuf", strlen("g_onOffDebugGetBinBuf")))
    {
        g_onOffDebugGetBinBuf = 1;
    }

    if (MyMemmem(debug, strlen(debug), "g_onOffDebugParseBin", strlen("g_onOffDebugParseBin")))
    {
        g_onOffDebugParseBin = 1;
    }

    if (MyMemmem(debug, strlen(debug), "g_onOffDebugMutatorElement", strlen("g_onOffDebugMutatorElement")))
    {
        g_onOffDebugMutatorElement = 1;
    }

    if (MyMemmem(debug, strlen(debug), "g_onOffDebugDoRelation", strlen("g_onOffDebugDoRelation")))
    {
        g_onOffDebugDoRelation = 1;
    }

    if (MyMemmem(debug, strlen(debug), "g_onOffDebugDoFixup", strlen("g_onOffDebugDoFixup")))
    {
        g_onOffDebugDoFixup = 1;
    }

    if (MyMemmem(debug, strlen(debug), "g_onOffDebugDoTransformer", strlen("g_onOffDebugDoTransformer")))
    {
        g_onOffDebugDoTransformer = 1;
    }

    if (MyMemmem(debug, strlen(debug), "g_onOffDebugMutatorPits", strlen("g_onOffDebugMutatorPits")))
    {
        g_onOffDebugMutatorPits = 1;
    }

    if (MyMemmem(debug, strlen(debug), "g_onOffDebugPublisher", strlen("g_onOffDebugPublisher")))
    {
        g_onOffDebugPublisher = 1;
    }
    
    if (MyMemmem(debug, strlen(debug), "all", strlen("all")))
    {
        g_onOffDebugParseXml = 1;
        g_onOffDebugParseTestModel = 1;
        g_onOffDebugParseStateModel = 1;
        g_onOffDebugParseDataModel = 1;
        g_onOffDebugParseAssociated = 1;
        g_onOffDebugGetBinBuf = 1;
        g_onOffDebugParseBin = 1;
        g_onOffDebugMutatorElement = 1;
        g_onOffDebugDoRelation = 1;
        g_onOffDebugDoFixup = 1;
        g_onOffDebugDoTransformer = 1;
        g_onOffDebugMutatorPits = 1;
        g_onOffDebugPublisher = 1;
    }

    if (MyMemmem(debug, strlen(debug), "null", strlen("null")))
    {
        g_onOffDebugParseXml = 0;
        g_onOffDebugParseTestModel = 0;
        g_onOffDebugParseStateModel = 0;
        g_onOffDebugParseDataModel = 0;
        g_onOffDebugParseAssociated = 0;
        g_onOffDebugGetBinBuf = 0;
        g_onOffDebugParseBin = 0;
        g_onOffDebugMutatorElement = 0;
        g_onOffDebugDoRelation = 0;
        g_onOffDebugDoFixup = 0;
        g_onOffDebugDoTransformer = 0;
        g_onOffDebugMutatorPits = 0;
        g_onOffDebugPublisher = 0;
    }

    //这里可以搞个简单模式，挑几个有用的打印
    //(MyMemmem(debug , strlen(debug) ,"all" , strlen("all")))
    {

    }
}

#ifdef __cplusplus
}
#endif
