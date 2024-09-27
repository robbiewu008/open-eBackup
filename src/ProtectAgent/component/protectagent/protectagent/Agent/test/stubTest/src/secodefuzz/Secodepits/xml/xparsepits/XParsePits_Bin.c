/*
��Ȩ���� (c) ��Ϊ�������޹�˾ 2012-2018


����ָ������ģ�ͣ�������Ҫ�����Ԫ�ض������������ref����
*/

#include "../XML.h"

#ifdef __cplusplus
extern "C" {
#endif

static SBinElement* g_temp = NULL;
static SBinElement* g_tempRoot;

// �Ȳ����Լ��ĸ��ף�Ȼ�����ֵܣ�үү�����׵��ֵ�.... ....
static int FoundRecentlyByName55(char *name)
{
    SBinElement* temp = g_tempRoot;
    For_Tree_Start(temp)
    {
        if (temp->xpathName != NULL)
        {      
            if (strcmp(temp->xpathName, name) == 0 )
            {
                return 0;
            }
        }
    }
    For_Tree_End(temp)

    return 1;
}


static int GetIsHasNotSolved()
{
    if (g_temp != NULL)
    {
        // �жϽ������Ԫ�ػ��ڷ�
        return FoundRecentlyByName55(g_temp->solvedXpathName);
    }

    return 0;
}


// ��һ�ּ����ѣ�����
static int NumberCheck(SMutatorElement* tempMutatorElement, int hasbin, char ** buf, int* length)
{
    SXMLElement *tempXml = tempMutatorElement->xmlElement;
    s64 defaultNumberValue;


    if (hasbin)
    {
        // Ŀǰ��֧��32,�Ժ��޸�
        defaultNumberValue = GetNumberBinValue(0, tempXml->size, tempXml->isSigned, (u8*)(*buf));

        // token ����ƥ�䣬������
        if (tempXml->isToken == 1)
        {
            if (tempXml->numberValue != defaultNumberValue)
            {
                if (g_onOffDebugParseBin)
                {            
                    printf("		token is mismatch, xml value is %ld ,bin value is %ld , check failed@@@@\r\n", tempXml->numberValue, defaultNumberValue);
                }

                return R_No;
            }
        }

        // constraint
        if (tempXml->constraint)
        {         
            if (DoConstraint(tempXml) == R_No)
            {
                if (g_onOffDebugParseBin)
                {               
                    printf("		constraint is mismatch , check failed@@@@\r\n");
                }
                return R_No;
            }
        }
    }
    else
    {

    }

    return R_Yes;
}

//��һ�ּ����ѣ�����
static int FlagCheck(SMutatorElement* tempMutatorElement, int hasBin, s64 value)
{
    SXMLElement *tempXml = tempMutatorElement->xmlElement;
    // s64 		default_Number_value;

    if (hasBin)
    {
        // token ����ƥ�䣬������
        if (tempXml->isToken == 1)
        {
            if (tempXml->numberValue != value)
            {
                if (g_onOffDebugParseBin)
                {            
                    printf("		token is mismatch, xml value is %ld ,bin value is %ld , check failed@@@@\r\n", tempXml->numberValue, value);
                }

                return R_No;
            }
        }

        // constraint
        if (tempXml->constraint)
        {         
            if (DoConstraint(tempXml) == R_No)
            {
                if (g_onOffDebugParseBin)
                {               
                    printf("		constraint is mismatch , check failed@@@@\r\n");
                }
                return R_No;
            }
        }
    }
    else
    {

    }

    return R_Yes;
}

// ��һ�ּ����ѣ�����
static int StringCheck(SMutatorElement* tempMutatorElement, int hasBin, char ** buf, int* length)
{
    SXMLElement *tempXml = tempMutatorElement->xmlElement;

    if (hasBin)
    {
        if (GetIsHasNotSolved())
        {
            // ������ȷ����Ԫ�������ǲ������
            if (tempXml->isToken == 0)
            {
                if (g_onOffDebugParseBin)
                {            
                    printf("		two not solved element , check failed@@@@\r\n");
                }

                return R_No;
            }
            else
            {
                // ��token�������ҵ��ַ���
                char *rel = MyMemmem(*buf, *length, (char*)tempXml->stringValue, tempXml->length); 
                if (rel == NULL)
                {
                    if (g_onOffDebugParseBin)
                    {               
                        printf("		not found value because token , check failed@@@@\r\n");
                    }

                    return R_No;
                }
            }
        }
        else
        {
        // ���bin��nullTerminated�Ƿ�����
        if (tempXml->isnullTerminated == 1)
        {
            if ((*buf)[tempXml->length] != 0)
            {
                return R_No;
            }
        }

        // ���token
        if (tempXml->isToken)
        {         
            if (memcmp(tempXml->stringValue, *buf, tempXml->length) != 0)
            {
                if (g_onOffDebugParseBin)
                {               
                    printf("		token is mismatch , check failed@@@@\r\n");
                }

                return R_No;
            }
        }

        // constraint
        if (tempXml->constraint)
        {         
            if (DoConstraint(tempXml) == R_No)
            {
                if (g_onOffDebugParseBin)
                {               
                    printf("		constraint is mismatch , check failed@@@@\r\n");
                }
                return R_No;
            }
        }

        }
    }
    else
    {

    }

    return R_Yes;
}

// ��һ�ּ����ѣ�����
static int BlobCheck(SMutatorElement* tempMutatorElement, int hasBin, char ** buf, int* length)
{
    SXMLElement *tempXml = tempMutatorElement->xmlElement;

    if (hasBin)
    {
        if (GetIsHasNotSolved())
        {
            // ������ȷ����Ԫ�������ǲ������
            if (tempXml->isToken == 0)
            {
                if (g_onOffDebugParseBin)
                {
                    printf("		two not solved element , check failed@@@@\r\n");
                }
                return R_No;
            }
            else
            {
                // ��token�������ҵ��ַ���
                char *rel = MyMemmem(*buf, *length, (char*)tempXml->blobValue, tempXml->length); 
                if (rel == NULL)
                {
                    if (g_onOffDebugParseBin)
                    {
                        printf("		not found blob value because token , check failed@@@@\r\n");

                        printf("		Blob length is %d\r\n", tempXml->length);
                        HexDump(tempXml->blobValue, tempXml->length);
                    }

                    return R_No;
                }
            }
        }
        else
        {
            //���token
            if (tempXml->isToken)
            {         
                if (memcmp(tempXml->blobValue, *buf, tempXml->length) != 0)
                {
                    if (g_onOffDebugParseBin)
                    {               
                        printf("		token is mismatch , check failed@@@@\r\n");
                    }
                    return R_No;
                }
            }

            // constraint
            if (tempXml->constraint)
            {         
                if (DoConstraint(tempXml) == R_No)
                {
                    if (g_onOffDebugParseBin)
                    {               
                        printf("		constraint is mismatch , check failed@@@@\r\n");
                    }
                    return R_No;
                }
            }
        }
    }
    else
    {

    }

    return R_Yes;
}

static s64 GetRelationSize(SBinElement * tempSize)
{
    int len = 0;
    if (strcmp(tempSize->mutatorElement->xmlElement->typeName, "Number") == 0)
    {   
        len = tempSize->defaultNumberValue;
    }
    else if (strcmp(tempSize->mutatorElement->xmlElement->typeName, "Flag") == 0)
    {   
        len = tempSize->defaultNumberValue;
    }
    else if (strcmp(tempSize->mutatorElement->xmlElement->typeName, "String") == 0)
    {   
        len = atol(tempSize->defaultStringValue);
    }
    else
    {   
        xml_assert(0, "type error");
    }

    return  DoExpressionGet(len ,tempSize->mutatorElement->xmlElement->expressionGet);
}


static void DoSolvedElement(SBinElement* tempBin, int length, char* buf)
{
    if (strcmp(g_temp->mutatorElement->xmlElement->typeName, "Blob") == 0)
    {
        g_temp->defaultBlobValue = malloc(length);
        g_testcaseMemory[g_testcaseMemoryCount++] = g_temp->defaultBlobValue;
        memcpy(g_temp->defaultBlobValue, buf, length);
        g_temp->defaultLength = length;
    }
    else // String
    {
        g_temp->defaultStringValue = malloc(length + 1);
        g_testcaseMemory[g_testcaseMemoryCount++] = g_temp->defaultStringValue;
        memcpy(g_temp->defaultStringValue, buf, length);
        g_temp->defaultStringValue[length] = 0;
        g_temp->defaultLength = length;

    }

    memcpy(g_temp->solvedXpathName, tempBin->xpathName, MAX_XPATH_NAME_LENGTH);
}

// �õ�����Ԫ�ض�����
static int getBinElement(SMutatorElement* tempMutatorElement, SBinElement* tempBinElement, int hasBin, char** buf, int *length )
{
    // ʹ��S_Mutator_Element ����ѭ��
    SMutatorElement *child; 

    child = tempMutatorElement->children;

    SXMLElement *tempXml;
    SBinElement * tempBin = NULL;
    int ret = R_Yes;

    while (child != NULL) 
    { 
        u32 tempOccurs = 0;
        tempXml = child->xmlElement;

        char* name = tempXml->typeName;

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

        aaaa:

        if (g_onOffDebugParseBin)
        {
            printf("\r\n----length is left %d,----buf is %p,----type is %s  \n", *length, *buf, name);
        }

        tempXml = child->xmlElement;
        name = tempXml->typeName;

        tempBin = malloc(sizeof(SBinElement));
        g_testcaseMemory[g_testcaseMemoryCount++] = (char*)tempBin;
        Hw1Memset(tempBin, 0, sizeof(SBinElement));

        tempBin->mutatorElement = child;
        tempBin->binOffset = *buf;

        if (tempXml->isOccurs)
        {      
            sprintf(tempBin->xpathName,"%s.%s[%d]", tempBinElement->xpathName, tempXml->name, tempOccurs);
        }
        else
        {      
            sprintf(tempBin->xpathName, "%s.%s", tempBinElement->xpathName, tempXml->name);
        }

        if (g_onOffDebugParseBin)
        {
            printf("    xpath_name =  %s  \n", tempBin->xpathName);
        }

        if (tempXml->isOccurs)
        {
            // �������Ϊ0,�Ǻ�û����һ����
            if ((tempXml->minOccurs == 0) && (tempXml->maxOccurs == 0))
            {
                if (g_onOffDebugParseBin)
                {
                    printf("    	occurs is  0,goto next element ...\r\n");
                }

                goto next;
            }

            // û��bin,����minOccurs����
            if ((tempXml->minOccurs == 0) && (hasBin == 0))
            {
                if (g_onOffDebugParseBin)
                {
                    printf("    	no bin,minOccurs is 0,goto next element ...\r\n");
                }
                goto next;
            }
        }
        if (hasBin)
        {
            // ��������飬relation count,���֮
            if (child->lastIsRelationC == 1)
            {
                // �õ�relation�ĳ���
                SBinElement *relationCount = BinElementFoundRecentlyByXpathName(tempBinElement, child->lastRelationC);

                if (relationCount == NULL)
                {            
                    xml_assert(0 , "relation element(count) not found");
                }

                int solvedCount = DoExpressionGet(relationCount->defaultNumberValue, relationCount->mutatorElement->xmlElement->expressionGet);

                tempBin->defaultCount = solvedCount;

                if (g_onOffDebugParseBin)
                {            
                    printf("		solved count is %d by relation %s\r\n", tempBin->defaultCount, relationCount->xpathName);
                }

                // �����0�ǲ��Ǿ�ֱ��continue��
                if (tempBin->defaultCount == 0)
                {
                    if (g_onOffDebugParseBin)
                    {
                        printf("    	solved occurs is  0,goto next element ...\r\n");
                    }

                    goto next;;
                }
            }

            // relation size���һ��Ԫ��
            if (child->lastIsRelationS == 1)
            {
                // �õ�relation�ĳ���
                SBinElement *relationSize = BinElementFoundRecentlyByXpathName(tempBinElement, child->lastRelationS);
                SBinElement *relationOf = BinElementFoundRecentlyByXpathName(tempBinElement, child->lastRelationOfS);

                // Ҳ����Ը�һ��
                if (strcmp(tempBin->mutatorElement->xpathName, child->lastRelationOfS) == 0)
                {            
                    relationOf = tempBin;
                }

                if ((relationSize == NULL) || (relationOf == NULL))
                {            
                    xml_assert(0, "relation element(size) or relation of element not found");
                }

                int solvedLength = GetRelationSize(relationSize);

                int tempLength = solvedLength - (int)(*buf - relationOf->binOffset) ;

                tempBin->defaultLength = tempLength;
                tempBin->defaultLengthHasSolve = 1;

                if (g_onOffDebugParseBin)
                {            
                    printf("		solved1 length is %d by relation %s\r\n", tempBin->defaultLength, relationSize->xpathName);
                }
            }

            // ���block�ȣ�relation size, �����ֶβ����Լ��Ķ��ӣ������ھͿ��Եõ����ȵ�
            if ((child->relationOfIs) && (child->isRelationParentAndChild == 0))
            {
                if (child->relationOfRelationName)
                {
                    SBinElement *relationSize = BinElementFoundRecentlyByXpathName(tempBinElement, child->relationOfRelationName);

                    if (relationSize == NULL)
                    {               
                        xml_assert(0 , "relation element(size) not found");
                    }


                    tempBin->defaultLength = GetRelationSize(relationSize);
                    tempBin->defaultLengthHasSolve = 1;

                    if (g_onOffDebugParseBin)
                    {               
                        printf("		solved2 length is %d by relation %s\r\n", tempBin->defaultLength, relationSize->xpathName);
                    }
                }

                // ��Ҫ���������ô����ô?
            }

            if (tempBin->defaultLength < 0)
            {
                if (g_onOffDebugParseBin)
                {            
                    printf("		solved length is %d < 0 , check failed@@@@\r\n", tempBin->defaultLength);
                }

                ret = R_No;
                goto ERR;
            }

            // ���ȼ��
            if (tempXml->length != 0)
            {
                if (tempBin->defaultLength != 0)
                {
                    // �����ĳ��ȱ��Լ��ĳ��ȴ��ǿ��ܵģ���������Ԫ����������
                    if (tempXml->length <= tempBin->defaultLength)
                    {
                        tempBin->defaultLength = tempXml->length;
                    }
                    else
                    {
                        if (g_onOffDebugParseBin)
                        {
                            printf("		solved length is %d < xml length is %d , check failed@@@@\r\n", tempBin->defaultLength, tempXml->length);
                        }

                        ret = R_No;
                        goto ERR;
                    }
                }
                else
                {            
                    tempBin->defaultLength = tempXml->length;
                }
            }


            // ��bin�������жϳ���
            if (tempXml->size != 0)
            {
                int tempLength = GetLengthFromSize(tempXml->size );

                if (tempBin->defaultLength != 0)
                {
                    // �����ĳ��ȱ��Լ��ĳ��ȴ��ǿ��ܵģ���������Ԫ����������
                    if (tempLength <= tempBin->defaultLength)
                    {
                        tempBin->defaultLength = tempLength;
                    }
                    else
                    {
                        if (g_onOffDebugParseBin)
                        {                  
                            printf("		solved length is %d < xml size length is %d , check failed@@@@\r\n", tempBin->defaultLength, tempXml->length);
                        }

                        ret = R_No;
                        goto ERR;
                        }
                }
                else
                {            
                    tempBin->defaultLength = tempLength;
                }
            }

            if (tempBin->defaultLength > *length)
            {
                if (g_onOffDebugParseBin)
                {            
                    printf("		solved length is %d > left length is %d , check failed@@@@\r\n", tempBin->defaultLength, *length);
                }

                ret = R_No;
                goto ERR;
            }

        }

        /************************************************

        Ԫ�ؽ���

        *************************************************/
        if (strcmp(name, "Number") == 0)
        {
            if (hasBin)
            {
                if (*length == 0)
                {         
                    return R_No;
                }
            }

            char* tempBuf = *buf;
            int tempLength = *length;

            ret = NumberCheck(child, hasBin, &tempBuf, &tempLength);

            if (ret == R_No)
            {
                goto ERR;
            }

            if (hasBin)
            {
                //
                tempBin->defaultNumberValue = GetNumberBinValue(0, tempXml->size, tempXml->isSigned, (u8*)(*buf));

                *buf = *buf + tempBin->defaultLength;
                *length = *length -tempBin->defaultLength;
            }
            else
            {
                // û��bin��ֱ�Ӵ�����ģ�����ֵ
                tempBin->defaultNumberValue = tempXml->numberValue;
            }

            BinElementAddChildren(tempBinElement, tempBin);

            if (g_onOffDebugParseBin)
            {
                printf("	number size is =  %d \n", tempXml->size);
                printf("	~~~~value is :\n");
                printf("                          0x%lx --%ld \n", tempBin->defaultNumberValue, tempBin->defaultNumberValue);
            }

        }

        if (strcmp(name ,"Flag") == 0)
        {
            if (hasBin)
            {
                // �����û�п��Ǵ�С�ˣ�Ҫ���ǵ�
                // �õ�ÿ����ֵ����bit�㷨����Flags��ȡ�ã����Ǵ�bufȡ��
                // ��ҪУ��token
                tempBin->defaultNumberValue = 
                GetSomeBitValue(tempBinElement->defaultNumberValue, tempXml->position, tempXml->size, tempBinElement->mutatorElement->xmlElement->size);
            }
            else
            {
                // û��bin��ֱ�Ӵ�����ģ�����ֵ
                tempBin->defaultNumberValue = tempXml->numberValue;
            }

            ret = FlagCheck(child, hasBin, tempBin->defaultNumberValue);
            if (ret == R_No)
            {
                goto ERR;
            }

            // ʹ��position��size��ȡ�Լ���ֵ
            BinElementAddChildren(tempBinElement, tempBin);

            if (g_onOffDebugParseBin)
            {
                printf("	Flag size is =  %d \n", tempXml->size);
                printf("	~~~~value is :\n");
                printf("	                      0x%lx --%ld \n", tempBin->defaultNumberValue, tempBin->defaultNumberValue);
            }
        }

        if (strcmp(name,"String") == 0)
        {
            char* tempBuf =*buf;
            int tempLength =*length;

            ret = StringCheck(child, hasBin, &tempBuf, &tempLength);

            if (ret == R_No)
            {
                goto ERR;
            }

            if (hasBin)
            {
                if (GetIsHasNotSolved())
                {
                    // �����жϣ�һ��Ϊtoken ��һ���г���
                    char *rel = MyMemmem(*buf, *length, (char*)tempXml->stringValue, tempXml->length); 

                    // �����һ���������кô���,��ȡ����ֵ��Ҫ�����\0
                    tempBin->defaultStringValue = malloc(tempXml->length + 1);
                    g_testcaseMemory[g_testcaseMemoryCount++] = tempBin->defaultStringValue;

                    memcpy(tempBin->defaultStringValue, rel, tempXml->length);
                    tempBin->defaultStringValue[tempXml->length] = 0;
                    tempBin->defaultLength = tempXml->length;

                    // ��δ��ֵ�ĸ�ֵ
                    int temp_length1 = rel - *buf;
                    DoSolvedElement(tempBin, temp_length1, *buf);

                    *buf = *buf + tempXml->length + temp_length1;
                    *length = *length - tempXml->length - temp_length1;
                }
                else
                {
                    if (tempBin->defaultLength != 0)
                    {
                        // ���null���Ժ����
                        tempBin->defaultStringValue = malloc(tempBin->defaultLength + 1);
                        g_testcaseMemory[g_testcaseMemoryCount++] = tempBin->defaultStringValue;

                        memcpy(tempBin->defaultStringValue, *buf, tempBin->defaultLength);
                        tempBin->defaultStringValue[tempBin->defaultLength] = 0;

                        *buf = *buf + tempBin->defaultLength;
                        *length = *length - tempBin->defaultLength;
                    }
                    else
                    {
                        if (tempBin->defaultLengthHasSolve)
                        {
                            // �����г�����0�������û�취
                            tempBin->defaultLength = 0;
                            tempBin->defaultStringValue = NULL;
                        }
                        else
                        {
                            // զ�죬���ʱ��û�г��Ȱ�:),�����relation�ܽ�������Ǿͽ����
                            tempBin->defaultLength = *length;
                            tempBin->defaultStringValue = *buf;

                            g_temp = tempBin;
                        }
                    }
                }
            }
            else
            {
                tempBin->defaultLength = tempXml->length;
                tempBin->defaultStringValue = (char*)tempXml->stringValue;
            }

            BinElementAddChildren(tempBinElement, tempBin);

            if (g_onOffDebugParseBin)
            {
                printf("	String length is =  %d \n", tempBin->defaultLength);

                if (tempBin == g_temp)
                {            
                    printf("		(length no solved)\n");
                }

                printf("	~~~~value is :\n");

                if (tempBin->defaultLength > DISPLAY_MAX_VALUE_LENGTH)
                {
                    HexDump((u8 *)tempBin->defaultStringValue, DISPLAY_MAX_VALUE_LENGTH);
                    printf("	... ...    \n");
                }
                else
                {
                    HexDump((u8 *)tempBin->defaultStringValue, tempBin->defaultLength);
                }
            }
        }

        if (strcmp(name, "Blob") == 0)
        {
            char* tempBuf = *buf;
            int tempLength = *length;

            ret = BlobCheck(child, hasBin, &tempBuf, &tempLength);

            if (ret == R_No)
            {
                goto ERR;
            }

            if (hasBin)
            {
                if (GetIsHasNotSolved())
                {
                    // �����жϣ�һ��Ϊtoken ��һ���г���
                    char *rel = MyMemmem(*buf, *length, (char*)tempXml->blobValue, tempXml->length); 

                    //
                    tempBin->defaultBlobValue = malloc(tempXml->length);
                    g_testcaseMemory[g_testcaseMemoryCount++] = tempBin->defaultBlobValue;

                    memcpy(tempBin->defaultBlobValue, rel, tempXml->length);
                    tempBin->defaultLength = tempXml->length;

                    // ��δ��ֵ�ĸ�ֵ
                    int tempLength1 = rel - *buf;
                    DoSolvedElement(tempBin, tempLength1, *buf);

                    *buf = *buf + tempXml->length + tempLength1;
                    *length = *length - tempXml->length - tempLength1;
                }
                else
                {
                    if (tempBin->defaultLength != 0)
                    {
                        // ���null���Ժ����
                        tempBin->defaultBlobValue = malloc(tempBin->defaultLength);
                        g_testcaseMemory[g_testcaseMemoryCount++] = tempBin->defaultBlobValue;
                        memcpy(tempBin->defaultBlobValue, *buf, tempBin->defaultLength);

                        *buf = *buf + tempBin->defaultLength;
                        *length = *length - tempBin->defaultLength;
                    }
                    else
                    {
                        if (tempBin->defaultLengthHasSolve)
                        {
                            // �����г�����0�������û�취
                            tempBin->defaultLength = 0;
                            tempBin->defaultBlobValue = NULL;
                        }
                        else
                        {
                            // զ�죬���ʱ��û�г��Ȱ�:),�����relation�ܽ�������Ǿͽ����
                            tempBin->defaultLength = *length;
                            tempBin->defaultBlobValue = *buf;

                            g_temp = tempBin;
                        }
                    }
                }
            }
            else
            {
                if (tempXml->length)
                {
                    // xmlû��ֵ����0���
                    tempBin->defaultLength = tempXml->length;
                    tempBin->defaultBlobValue = malloc(tempXml->length);
                    g_testcaseMemory[g_testcaseMemoryCount++] = tempBin->defaultBlobValue;
                    Hw1Memset(tempBin->defaultBlobValue, 0, tempXml->length);
                    memcpy(tempBin->defaultBlobValue, tempXml->blobValue, tempXml->xmlValueLength);
                }
                else if (tempXml->xmlValueLength > 0)
                {
                    // ʹ��xml��ֵ�������
                    tempBin->defaultLength = tempXml->xmlValueLength;
                    tempBin->defaultBlobValue = malloc(tempXml->xmlValueLength);
                    g_testcaseMemory[g_testcaseMemoryCount++] = tempBin->defaultBlobValue;
                    memcpy(tempBin->defaultBlobValue, tempXml->blobValue, tempXml->xmlValueLength);
                }
                else
                {
                    // �����ֵ�ˣ��Ϳ����������:)
                }
            }

            BinElementAddChildren(tempBinElement, tempBin);

            if (g_onOffDebugParseBin)
            {
                printf("	Blob length is =  %d \n", tempBin->defaultLength);
                if (tempBin == g_temp)
                {            
                    printf("		(length no solved)\n");
                }
                printf("	~~~~value is :\n");

                if (tempBin->defaultLength > DISPLAY_MAX_VALUE_LENGTH)
                {
                    HexDump( (u8 *)tempBin->defaultBlobValue, DISPLAY_MAX_VALUE_LENGTH);
                    printf("	... ...    \n");
                }
                else
                {
                    HexDump((u8 *)tempBin->defaultBlobValue, tempBin->defaultLength);
                }

            }

        }

        if (strcmp(name, "Padding") == 0)
        {
            if (hasBin)
            {
                if (*length == 0)
                {         
                    return R_No;
                }
            }

            // ��ȡֵ
            if (hasBin)
            {
                // �õ�����
                SBinElement *alignedTo = NULL;
                int length111 = 0;
                int alignment = atol(tempXml->alignment) / 8;

                if (tempXml->alignedTo != NULL)
                {            
                    alignedTo = BinElementFoundRelationofByName(tempBinElement, tempXml->alignedTo);
                }


                if (alignedTo == NULL)
                {            
                    length111 = *buf - g_tempRoot->binOffset;
                }
                else
                {            
                    length111 = *buf - alignedTo->binOffset;
                }

                tempBin->defaultLength = (alignment - length111 % alignment) % alignment;

                tempBin->defaultPaddingValue = malloc(tempBin->defaultLength);
                g_testcaseMemory[g_testcaseMemoryCount++] = tempBin->defaultPaddingValue;
                memcpy(tempBin->defaultPaddingValue, *buf, tempBin->defaultLength);
                *buf = *buf + tempBin->defaultLength;
                *length = *length - tempBin->defaultLength;
            }
            else
            {
                // ����ں���ڸ�ֵ������Ҳûɶ��
                tempBin->defaultPaddingValue = malloc(tempBin->defaultLength);
                g_testcaseMemory[g_testcaseMemoryCount++] = tempBin->defaultPaddingValue;
                Hw1Memset(tempBin->defaultPaddingValue, 0, tempBin->defaultLength);
            }

            BinElementAddChildren(tempBinElement, tempBin);

            if (g_onOffDebugParseBin)
            {
                printf("	Padding length is =  %d \n", tempBin->defaultLength);
                printf("	~~~~value is :\n");
                HexDump((u8 *)tempBin->defaultPaddingValue, tempBin->defaultLength);
            }

            ret = R_Yes;

        }

        if (strcmp(name, "Block") == 0)
        {
            BinElementAddChildren(tempBinElement, tempBin);

            char* tempBuf = *buf;
            int tempLength = *length;

            // ����Ѿ��õ�length��ʹ��֮
            if (tempBin->defaultLength != 0)
            {         
                tempLength = tempBin->defaultLength;
            }

            ret = getBinElement(child, tempBin, hasBin, &tempBuf, &tempLength);

            if (hasBin)
            {

                // ��default_length���������
                if ((tempBin->defaultLength > 0) && (tempLength != 0) && (!GetIsHasNotSolved()))
                {
                    if (g_onOffDebugParseBin)
                    {               
                        printf("	solved length is %d ,get length is %d , check failed@@@@\n", tempBin->defaultLength, tempBin->defaultLength - tempLength);
                    }

                    ret = R_No;
                }

                if (ret == R_No)
                {
                    BinElementDelChildren(tempBinElement, tempBin);
                    goto ERR;
                }

                // ������Լ��Ķ��ӱ����Լ��ĳ��ȣ����֮
                if ((child->relationOfIs) && (child->isRelationParentAndChild == 1))
                {
                    if (g_onOffDebugParseBin)
                    {
                        printf("	my xpath %s\n", tempBin->xpathName);
                        printf("	relation xpath %s\n", tempBin->mutatorElement->relationOfRelationName);
                    }

                    SBinElement *relationSize = BinElementFoundRecentlyByXpathName(tempBin, tempBin->mutatorElement->relationOfRelationName);

                    if (relationSize == NULL)
                    {               
                        xml_assert(0, "relation1 element(size) not found");
                    }

                    tempBin->defaultLength = GetRelationSize(relationSize);

                    if (g_onOffDebugParseBin)
                    {               
                        printf("		solved3 length is %d by relation %s\r\n", tempBin->defaultLength, relationSize->xpathName);
                    }
                }


                // ������Լ��Ķ���û�н�����ȣ����֮
                if ((tempBin->defaultLength) && (GetIsHasNotSolved()))
                {
                    //��δ��ֵ�ĸ�ֵ
                    int tempLength2 = tempBin->defaultLength - (g_temp->binOffset - tempBin->binOffset );

                    DoSolvedElement(tempBin, tempLength2, g_temp->binOffset);
                }

                // �����û�г��ȣ�����֮  ��������ʱ��ĳ��Ȳ�һ��׼ȷ����Ϊ������û����Ķ���
                if (tempBin->defaultLength)
                {            
                    ;
                }
                else
                {            
                    tempBin->defaultLength = *length - tempLength;
                }


                *buf = *buf + tempBin->defaultLength;
                *length = *length - tempBin->defaultLength;

            }
            else
            {

            }
        }

        if (strcmp(name, "Flags") == 0)
        {
            if (hasBin)
            {
                if (*length == 0)
                {         
                    return R_No;
                }
            }

            if (hasBin)
            {
                // �õ�ֵ
                tempBin->defaultNumberValue = GetNumberBinValue(0, tempXml->size, tempXml->isSigned, (u8*)(*buf));
            }

            if (g_onOffDebugParseBin)
            {
                printf("	Flags size is =  %d \n", tempXml->size);
                printf("	~~~~value is :\n");
                printf("	                      0x%lx --%ld \n", tempBin->defaultNumberValue, tempBin->defaultNumberValue);
            }

            BinElementAddChildren(tempBinElement, tempBin);

            char* tempBuf = *buf;
            int tempLength = *length;

            ret = getBinElement(child, tempBin, hasBin, &tempBuf, &tempLength);

            if (ret == R_No)
            {
                BinElementDelChildren(tempBinElement, tempBin);
                goto ERR;
            }

            if (hasBin)
            {
                tempBin->defaultLength = GetLengthFromSize(tempXml->size );

                *buf = *buf + tempBin->defaultLength;
                *length = *length - tempBin->defaultLength;
            }
        }

        if (strcmp(name, "Choice") == 0)
        {
            char* buf1 = *buf;
            int length1 = *length;

            BinElementAddChildren(tempBinElement, tempBin);

            //Ѱ�Һ��ʵĶ���
            ret = getBinElement(child,tempBin, hasBin, &buf1, &length1);

            if (ret == R_No)
            {
                BinElementDelChildren(tempBinElement, tempBin);
                goto ERR;
            }

            if (hasBin)
            {
                tempBin->defaultLength = *length - length1;

                *buf = *buf+tempBin->defaultLength;
                *length = *length -tempBin->defaultLength;
            }
        }

        /************************************************

        Ԫ�ؽ���over

        *************************************************/

        // choice�Ļ�����һ�����Ӿ�����
        if (strcmp(tempMutatorElement->xmlElement->typeName, "Choice") == 0)
        {
            return R_Yes;
        }

        if (tempXml->isOccurs)
        {
            tempOccurs++;

            if (hasBin == 0)
            {
                // ֻ��minOccurs������
                if (tempOccurs  >= tempXml->minOccurs)
                {            
                    goto next;
                }
                goto aaaa;
            }


            if (tempBin->defaultCount != 0)
            {
                // ����Ļ��������������Ȼ��ԭ����child
                if (tempOccurs < tempBin->defaultCount)
                {            
                    goto aaaa;
                }
            }
            else if (tempOccurs < tempXml->maxOccurs)
            {         
                goto aaaa;
            }
        }

        ERR:	
        // û��binӦ�õ����������?
        if (ret == R_No)
        {
            // ����Ļ�У��ʧ�ܲ���������
            if (tempXml->isOccurs)
            {
                if (tempBin->defaultCount != 0)
                {
                    if (tempOccurs != tempBin->defaultCount)
                    {
                        if (g_onOffDebugParseBin)
                        {                  
                            printf("	occurs count is mismatch, get count is %d , default count is %d, check failed@@@@\n", tempOccurs, tempBin->defaultCount);
                        }
                        return R_No;
                    }
                }


                // ��ѡ��ʱ��ֱ��return ʧ��
                if (tempOccurs < tempXml->minOccurs)
                {
                    if (g_onOffDebugParseBin)
                    {               
                        printf("	occurs count is mismatch, get count is %d < minOccurs is %d, check failed@@@@\n", tempOccurs, tempXml->minOccurs);
                    }
                    return R_No;
                }
                else
                // ��ѡ��ʱ�����
                { 
                    goto next;
                }
            }

            // ������choice�Ļ�����ʱ����ʧ�ܲ���������
            if (strcmp(tempMutatorElement->xmlElement->typeName, "Choice") == 0)
            {
                child = child->next;  
                if (child != NULL)
                {
                    continue;
                }
                else
                {
                    // choice���һ��ѡ��ʧ�ܣ�֤������ʧ����
                    if (g_onOffDebugParseBin)
                    {               
                        printf("	Choice all child is mismatch, check failed@@@@\n");
                    }
                    return R_No;
                }
            }
            
            // �������飬����choice��һ���Ǳ�ѡ�ģ������ǲ��Ǽ��
            return R_No;
        }

next:
        ret = R_Yes;
        child = child->next;

    }

    return R_Yes;
}

// ÿ��binҪ��һ�������⻹��˵ô�*�*�*�*�*�*�*�*�*�*�*�*�*�*�*�*�*�*�*�*�***************************
SBinElement* ParseBin(SMutatorElement* tempMutatorElement, char* binBuf, int length) 
{  
    if (g_onOffDebugParseBin)
    {
        printf("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\r\n");   
        printf("****Parse Bin Start\r\n");  
        printf("* * * * * * * * * * * * * *\r\n");  
    }

    SBinElement* binElementRoot;

    binElementRoot = malloc(sizeof(SBinElement));
    g_testcaseMemory[g_testcaseMemoryCount++] = (char*)binElementRoot;
    Hw1Memset(binElementRoot, 0, sizeof(SBinElement));
    memcpy(binElementRoot->xpathName, tempMutatorElement->xpathName, strlen(tempMutatorElement->xpathName) + 1);
    binElementRoot->mutatorElement = tempMutatorElement;

    int ret;
    char* buf1 = binBuf;
    int length1 = length;

    binElementRoot->binOffset = buf1;

    g_tempRoot = binElementRoot;
    g_temp = NULL;

    int hasbin = 0;
    if (binBuf)
    {   
        hasbin =1;
    }

    ret = getBinElement(tempMutatorElement, binElementRoot, hasbin, &buf1, &length1);
    if (ret == R_No)
    {      
        xml_assert(0, "Bin check failed");
    }

    if (length1 != 0)
    {
        // xml_assert(0 ,"lenght is not 0");
        // ��ʱ����û�н⿪��Ҫ�жϣ��ȼ����ܰ�
        printf("\r\nParse_Bin, lenght is not 0\r\n");
    }

    if (g_onOffDebugParseBin)
    {
        printf("* * * * * * * * * * * * * *\r\n");  
        printf("****Parse Bin  End\r\n");  
        printf("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\r\n");  
    }

    return binElementRoot;
} 

#ifdef __cplusplus
}
#endif
