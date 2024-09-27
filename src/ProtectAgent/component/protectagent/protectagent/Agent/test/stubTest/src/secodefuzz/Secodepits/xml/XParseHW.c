/*
��Ȩ���� (c) ��Ϊ�������޹�˾ 2012-2018

// Ϊ�˶��⿪Դ��ʹ������ļ�������

// ɾ�������ļ�
XParsePits_TestModel.c
XParsePits_StateModel.c

XParseXml.c
XParseXml_TestModel.c
XParseXml_StateModel.c
XParseXml_DataModel.c

XDoState.c
XDoAction.c
XDoPublisher.c

// ɾ��һ�º���
DT_Pits_ParsePits
DT_Pits_DoState

// pits ֻ����HW��β

// debug������

// �޸�makefile

Ҳ��Ҫ�����ע��ɾ��

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
        // ��������DataModel��һ���Խ�����ɣ��Ժ�Ͳ���Ҫ��
        if (strcmp((char*)child->name, "DataModel") == 0)
        {
            temp = malloc(sizeof(SXMLElement));
            g_testcaseMemory[g_testcaseMemoryCount++] = (char*)temp;
            Hw1Memset(temp, 0, sizeof(SXMLElement));

            //ָ�뻥ָ��������������
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

            // �õ�ref��Ϣ,����ģ�ͽ�֧��ref����ò��
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

    // ����xml�﷨
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

    // ��������ģ���﷨
    GetAllXmlElementHW(root);

    tempHW[id] = malloc(sizeof(SStateElement));
    g_testcaseMemory[g_testcaseMemoryCount++] = (char*)tempHW[id];
    Hw1Memset(tempHW[id], 0, sizeof(SStateElement));

    {
        //
        tempHW[id]->mutatorElement = ParseDataModel(root, dataModelName);

        // �㶨relation, fixup�ȹ�ϵ�ֶ�֮�����ϵ
        ParseAssociated(tempHW[id]->mutatorElement);

        // char * buf = NULL;
        // int length =0;

        // ���û����ýӿ���������
        if (g_binName)
        {            
            GetBinBuf(g_binName, &(tempHW[id]->binBuf), &tempHW[id]->binLength);
        }
        else if (g_binBuf)
        {
            tempHW[id]->binBuf = g_binBuf;
            tempHW[id]->binLength = g_binBufLength;
        }

        // ����bin,�õ�����Ԫ�ص�Ĭ������	
        // ���binΪnull,������ģ�͵õ�����
        tempHW[id]->binElement = ParseBin(tempHW[id]->mutatorElement, tempHW[id]->binBuf, tempHW[id]->binLength);

        tempHW[id]->outBuf = malloc(OUT_BUF_MAX_LENGTH);
        g_testcaseMemory[g_testcaseMemoryCount++] = tempHW[id]->outBuf;
    }

    return id;  
}  


 void DoActionHW(int id)
{
    //�õ���������
    GetMutatorElementValue(tempHW[id]->binElement);

    //����Padding
    DoPadding(tempHW[id]->binElement);

    //����relation
    DoRelation(tempHW[id]->binElement);

    //����fixup
    DoFixup(tempHW[id]->binElement);

    //����transformer
    DoTransformer(tempHW[id]->binElement);

    //��ϱ���ֵ,�õ����buf
    tempHW[id]->outLength = GetPitsBuf(tempHW[id]->binElement, tempHW[id]->outBuf);

    g_publisherBuf = tempHW[id]->outBuf;
    g_publisherBufLen = tempHW[id]->outLength;

}

#ifdef __cplusplus
}
#endif
