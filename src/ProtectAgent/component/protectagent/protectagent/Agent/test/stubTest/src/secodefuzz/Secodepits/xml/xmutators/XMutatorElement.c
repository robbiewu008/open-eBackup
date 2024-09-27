/*
版权所有 (c) 华为技术有限公司 2012-2018

实现元素变异
*/
#include "../XML.h"

#ifdef __cplusplus
extern "C" {
#endif

static void GetMutatorElementValue2(SBinElement* temp)
{
    SXMLElement* tempXml = temp->mutatorElement->xmlElement;

    if (tempXml == NULL)
    {   
        return;
    }

    if (g_onOffDebugMutatorElement)
    {   
        printf("GetMutatorElementValue2 %s\r\n", temp->xpathName);
    }

    char *typeName = tempXml->typeName;

    // Analyzer单独处理,不变异用Analyzer标记干嘛
    if ((tempXml->isAnalyzer == 1) && (tempXml->isMutable == 1))
    {
        DoAnalyzer(temp);
    }
    else if (strcmp(typeName, "Number") == 0)
    {
        s64 tempValue = 0;
        // 不适用变异出来的内存，大小端会改写内存，样本保存的时候会有问题，
        temp->mutaterValue = (char*)&temp->tempNumberValueMemory;

        if (tempXml->size  > 32)
        {
            if (tempXml->isMutable == 1)
            {         
                tempValue = *(s64*)DT_SetGetS64(&g_Element[g_tempElementCount], temp->defaultNumberValue);
                temp->hasMutator = DT_GET_IsBeMutated(&g_Element[g_tempElementCount]);
            }
            else
            {         
                tempValue = temp->defaultNumberValue;
            }

            temp->mutaterLength = 8 ;
        }
        else if (tempXml->size > 16)
        {
            if (tempXml->isMutable == 1)
            {         
                tempValue	= *(s32*)DT_SetGetS32(&g_Element[g_tempElementCount], temp->defaultNumberValue);
                temp->hasMutator = DT_GET_IsBeMutated(&g_Element[g_tempElementCount]);
            }
            else
            {         
                tempValue = temp->defaultNumberValue;
            }

            temp->mutaterLength = 4 ;
        }
        else if (tempXml->size > 8)
        {
            if (tempXml->isMutable == 1)
            {
                tempValue = *(s16*)DT_SetGetS16(&g_Element[g_tempElementCount], temp->defaultNumberValue);
                temp->hasMutator = DT_GET_IsBeMutated(&g_Element[g_tempElementCount]);
            }
            else
            {         
                tempValue = temp->defaultNumberValue;
            }

            temp->mutaterLength = 2 ;
        }
        else 
        {	
            if (tempXml->isMutable == 1)
            {
                tempValue = *(s8*)DT_SetGetS8(&g_Element[g_tempElementCount], temp->defaultNumberValue);
                temp->hasMutator = DT_GET_IsBeMutated(&g_Element[g_tempElementCount]);
            }
            else
            {         
                tempValue = temp->defaultNumberValue;
            }

            temp->mutaterLength = 1 ;
        }

        temp->mutaterValueNumber = tempValue;

        SetNumberBinValue(0, tempXml->size, 0, (u8 *)temp->mutaterValue, tempValue);

        if (g_onOffDebugMutatorElement)
        {
            printf("	Number: M length is %d ,default value is %d ,0x%x\r\n", temp->mutaterLength, (s32)temp->defaultNumberValue, (s32)temp->defaultNumberValue);
            printf("	value is: %d 0x%x\r\n", (s32)temp->mutaterValueNumber, (s32)temp->mutaterValueNumber);
            HexDump((u8 *)temp->mutaterValue, temp->mutaterLength);
        }

        if (tempXml->isMutable == 1)
        {      
            g_tempElementCount++;
        }

    }
    else if (strcmp(typeName, "Flags") == 0)
    {
        temp->mutaterValue = (char *)&temp->tempNumberValueMemory;

        // 这个变异实际浪费了，就是用来保存内存的
        if (tempXml->size > 32)
        {
            temp->mutaterLength = 8 ;
        }
        else if (tempXml->size > 16)
        {
            temp->mutaterLength = 4 ;
        }
        else if (tempXml->size > 8)
        {
            temp->mutaterLength = 2 ;
        }
        else 
        {
            temp->mutaterLength = 1 ;
        }

        // 哈哈哈
        temp->mutaterValueNumber = 0;

    }
    else if (strcmp(typeName, "Flag") == 0)
    {
        s64 temp_value =0;
        // 不适用变异出来的内存，大小端会改写内存，样本保存的时候会有问题，
        temp->mutaterValue = (char *)&temp->tempNumberValueMemory;

        if (tempXml->size > 32)
        {
            if (tempXml->isMutable == 1)
            {
                temp_value = *(s64*)DT_SetGetS64(&g_Element[g_tempElementCount], temp->defaultNumberValue);
                temp->hasMutator = DT_GET_IsBeMutated(&g_Element[g_tempElementCount]);
            }
            else
            {         
                temp_value = temp->defaultNumberValue;
            }

            temp->mutaterLength = 8 ;
        }
        else if (tempXml->size > 16)
        {
            if (tempXml->isMutable == 1)
            {
                temp_value	= *(s32*)DT_SetGetS32(&g_Element[g_tempElementCount], temp->defaultNumberValue);
                temp->hasMutator = DT_GET_IsBeMutated(&g_Element[g_tempElementCount]);
            }
            else
            {         
                temp_value = temp->defaultNumberValue;
            }

            temp->mutaterLength = 4 ;
        }
        else if (tempXml->size > 8)
        {
            if (tempXml->isMutable == 1)
            {         
                temp_value	= *(s16*)DT_SetGetS16(&g_Element[g_tempElementCount], temp->defaultNumberValue);
                temp->hasMutator = DT_GET_IsBeMutated(&g_Element[g_tempElementCount]);
            }
            else
            {         
                temp_value = temp->defaultNumberValue;
            }

            temp->mutaterLength = 2 ;
        }
        else 
        {	
            if (tempXml->isMutable == 1)
            {      
                temp_value	= *(s8*)DT_SetGetS8(&g_Element[g_tempElementCount], temp->defaultNumberValue);
                temp->hasMutator = DT_GET_IsBeMutated(&g_Element[g_tempElementCount]);
            }
            else
            {         
                temp_value = temp->defaultNumberValue;
            }

            temp->mutaterLength	= 1 ;
        }

        temp->mutaterValueNumber = temp_value;

        s64 aaa = temp->parent->mutaterValueNumber;

        SetSomeBitValue(&aaa, temp->mutaterValueNumber, tempXml->position, tempXml->size, temp->parent->mutatorElement->xmlElement->size);
        temp->parent->mutaterValueNumber = aaa;

        // 应该在这里把变异的值赋值给Flags
        SetNumberBinValue(0, temp->parent->mutatorElement->xmlElement->size, 0, (u8 *)temp->parent->mutaterValue, aaa);

        if (g_onOffDebugMutatorElement)
        {
            printf("	my value is: %d 0x%x\r\n", (s32)temp->mutaterValueNumber, (s32)temp->mutaterValueNumber);

            printf("	parent is Flags: size is %d\r\n",temp->parent->mutatorElement->xmlElement->size);
            printf("	value is: %d 0x%x\r\n", (s32)temp->parent->mutaterValueNumber, (s32)temp->parent->mutaterValueNumber);
            HexDump((u8 *)temp->parent->mutaterValue, temp->parent->mutaterLength);
        }

        if (tempXml->isMutable == 1)
        {      
            g_tempElementCount++;
        }
    }
    else if (strcmp(typeName, "String") == 0)
    {
        char* value=NULL;
        int length=0;

        if (tempXml->isMutable == 1)
        {
            if (tempXml->isStringNumber == 1)
            {         
                value = DT_SetGetStringNum(&g_Element[g_tempElementCount], strlen(temp->defaultStringValue) + 1, g_maxOutOfMutator, temp->defaultStringValue);
            }
            else
            {
                value = DT_SetGetString(&g_Element[g_tempElementCount], strlen(temp->defaultStringValue) + 1, g_maxOutOfMutator, temp->defaultStringValue);
            }

            length	 = DT_GET_MutatedValueLen(&g_Element[g_tempElementCount]) ;

            temp->hasMutator = DT_GET_IsBeMutated(&g_Element[g_tempElementCount]);
        }

        if (temp->hasMutator)
        {
            // 对于length 大于value实际长度的情况，变异后仍然应该补充0，暂不实现
            temp->mutaterValue = value;
            temp->mutaterLength = length;

            if (tempXml->isnullTerminated != 1)
            {
                if (temp->mutaterLength > 0)
                {            
                    temp->mutaterLength = temp->mutaterLength - 1;
                }
            }
        }
        else
        {
            temp->mutaterValue = temp->defaultStringValue;
            if (temp->defaultLength > 0)
            {
                temp->mutaterLength = temp->defaultLength;
            }
            // 采用value实际长度
            else if (temp->defaultStringValue != NULL)
            {
                temp->mutaterLength = strlen(temp->defaultStringValue) + 1;

                if (tempXml->isnullTerminated != 1)
                {
                    if (temp->mutaterLength > 0)
                    {            
                        temp->mutaterLength = temp->mutaterLength - 1;
                    }
                }
            }
            else  // 这种情况不应该存在
            {
                temp->mutaterLength = 0;
            }
        }

        if (g_onOffDebugMutatorElement)
        {
            printf("	String: length is %d\r\n", temp->mutaterLength);
            printf("	value is:\r\n");
            HexDump((u8 *)temp->mutaterValue, temp->mutaterLength);
        }

        if (tempXml->isMutable == 1)
        {      
            g_tempElementCount++;
        }
    }
    else if (strcmp(typeName, "Blob") == 0)
    {
        if (tempXml->isMutable == 1)
        {
            temp->mutaterValue = DT_SetGetBlob(&g_Element[g_tempElementCount], temp->defaultLength, g_maxOutOfMutator, temp->defaultBlobValue);
            temp->mutaterLength = DT_GET_MutatedValueLen(&g_Element[g_tempElementCount]) ;
            temp->hasMutator = DT_GET_IsBeMutated(&g_Element[g_tempElementCount]);
        }
        else
        {
            temp->mutaterValue = temp->defaultBlobValue;
            temp->mutaterLength = temp->defaultLength;
        }

        if (g_onOffDebugMutatorElement)
        {
            printf("	Blob: length is %d\r\n", temp->mutaterLength);
            printf("	value is:\r\n");
            HexDump((u8 *)temp->mutaterValue, temp->mutaterLength);
        }

        if (tempXml->isMutable == 1)
        {      
            g_tempElementCount++;
        }
    }
    // 非元素跳过
    else if (strcmp(typeName ,"Block") == 0)
    {

    }

}

void GetMutatorElementValue(SBinElement* temp)
{
    
    For_Tree_Start(temp)
    {
        GetMutatorElementValue2(temp);
    }
    For_Tree_End(temp)
}

#ifdef __cplusplus
}
#endif
