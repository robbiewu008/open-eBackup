/*
��Ȩ���� (c) ��Ϊ�������޹�˾ 2012-2018


������������ģ�ͣ�����xml�ṹ�壬ÿ���ڵ�һһ��Ӧ

//�õ�xmlԪ�أ� ������������ģ�ͣ�   ����xmlNodePtr�����ṹ��������g_xml_element����ֵ��_private

//���е�xml�﷨�������ֹ��ȫ���⿪����g_xml_element

//֮��Ĵ����������xml�﷨���Ա��Ժ��滻�﷨�滻����ģ�ͽṹ����
*/

#include "../XML.h"

#ifdef __cplusplus
extern "C" {
#endif

static int g_nameCountD = 1;

//������ȿ�����Ҫ����
static u8 outBuf[1024];

static s64 GetPositionValue(hw_xmlNodePtr node)
{
    char* temp_position = XmlGetGetProp(node, "position");

    //�����ܴ���
    if (temp_position == NULL)
    {
        xml_assert(0, "not found position");
        return 0;
    }

    int temp_number = atol(temp_position);
    return temp_number;
}

static s64 GetNumberValue(hw_xmlNodePtr node)
{
    u8* temp_value = (u8 *)XmlGetGetProp(node, "value");

    //����ģ����û��ֵ��Ĭ��Ϊ0
    if (temp_value == NULL)
    {   
        return 0;
    }

    u8* temp_valuetype = (u8 *)XmlGetGetProp(node, "valueType");

    if (( temp_valuetype != NULL) && (strcmp((char *)temp_valuetype, "hex") == 0) )
    {
        Hw1Memset(outBuf, 0, 1024);
        if (temp_value != NULL)
        {      
            TrimSpace(temp_value, outBuf);
        }

        s64 temp_number = Hex2Dec(outBuf);
        return temp_number;
    }

    if ((strlen((char *)temp_value) > 2) && (memcmp(temp_value, "0x", 2) == 0) )
    {
        Hw1Memset(outBuf, 0, 1024);
        if (temp_value != NULL)
        {      
            TrimSpace(temp_value + 2, outBuf);
        }

        s64 temp_number = Hex2Dec(outBuf);
        return temp_number;
    }

    int temp_number = atol((char *)temp_value);
    return temp_number;
}

static u8* GetStringValue(hw_xmlNodePtr node)
{
    u8* temp_value = (u8 *)XmlGetGetProp(node, "value");

    //�����ַ��������û��ֵ��Ϊnull
    return temp_value;
}

static u8* GetBlobValue(hw_xmlNodePtr node, int* length)
{
    u8* temp_value = (u8 *)XmlGetGetProp(node, "value");
    u8* temp_valuetype = (u8 *)XmlGetGetProp(node, "valueType");

    if ((temp_valuetype != NULL) && (strcmp((char *)temp_valuetype, "hex") == 0) && (temp_value != NULL))
    {
        Hw1Memset(outBuf, 0, 1024);
        if (temp_value != NULL)
        {      
            TrimSpace(temp_value, outBuf);
        }


        int len = strlen((char *)outBuf) / 2;
        u8 *buf = malloc( len);
        g_testcaseMemory[g_testcaseMemoryCount++] = (char *)buf;
        HexStrToByte((char*)outBuf, (unsigned char*)buf, (int)strlen((char*)outBuf));
        *length = len;

        return buf;
    }

    //�Ժ��޸�
    if ((temp_valuetype!=NULL) && (strcmp((char*)temp_valuetype, "ipv4") == 0) && (temp_value != NULL))
    {
        Hw1Memset(outBuf, 0, 1024);
        if (temp_value != NULL)
        {      
            TrimSpace(temp_value, outBuf);
        }

        //ipv4��blob���ռ4���ֽ�
        u8 *buf = malloc( 4);
        g_testcaseMemory[g_testcaseMemoryCount++] = (char*)buf;
        u32* aaaa = (u32*)buf;
        *aaaa = FromIpstrToUint((char*)temp_value);
        *length = 4;

        return buf;
    }

    if ((temp_valuetype == NULL) && (temp_value != NULL))
    {
        *length = strlen((char *)temp_value);
        return temp_value;
    }

    return outBuf;
}

static int GetNullTerminated(hw_xmlNodePtr node)
{
    char* temp_value = XmlGetGetProp(node, "nullTerminated");

    if (temp_value == NULL)
    {   
        return 0;
    }

    if (strcmp(temp_value, "true") == 0)
    {   
        return 1;
    }

    return 0;
}

static int GetSize(hw_xmlNodePtr node)
{
    char* temp_value = XmlGetGetProp(node, "size");

    if (temp_value == NULL)
    {   
        return 0;
    }

    int temp_number = atol(temp_value);

    return temp_number;
}


static int GetLength(hw_xmlNodePtr node)
{
    char* temp_value = XmlGetGetProp(node, "length");

    //Ĭ��Ϊ0���Ǿ���Ҫ�Ժ�ֵ��
    if (temp_value == NULL)
    {   
        return 0;
    }

    int temp_number = atol(temp_value);

    return temp_number;
}

static int GetToken(hw_xmlNodePtr node)
{
    char* temp_value = XmlGetGetProp(node, "token");

    //Ĭ��Ϊ��
    if (temp_value == NULL)
    {   
        return 0;
    }

    if (strcmp(temp_value, "true") == 0)
    {   
        return 1;
    }

    return 0;
}

static int GetMutable(hw_xmlNodePtr node)
{
    char* temp_value = XmlGetGetProp(node, "mutable");

    //Ĭ��Ϊ��
    if (temp_value == NULL)
    {   
        return 1;
    }

    if (strcmp(temp_value, "false") == 0)
    {   
        return 0;
    }

    return 1;
}

static int Getsigned(hw_xmlNodePtr node)
{

    char* temp_value = XmlGetGetProp(node, "signed");

    //Ĭ��Ϊ��
    if (temp_value == NULL)
    {   
        return 1;
    }

    if (strcmp(temp_value, "false") == 0)
    {   
        return 0;
    }

    return 1;
}

static int GetstringNumber(hw_xmlNodePtr node)
{
    char* temp_value = XmlGetGetProp(node, "stringNumber");

    //Ĭ��Ϊ��
    if (temp_value == NULL)
    {   
        return 0;
    }

    if (strcmp(temp_value, "true") == 0)
    {   
        return 1;
    }

    return 0;
}

static u32 GetOccurs(hw_xmlNodePtr node)
{
    char* temp_value = XmlGetGetProp(node, "occurs");

    if (temp_value == NULL)
    {   
        return 0xfffffff;
    }

    int temp_number = atol(temp_value);

    return temp_number;
}

static u32 GetMinOccurs(hw_xmlNodePtr node)
{
    char* temp_value = XmlGetGetProp(node, "minOccurs");

    if (temp_value == NULL)
    {   
        return 0xfffffff;
    }

    int temp_number = atol(temp_value);

    if (temp_number == -1)
    {   
        temp_number = 0;
    }

    return temp_number;
}

static u32 GetMaxOccurs(hw_xmlNodePtr node)
{
    char* temp_value = XmlGetGetProp(node, "maxOccurs");

    if (temp_value == NULL)
    {   
        return 0xfffffff;
    }

    int temp_number = atol(temp_value);

    return temp_number;
}

static char* Getconstraint(hw_xmlNodePtr node)
{
    char* temp_value = XmlGetGetProp(node, "constraint");

    return temp_value;
}

static u32 DoOccurs(hw_xmlNodePtr node, SXMLElement *temp)
{
    temp->occurs = GetOccurs(node);
    temp->minOccurs = GetMinOccurs(node);
    temp->maxOccurs = GetMaxOccurs(node);


    //һ����Ч��Ϊ����
    if ((temp->occurs != 0xfffffff) || (temp->minOccurs != 0xfffffff) || (temp->maxOccurs != 0xfffffff))
    {
        temp->isOccurs = 1;

        if (g_onOffDebugParseXml)
        {
            printf("		occurs =  %d  \n", temp->occurs);
            printf("		minOccurs =  %d  \n", temp->minOccurs);
            printf("		maxOccurs =  %d  \n", temp->maxOccurs);
        }
    }


    //�����ں��ʹ��minOccurs��maxOccurs������
    if (temp->occurs != 0xfffffff)
    {
        temp->minOccurs = temp->occurs;
        temp->maxOccurs = temp->occurs;
    }
    else
    {
        if ((temp->minOccurs == 0) || (temp->minOccurs == -1))
        {      
            temp->minOccurs = 0;
        }
        else if (temp->minOccurs == 0xfffffff)
        {
            temp->minOccurs = 1;//���û�У��������飬Ĭ��Ϊ1
        }
    }

    if (temp->minOccurs > temp->maxOccurs)
    {
        xml_assert(0, "minOccurs > maxOccurs erro");
    }

    return 1;
}


static u32 DoAssociated(hw_xmlNodePtr node, SXMLElement *temp)
{
    hw_xmlNodePtr child_child = node->children;

    while (child_child != NULL)
    {
        if (strcmp((char*)child_child->name, "Analyzer") == 0)
        {
            temp->isAnalyzer = 1;
            temp->analyzerClassName = XmlGetGetProp(child_child, "class");
        }
        
        if (strcmp((char*)child_child->name, "Relation") == 0)
        {
            temp->isRelation = 1;
            temp->relationType = XmlGetGetProp(child_child, "type");
            temp->RelationOf = XmlGetGetProp(child_child, "of");
            temp->expressionGet = XmlGetGetProp(child_child, "expressionGet");
            temp->expressionSet = XmlGetGetProp(child_child, "expressionSet");
        }

        if (strcmp((char*)child_child->name, "Fixup") == 0)
        {
            temp->isFixup = 1;
            temp->className = XmlGetGetProp(child_child, "class");

            hw_xmlNodePtr sunzi = child_child->children;
            int i = 0;
            while (sunzi != NULL)
            {
                if (strcmp((char*)sunzi->name, "Param") != 0)
                {
                    sunzi = sunzi->next; 
                    continue;
                }

                temp->paramName[i] = XmlGetGetProp(sunzi, "name");
                temp->paramValue[i] = XmlGetGetProp(sunzi, "value");

                i++;
                sunzi = sunzi->next;
            }
        }

        if (strcmp((char*)child_child->name, "Transformer") == 0)
        {
            temp->isTransformer = 1;
            temp->className1 = XmlGetGetProp(child_child, "class");

            hw_xmlNodePtr sunzi = child_child->children;
            int i = 0;
            while (sunzi != NULL)
            {
                if (strcmp((char*)sunzi->name, "Param") != 0)
                {
                    sunzi = sunzi->next; 
                    continue;
                }

                temp->paramName1[i] = XmlGetGetProp(sunzi, "name");
                temp->paramValue1[i] = XmlGetGetProp(sunzi, "value");

                i++;
                sunzi = sunzi->next;
            }
        }
        child_child = child_child->next;
    }

    return 1;
}

void ParseXmlDataModel(hw_xmlNodePtr root, hw_xmlNodePtr xmlParent)
{
    hw_xmlNodePtr child; 
    child = xmlParent->children;

    SXMLElement *temp;

    while (child != NULL) 
    {  
        // test����ֱ�Ӻ��ԣ���ʱû������ɶ�ô�
        if (strcmp((char*)child->name, "text") == 0)
        {
            child = child->next; 
            continue;
        }

        // ����xmlԪ���ڴ棬ֱ�ӹҽ���node����
        temp = malloc(sizeof(SXMLElement));
        g_testcaseMemory[g_testcaseMemoryCount++] = (char*)temp;
        Hw1Memset(temp, 0, sizeof(SXMLElement));

        //ָ�뻥ָ��������������
        temp->xmlNode = child;
        child->_private = temp;

        /*******************************
        ���ԵĶ��������ϱ�
        ********************************/
        temp->name = XmlGetGetProp(child, "name");
        temp->typeName = (char*)child->name;

        // û�����֣�����Ĭ�ϵ����֣����ܺ��ֹ������ĳ�ͻ��������
        // Ĭ�����ֲ�ȡԪ�����ͼ����ֵķ�ʽ
        if (temp->name == NULL)
        {
            temp->name = malloc(40);
            g_testcaseMemory[g_testcaseMemoryCount++] = temp->name;
            sprintf(temp->name,"%s_%03d", temp->typeName, g_nameCountD++);
        }

        if (g_onOffDebugParseXml)
        {      
            printf("	----Start Parse type=%s ----name=\"%s\"  \n", temp->typeName , temp->name );  
        }

        temp->size = GetSize(child);

        if (g_onOffDebugParseXml)
        {      
            printf("		GetSize =  %d  \n", temp->size);  
        }

        temp->length = GetLength(child);

        if(g_onOffDebugParseXml)
        {      
            printf("		GetLength =  %d  \n", temp->length);  
        }

        temp->isToken = GetToken(child);

        if (g_onOffDebugParseXml)
        {      
            printf("		GetToken =  %d  \n", temp->isToken);
        }

        temp->isMutable = GetMutable(child);

        if (g_onOffDebugParseXml)
        {      
            printf("		GetMutable =  %d  \n", temp->isMutable);
        }

        temp->isSigned = Getsigned(child);

        if (g_onOffDebugParseXml)
        {      
            printf("		Getsigned =  %d  \n", temp->isSigned);
        }

        temp->isStringNumber = GetstringNumber(child);

        if (g_onOffDebugParseXml)
        {      
            printf("		GetstringNumber =  %d  \n", temp->isStringNumber);
        }

        temp->constraint = Getconstraint(child);

        // ���������Ǽ�������
        DoOccurs(child, temp);

        // ��ȡ�ֶι�ϵ֮��Ĳ���������relation,fixup,transform
        DoAssociated(child, temp);

        /*******************************
        ���ԵĶ����������
        *******************************/
        if (strcmp((char*)child->name, "Number") == 0)
        {
            temp->numberValue = GetNumberValue(child);

            if (g_onOffDebugParseXml)
            {         
                printf("		GetNumberValue =  %ld  \n", temp->numberValue);  
            }
        }


        if (strcmp((char*)child->name, "Flag") == 0)
        {
            temp->position = GetPositionValue(child);

            if (g_onOffDebugParseXml)
            {         
                printf("		GetPositionValue =  %d  \n", temp->position);  
            }


            temp->numberValue = GetNumberValue(child);

            if (g_onOffDebugParseXml)
            {         
                printf("		getFlagValue =  %ld  \n", temp->numberValue);  
            }
        }

        if (strcmp((char*)child->name, "String") == 0)
        {
            temp->isnullTerminated = GetNullTerminated(child);

            if (g_onOffDebugParseXml)
            {         
                printf("		GetNullTerminated =  %d  \n", temp->isnullTerminated);  
            }

            char *temp_string = (char*)GetStringValue(child);

            int length = 0;
            if (temp_string != NULL)
            {         
                length = strlen(temp_string);
            }

            if (temp->length > 0)
            {
                // �жϳ��ȵ���Ч��,length����С��value�ĳ���
                if (temp->isnullTerminated)
                {
                    if (temp->length < (length + 1))
                    {               
                        xml_assert(0,"length is not enough");
                    }
                }
                else
                {
                    if (temp->length < length)
                    {               
                        xml_assert(0, "length is not enough");
                    }
                }
            }
            else
            {
                // ���token��������û�г��ȣ��򳤶�����value
                if (temp->isToken)
                {
                    if (temp->isnullTerminated)
                    {               
                        temp->length = length + 1;
                    }
                    else
                    {               
                        temp->length = length;
                    }
                }
            }

            // ע�⣬��������û�У��ȴ��Ժ�ֵ
            // length��value���Ȳ�ƥ��Ļ���������0���,Ӧ���ڱ��������wenhao
            if (temp->length > 0)
            {
                temp->stringValue = malloc(temp->length + 1);
                g_testcaseMemory[g_testcaseMemoryCount++] = (char*)temp->stringValue;
                memset(temp->stringValue, 0, temp->length + 1);

                if (temp_string != NULL)
                {
                    memcpy(temp->stringValue, temp_string, length);
                }
            }
            // ֻ��ֵ��û�г��ȣ���û��bin��ʱ�������
            else if (length > 0)
            {
                temp->stringValue = malloc(length + 1);
                g_testcaseMemory[g_testcaseMemoryCount++] = (char*)temp->stringValue;
                memset(temp->stringValue, 0, length + 1);

                if (temp_string != NULL)
                {
                    memcpy(temp->stringValue, temp_string, length);
                }
            }

            if (g_onOffDebugParseXml)
            {         
                printf("		GetStringValue =  %s  \n", temp->stringValue);
            }
        }

        if (strcmp((char*)child->name, "Blob") == 0)
        {
            int length = 0;
            temp->blobValue = GetBlobValue(child, &length);

            if (temp->isToken == 1)
            {
                if ((temp->length != 0) && (temp->length != length))
                {            
                    xml_assert(0, "length is mismatch");
                }

                temp->length = length;
            }
            else if (temp->length)
            {
                // xml value�ĳ���һ��Ҫ��lengthС
                if (temp->length<length)
                {            
                    xml_assert(0, "length is mismatch");
                }
            }

            temp->xmlValueLength = length;

            if (g_onOffDebugParseXml)
            {         
                printf("		GetBlobValue =  \n");
            }

            // ��Ȼ����û��ֵ����
            if ((g_onOffDebugParseXml) && (temp->blobValue != NULL))
            {         
                HexDump(temp->blobValue, temp->length);
            }
        }

        if (strcmp((char*)child->name, "Padding") == 0)
        {
            temp->alignment = XmlGetGetProp(child, "alignment");
            temp->alignedTo = XmlGetGetProp(child, "alignedTo");

            if (g_onOffDebugParseXml)
            {         
                if (temp->alignment)
                {            
                    printf("		Padding alignment = %s \n", temp->alignment);
                }
            }

            if (g_onOffDebugParseXml)
            {         
                if (temp->alignedTo)
                {            
                    printf("		Padding alignedTo = %s \n", temp->alignedTo);
                }
            }
        }

        if (strcmp((char*)child->name, "Block") == 0)
        {
            if (XmlGetGetProp(child,"ref")!= NULL)
            {
                temp->isHasRef = 1;
                temp->xmlRef = XmlGetDataModelByName(root, XmlGetGetProp(child, "ref"));
            }
            else
            {         
                temp->isHasRef = 0;
            }

            ParseXmlDataModel(root, child);
        }

        if (strcmp((char*)child->name, "Flags") == 0)
        {
            ParseXmlDataModel(root, child);
        }

        // choice�����"Ӧ��"֧�����飬ref,����
        if (strcmp((char*)child->name, "Choice") == 0)
        {
            ParseXmlDataModel(root ,child);
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
