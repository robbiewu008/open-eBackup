/*
版权所有 (c) 华为技术有限公司 2012-2018

// 为了对外开源，使用这个文件，可以

// 删除以下文件
XParsePits_TestModel.c
XParsePits_StateModel.c

XParseXml.c
XParseXml_TestModel.c
XParseXml_StateModel.c
XParseXml_DataModel.c

XDoState.c
XDoAction.c
XDoPublisher.c

// 删除一下函数
DT_Pits_ParsePits
DT_Pits_DoState

// pits 只保留HW结尾

// debug清理部分

// 修改makefile

也需要把这段注释删除

*/

#include "XML.h"

#ifdef __cplusplus
extern "C" {
#endif

static SStateElement* tempHW[MAX_DATAMODEL_COUNT] = {0};
static int g_dataModelCount = 0;

static void GetAllXmlElementHW(hw_xmlNodePtr root)
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
                printf("****ParseXml DataModel %s  Start\r\n", temp->name);  
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
                printf("****ParseXml DataModel %s  End\r\n", temp->name);  
                printf("**************************************\r\n");  
            }
        }
        
        child = child->next;  
    }

    return;
}
 
int ParseXmlHW(char* docName, char* dataModelName, int isfirst) 
{  
    hw_xmlDocPtr doc;  
    hw_xmlNodePtr root; 
    
    if (isfirst == 1)
    {
        g_dataModelCount = 0;
    }
    
    int id = g_dataModelCount;
    g_dataModelCount++;

    if (g_onOffDebugParseXml)
    {
        printf("**************************************\r\n");  
        printf("!!!!!!!!!!!!!!!~~~~~~Parse xmlHW file %s  ~~~~~~!!!!!!!!!!!!!!\r\n", docName);  
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
    GetAllXmlElementHW(root);

    tempHW[id] = malloc(sizeof(SStateElement));
    g_testcaseMemory[g_testcaseMemoryCount++] = (char*)tempHW[id];
    Hw1Memset(tempHW[id], 0, sizeof(SStateElement));

    {
        //
        tempHW[id]->mutatorElement = ParseDataModel(root, dataModelName);

        // 搞定relation, fixup等关系字段之间的联系
        ParseAssociated(tempHW[id]->mutatorElement);

        // char * buf = NULL;
        // int length =0;

        // 以用户调用接口设置优先
        if (g_binName)
        {            
            GetBinBuf(g_binName, &(tempHW[id]->binBuf), &tempHW[id]->binLength);
        }
        else if (g_binBuf)
        {
            tempHW[id]->binBuf = g_binBuf;
            tempHW[id]->binLength = g_binBufLength;
        }

        // 解析bin,得到数据元素的默认数据	
        // 如果bin为null,从数据模型得到数据
        tempHW[id]->binElement = ParseBin(tempHW[id]->mutatorElement, tempHW[id]->binBuf, tempHW[id]->binLength);

        tempHW[id]->outBuf = malloc(OUT_BUF_MAX_LENGTH);
        g_testcaseMemory[g_testcaseMemoryCount++] = tempHW[id]->outBuf;
    }

    return id;  
}  


 void DoActionHW(int id)
{
    //得到变异数据
    GetMutatorElementValue(tempHW[id]->binElement);

    //处理Padding
    DoPadding(tempHW[id]->binElement);

    //处理relation
    DoRelation(tempHW[id]->binElement);

    //处理fixup
    DoFixup(tempHW[id]->binElement);

    //处理transformer
    DoTransformer(tempHW[id]->binElement);

    //组合变异值,得到输出buf
    tempHW[id]->outLength = GetPitsBuf(tempHW[id]->binElement, tempHW[id]->outBuf);

    g_publisherBuf = tempHW[id]->outBuf;
    g_publisherBufLen = tempHW[id]->outLength;

}

#ifdef __cplusplus
}
#endif
