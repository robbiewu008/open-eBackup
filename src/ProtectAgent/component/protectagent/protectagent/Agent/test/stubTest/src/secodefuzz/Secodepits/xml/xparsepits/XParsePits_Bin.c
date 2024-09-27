/*
°æÈ¨ËùÓÐ (c) »ªÎª¼¼ÊõÓÐÏÞ¹«Ë¾ 2012-2018


½âÎöÖ¸¶¨Êý¾ÝÄ£ÐÍ£¬Éú³ÉÐèÒª±äÒìµÄÔªËØ¶þ²æÊ÷£¬²¢½â¾örefÎÊÌâ
*/

#include "../XML.h"

#ifdef __cplusplus
extern "C" {
#endif

static SBinElement* g_temp = NULL;
static SBinElement* g_tempRoot;

// ÏÈ²éÕÒ×Ô¼ºµÄ¸¸Ç×£¬È»ºóÊÇÐÖµÜ£¬Ò¯Ò¯£¬¸¸Ç×µÄÐÖµÜ.... ....
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
        // ÅÐ¶Ï½â¾öËüµÄÔªËØ»¹ÔÚ·ñ
        return FoundRecentlyByName55(g_temp->solvedXpathName);
    }

    return 0;
}


// ¾ÍÒ»ÂÖ¼ì²â¶øÒÑ£¬´¿µÄ
static int NumberCheck(SMutatorElement* tempMutatorElement, int hasbin, char ** buf, int* length)
{
    SXMLElement *tempXml = tempMutatorElement->xmlElement;
    s64 defaultNumberValue;


    if (hasbin)
    {
        // Ä¿Ç°½öÖ§³Ö32,ÉÔºóÐÞ¸Ä
        defaultNumberValue = GetNumberBinValue(0, tempXml->size, tempXml->isSigned, (u8*)(*buf));

        // token ±ØÐëÆ¥Åä£¬ÎÞÌõ¼þ
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

//¾ÍÒ»ÂÖ¼ì²â¶øÒÑ£¬´¿µÄ
static int FlagCheck(SMutatorElement* tempMutatorElement, int hasBin, s64 value)
{
    SXMLElement *tempXml = tempMutatorElement->xmlElement;
    // s64 		default_Number_value;

    if (hasBin)
    {
        // token ±ØÐëÆ¥Åä£¬ÎÞÌõ¼þ
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

// ¾ÍÒ»ÂÖ¼ì²â¶øÒÑ£¬´¿µÄ
static int StringCheck(SMutatorElement* tempMutatorElement, int hasBin, char ** buf, int* length)
{
    SXMLElement *tempXml = tempMutatorElement->xmlElement;

    if (hasBin)
    {
        if (GetIsHasNotSolved())
        {
            // Á½¸ö²»È·¶¨µÄÔªËØÏàÁ¬ÊÇ²»ÔÊÐíµÄ
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
                // ÊÇtoken±ØÐëÄÜÕÒµ½×Ö·û´®
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
        // ¼ì²âbinÖÐnullTerminatedÊÇ·ñÂú×ã
        if (tempXml->isnullTerminated == 1)
        {
            if ((*buf)[tempXml->length] != 0)
            {
                return R_No;
            }
        }

        // ¼ì²âtoken
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

// ¾ÍÒ»ÂÖ¼ì²â¶øÒÑ£¬´¿µÄ
static int BlobCheck(SMutatorElement* tempMutatorElement, int hasBin, char ** buf, int* length)
{
    SXMLElement *tempXml = tempMutatorElement->xmlElement;

    if (hasBin)
    {
        if (GetIsHasNotSolved())
        {
            // Á½¸ö²»È·¶¨µÄÔªËØÏàÁ¬ÊÇ²»ÔÊÐíµÄ
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
                // ÊÇtoken±ØÐëÄÜÕÒµ½×Ö·û´®
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
            //¼ì²âtoken
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

// µÃµ½±äÒìÔªËØ¶þ²æÊ÷
static int getBinElement(SMutatorElement* tempMutatorElement, SBinElement* tempBinElement, int hasBin, char** buf, int *length )
{
    // Ê¹ÓÃS_Mutator_Element Ê÷×öÑ­»·
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
            // Êý×é¸öÊýÎª0,ÄÇºÍÃ»ÓÐÊÇÒ»ÑùµÄ
            if ((tempXml->minOccurs == 0) && (tempXml->maxOccurs == 0))
            {
                if (g_onOffDebugParseBin)
                {
                    printf("    	occurs is  0,goto next element ...\r\n");
                }

                goto next;
            }

            // Ã»ÓÐbin,°´ÕÕminOccurs´¦Àí
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
            // Èç¹ûÊÇÊý×é£¬relation count,½â¾öÖ®
            if (child->lastIsRelationC == 1)
            {
                // µÃµ½relationµÄ³¤¶È
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

                // Èç¹ûÊÇ0ÊÇ²»ÊÇ¾ÍÖ±½ÓcontinueÁË
                if (tempBin->defaultCount == 0)
                {
                    if (g_onOffDebugParseBin)
                    {
                        printf("    	solved occurs is  0,goto next element ...\r\n");
                    }

                    goto next;;
                }
            }

            // relation size×îºóÒ»¸öÔªËØ
            if (child->lastIsRelationS == 1)
            {
                // µÃµ½relationµÄ³¤¶È
                SBinElement *relationSize = BinElementFoundRecentlyByXpathName(tempBinElement, child->lastRelationS);
                SBinElement *relationOf = BinElementFoundRecentlyByXpathName(tempBinElement, child->lastRelationOfS);

                // Ò²Ðí¾Í×Ô¸öÒ»¸ö
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

            // Èç¹ûblockµÈ£¬relation size, ³¤¶È×Ö¶Î²»ÊÇ×Ô¼ºµÄ¶ù×Ó£¬ÄÇÏÖÔÚ¾Í¿ÉÒÔµÃµ½³¤¶ÈµÄ
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

                // ÐèÒªÔÚÕâÀï¼ìÑéÃ´³¤¶ÈÃ´?
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

            // ³¤¶È¼ì²â
            if (tempXml->length != 0)
            {
                if (tempBin->defaultLength != 0)
                {
                    // ½âÎöµÄ³¤¶È±È×Ô¼ºµÄ³¤¶È´óÊÇ¿ÉÄÜµÄ£¬±ÈÈç×îºóµÄÔªËØÔÚÊý×éÀï
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


            // ÓÐbinÔòÊ×ÏÈÅÐ¶Ï³¤¶È
            if (tempXml->size != 0)
            {
                int tempLength = GetLengthFromSize(tempXml->size );

                if (tempBin->defaultLength != 0)
                {
                    // ½âÎöµÄ³¤¶È±È×Ô¼ºµÄ³¤¶È´óÊÇ¿ÉÄÜµÄ£¬±ÈÈç×îºóµÄÔªËØÔÚÊý×éÀï
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

        ÔªËØ½âÎö

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
                // Ã»ÓÐbinÔòÖ±½Ó´ÓÊý¾ÝÄ£ÐÍÀï¶ÁÖµ
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
                // Õâ¸ö»¹Ã»ÓÐ¿¼ÂÇ´óÐ¡¶Ë£¬Òª¿¼ÂÇµÄ
                // µÃµ½Ã¿¸öµÄÖµ£¬ÓÃbitËã·¨£¬´ÓFlagsÀïÈ¡µÃ£¬²»ÊÇ´ÓbufÈ¡µÃ
                // »¹ÒªÐ£Ñétoken
                tempBin->defaultNumberValue = 
                GetSomeBitValue(tempBinElement->defaultNumberValue, tempXml->position, tempXml->size, tempBinElement->mutatorElement->xmlElement->size);
            }
            else
            {
                // Ã»ÓÐbinÔòÖ±½Ó´ÓÊý¾ÝÄ£ÐÍÀï¶ÁÖµ
                tempBin->defaultNumberValue = tempXml->numberValue;
            }

            ret = FlagCheck(child, hasBin, tempBin->defaultNumberValue);
            if (ret == R_No)
            {
                goto ERR;
            }

            // Ê¹ÓÃpositionºÍsize»ñÈ¡×Ô¼ºµÄÖµ
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
                    // ¾­¹ýÅÐ¶Ï£¬Ò»¶¨Îªtoken £¬Ò»¶¨ÓÐ³¤¶È
                    char *rel = MyMemmem(*buf, *length, (char*)tempXml->stringValue, tempXml->length); 

                    // ¶à·ÖÅäÒ»¸ö³¤¶ÈÊÇÓÐºÃ´¦µÄ,»ñÈ¡±äÒìÖµÐèÒªºó±ßÓÐ\0
                    tempBin->defaultStringValue = malloc(tempXml->length + 1);
                    g_testcaseMemory[g_testcaseMemoryCount++] = tempBin->defaultStringValue;

                    memcpy(tempBin->defaultStringValue, rel, tempXml->length);
                    tempBin->defaultStringValue[tempXml->length] = 0;
                    tempBin->defaultLength = tempXml->length;

                    // ¸øÎ´¸³ÖµµÄ¸³Öµ
                    int temp_length1 = rel - *buf;
                    DoSolvedElement(tempBin, temp_length1, *buf);

                    *buf = *buf + tempXml->length + temp_length1;
                    *length = *length - tempXml->length - temp_length1;
                }
                else
                {
                    if (tempBin->defaultLength != 0)
                    {
                        // ¼ì²ânull£¬ÒÔºóÌí¼Ó
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
                            // ¾ÍÊÇÓÐ³¤¶ÈÊÇ0µÄÇé¿ö£¬Ã»°ì·¨
                            tempBin->defaultLength = 0;
                            tempBin->defaultStringValue = NULL;
                        }
                        else
                        {
                            // Õ¦°ì£¬Õâ¸öÊ±ºòÃ»ÓÐ³¤¶È°¡:),Èç¹ûÓÐrelationÄÜ½â¾öËû£¬ÄÇ¾Í½â¾ößÂ
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
                    // ¾­¹ýÅÐ¶Ï£¬Ò»¶¨Îªtoken £¬Ò»¶¨ÓÐ³¤¶È
                    char *rel = MyMemmem(*buf, *length, (char*)tempXml->blobValue, tempXml->length); 

                    //
                    tempBin->defaultBlobValue = malloc(tempXml->length);
                    g_testcaseMemory[g_testcaseMemoryCount++] = tempBin->defaultBlobValue;

                    memcpy(tempBin->defaultBlobValue, rel, tempXml->length);
                    tempBin->defaultLength = tempXml->length;

                    // ¸øÎ´¸³ÖµµÄ¸³Öµ
                    int tempLength1 = rel - *buf;
                    DoSolvedElement(tempBin, tempLength1, *buf);

                    *buf = *buf + tempXml->length + tempLength1;
                    *length = *length - tempXml->length - tempLength1;
                }
                else
                {
                    if (tempBin->defaultLength != 0)
                    {
                        // ¼ì²ânull£¬ÒÔºóÌí¼Ó
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
                            // ¾ÍÊÇÓÐ³¤¶ÈÊÇ0µÄÇé¿ö£¬Ã»°ì·¨
                            tempBin->defaultLength = 0;
                            tempBin->defaultBlobValue = NULL;
                        }
                        else
                        {
                            // Õ¦°ì£¬Õâ¸öÊ±ºòÃ»ÓÐ³¤¶È°¡:),Èç¹ûÓÐrelationÄÜ½â¾öËû£¬ÄÇ¾Í½â¾ößÂ
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
                    // xmlÃ»ÓÐÖµ£¬ÓÃ0Ìî³ä
                    tempBin->defaultLength = tempXml->length;
                    tempBin->defaultBlobValue = malloc(tempXml->length);
                    g_testcaseMemory[g_testcaseMemoryCount++] = tempBin->defaultBlobValue;
                    Hw1Memset(tempBin->defaultBlobValue, 0, tempXml->length);
                    memcpy(tempBin->defaultBlobValue, tempXml->blobValue, tempXml->xmlValueLength);
                }
                else if (tempXml->xmlValueLength > 0)
                {
                    // Ê¹ÓÃxmlµÄÖµÍêÕûÌî³ä
                    tempBin->defaultLength = tempXml->xmlValueLength;
                    tempBin->defaultBlobValue = malloc(tempXml->xmlValueLength);
                    g_testcaseMemory[g_testcaseMemoryCount++] = tempBin->defaultBlobValue;
                    memcpy(tempBin->defaultBlobValue, tempXml->blobValue, tempXml->xmlValueLength);
                }
                else
                {
                    // ²»Ìî³äÖµÁË£¬¾Í¿¿Õâ¸ö±äÒìÁË:)
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

            // »ñÈ¡Öµ
            if (hasBin)
            {
                // µÃµ½³¤¶È
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
                // Õâ¸öÔÚºó±ßÔÚ¸³Öµ£¬ÕâÀïÒ²Ã»É¶ÓÃ
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

            // Èç¹ûÒÑ¾­µÃµ½length£¬Ê¹ÓÃÖ®
            if (tempBin->defaultLength != 0)
            {         
                tempLength = tempBin->defaultLength;
            }

            ret = getBinElement(child, tempBin, hasBin, &tempBuf, &tempLength);

            if (hasBin)
            {

                // ÓÐdefault_length£¬±ØÐë×ð³ç
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

                // Èç¹ûÊÇ×Ô¼ºµÄ¶ù×Ó±£´æ×Ô¼ºµÄ³¤¶È£¬½â¾öÖ®
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


                // Èç¹ûÊÇ×Ô¼ºµÄ¶ù×ÓÃ»ÓÐ½â¾ö³¤¶È£¬½â¾öÖ®
                if ((tempBin->defaultLength) && (GetIsHasNotSolved()))
                {
                    //¸øÎ´¸³ÖµµÄ¸³Öµ
                    int tempLength2 = tempBin->defaultLength - (g_temp->binOffset - tempBin->binOffset );

                    DoSolvedElement(tempBin, tempLength2, g_temp->binOffset);
                }

                // Èç¹û»¹Ã»ÓÐ³¤¶È£¬¼ÆËãÖ®  £¬²»¹ýÕâÊ±ºòµÄ³¤¶È²»Ò»¶¨×¼È·£¬ÒòÎª¿ÉÄÜÓÐÃ»½â¾öµÄ¶ù×Ó
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
                // µÃµ½Öµ
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

            //Ñ°ÕÒºÏÊÊµÄ¶ù×Ó
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

        ÔªËØ½âÎöover

        *************************************************/

        // choiceµÄ»°Óöµ½Ò»¸ö¶ù×Ó¾ÍÐÐÁË
        if (strcmp(tempMutatorElement->xmlElement->typeName, "Choice") == 0)
        {
            return R_Yes;
        }

        if (tempXml->isOccurs)
        {
            tempOccurs++;

            if (hasBin == 0)
            {
                // Ö»¿´minOccursµÄÊýÁ¿
                if (tempOccurs  >= tempXml->minOccurs)
                {            
                    goto next;
                }
                goto aaaa;
            }


            if (tempBin->defaultCount != 0)
            {
                // Êý×éµÄ»°£¬Èç¹û»¹ÓÐÔòÈÔÈ»ÓÃÔ­À´µÄchild
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
        // Ã»ÓÐbinÓ¦¸Ãµ½²»ÁËÕâÀï°É?
        if (ret == R_No)
        {
            // Êý×éµÄ»°Ð£ÑéÊ§°Ü²»ÊÇÖÂÃüµÄ
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


                // ±ØÑ¡µÄÊ±ºòÖ±½Óreturn Ê§°Ü
                if (tempOccurs < tempXml->minOccurs)
                {
                    if (g_onOffDebugParseBin)
                    {               
                        printf("	occurs count is mismatch, get count is %d < minOccurs is %d, check failed@@@@\n", tempOccurs, tempXml->minOccurs);
                    }
                    return R_No;
                }
                else
                // ¿ÉÑ¡µÄÊ±ºò¼ÌÐø
                { 
                    goto next;
                }
            }

            // ¸¸Ç×ÊÇchoiceµÄ»°¼ì²âµÄÊ±ºò¼ì²âÊ§°Ü²»ÊÇÖÂÃüµÄ
            if (strcmp(tempMutatorElement->xmlElement->typeName, "Choice") == 0)
            {
                child = child->next;  
                if (child != NULL)
                {
                    continue;
                }
                else
                {
                    // choice×îºóÒ»¸öÑ¡ÏîÊ§°Ü£¬Ö¤Ã÷ÕûÌåÊ§°ÜÁË
                    if (g_onOffDebugParseBin)
                    {               
                        printf("	Choice all child is mismatch, check failed@@@@\n");
                    }
                    return R_No;
                }
            }
            
            // ²»ÊÇÊý×é£¬²»ÊÇchoice£¬Ò»¶¨ÊÇ±ØÑ¡µÄ£¬ÎÞÂÛÊÇ²»ÊÇ¼ì²â
            return R_No;
        }

next:
        ret = R_Yes;
        child = child->next;

    }

    return R_Yes;
}

// Ã¿¸öbinÒªÓÐÒ»¿ÃÊ÷£¬Õâ»¹ÓÃËµÃ´å*å*å*å*å*å*å*å*å*å*å*å*å*å*å*å*å*å*å*å*å***************************
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
        // ÔÝÊ±³¤¶ÈÃ»ÓÐ½â¿ª²»ÒªÖÐ¶Ï£¬ÏÈ¼ÌÐøÅÜ°É
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
