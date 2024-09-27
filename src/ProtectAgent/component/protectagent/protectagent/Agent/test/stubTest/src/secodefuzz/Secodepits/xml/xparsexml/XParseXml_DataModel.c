/*
版权所有 (c) 华为技术有限公司 2012-2018


解析所有数据模型，生成xml结构体，每个节点一一对应

//得到xml元素， 遍历整个数据模型，   保留xmlNodePtr完整结构，并生成g_xml_element，赋值给_private

//所有的xml语法到这里截止，全部解开存入g_xml_element

//之后的处理不允许出现xml语法，以便以后替换语法替换数据模型结构方便
*/

#include "../XML.h"

#ifdef __cplusplus
extern "C" {
#endif

static int g_nameCountD = 1;

//这个长度可能需要增加
static u8 outBuf[1024];

static s64 GetPositionValue(hw_xmlNodePtr node)
{
    char* temp_position = XmlGetGetProp(node, "position");

    //不可能存在
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

    //数据模型里没有值，默认为0
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

    //返回字符串，如果没有值就为null
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

    //以后修改
    if ((temp_valuetype!=NULL) && (strcmp((char*)temp_valuetype, "ipv4") == 0) && (temp_value != NULL))
    {
        Hw1Memset(outBuf, 0, 1024);
        if (temp_value != NULL)
        {      
            TrimSpace(temp_value, outBuf);
        }

        //ipv4在blob里就占4个字节
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

    //默认为0，那就需要以后赋值了
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

    //默认为假
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

    //默认为真
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

    //默认为真
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

    //默认为假
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


    //一个有效则为数组
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


    //我们在后边使用minOccurs和maxOccurs来计算
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
            temp->minOccurs = 1;//如果没有，还是数组，默认为1
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
        // test类型直接忽略，暂时没看到有啥用处
        if (strcmp((char*)child->name, "text") == 0)
        {
            child = child->next; 
            continue;
        }

        // 分配xml元素内存，直接挂接在node树上
        temp = malloc(sizeof(SXMLElement));
        g_testcaseMemory[g_testcaseMemoryCount++] = (char*)temp;
        Hw1Memset(temp, 0, sizeof(SXMLElement));

        //指针互指，查找起来方便
        temp->xmlNode = child;
        child->_private = temp;

        /*******************************
        共性的东西放在上边
        ********************************/
        temp->name = XmlGetGetProp(child, "name");
        temp->typeName = (char*)child->name;

        // 没有名字，给个默认的名字，可能和手工命名的冲突，不管了
        // 默认名字采取元素类型加数字的方式
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

        // 处理数组那几个参数
        DoOccurs(child, temp);

        // 获取字段关系之类的参数，比如relation,fixup,transform
        DoAssociated(child, temp);

        /*******************************
        特性的东西放在这边
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
                // 判断长度的有效性,length不能小于value的长度
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
                // 如果token成立，还没有长度，则长度来自value
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

            // 注意，长度允许没有，等待以后赋值
            // length与value长度不匹配的话，不够用0填充,应该在变异后体现wenhao
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
            // 只有值，没有长度，在没有bin的时候会有用
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
                // xml value的长度一定要比length小
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

            // 当然可以没有值了先
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

        // choice本身不濉"应该"支持数组，ref,哈哈
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
