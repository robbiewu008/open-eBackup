/*
版权所有 (c) 华为技术有限公司 2012-2018


Relation实现和分发
*/

#include "../XML.h"

#ifdef __cplusplus
extern "C" {
#endif

//XEXfunction
extern int function_AAAA(int size);
extern int function_BBBB(int size);
extern int function_mp4get(int size);
extern int function_mp4set(int size);
extern int function_mp4get1(int size);
extern int function_mp4set1(int size);
extern int function_IIIIG(int size);
extern int function_IIIIS(int size);
extern int function_IPV4_get(int size);
extern int function_IPV4_set(int size);
extern int function_get_mul10(int size);
extern int function_set_mul10(int size);

int DoExpressionGet(int size ,char* str)
{
    if (str == NULL)
        return size ;

    if (strcmp(str,"AAAA") == 0)
    {
        return function_AAAA(size);
    }

    if (strcmp(str,"mp4get") == 0)
    {
        return function_mp4get(size);
    }

    if (strcmp(str,"mp4get1") == 0)
    {
        return function_mp4get1(size);
    }

    if (strcmp(str,"IIIIG") == 0)
    {
        return function_IIIIG(size);
    }

    if (strcmp(str,"IPV4_get") == 0)
    {
        return function_IPV4_get(size);
    }

    if (strcmp(str,"get_mul10") == 0)
    {
        return function_get_mul10(size);
    }

    return size ;
}

int DoExpressionSet(int size, char* str)
{
    if (str == NULL)
        return size ;

    if (strcmp(str,"BBBB") == 0)
    {
        return function_BBBB(size);
    }

    if (strcmp(str,"mp4set") == 0)
    {
        return function_mp4set(size);
    }

    if (strcmp(str,"mp4set1") == 0)
    {
        return function_mp4set1(size);
    }

    if (strcmp(str,"IIIIS") == 0)
    {
        return function_IIIIS(size);
    }

    if (strcmp(str,"IPV4_set") == 0)
    {
        return function_IPV4_set(size);
    }

    if (strcmp(str,"set_mul10") == 0)
    {
        return function_set_mul10(size);
    }

    return size ;
}

void DoRelation(SBinElement* temp)
{
    SBinElement* temp1 = NULL;

    For_Tree_Start(temp)
    {
        SXMLElement* tempXml = temp->mutatorElement->xmlElement;

        // 如果size是变异来的，不做relation
        if ((g_pitsMutator.mutatorRelationsize == 1) && (temp->hasMutator  == 1))
        {
            if ((g_onOffDebugMutatorPits) && (tempXml->isRelation == 1) )
            {   
                printf("*********************%s relation mutator\r\n", temp->xpathName);
            }

            goto abcd;
        }

        if ((tempXml != NULL)
            && (strcmp(tempXml->typeName, "Number") == 0)
            && (tempXml->isRelation == 1)
            && (strcmp(tempXml->relationType, "size") == 0))
        {
            if (g_onOffDebugDoRelation)
            {         
                printf("	do relation :xpath=%s\r\n", temp->xpathName);
            }

            // 目前只关心自己父亲底下的
            temp1 = BinElementFoundRelationofByName( temp, tempXml->RelationOf);

            if (temp1 == NULL)
            {
                printf("	Relation_of %s not found\r\n", tempXml->RelationOf);
                return;
            }

            if (g_onOffDebugDoRelation)
            {         
                printf("		reation of=%s\r\n", temp1->xpathName);
            }

            int length = GetPitsBufLength(temp1);

            if (g_onOffDebugDoRelation)
            {         
                printf("		length=%d\r\n", length);
            }

            length = DoExpressionSet(length, temp->mutatorElement->xmlElement->expressionSet);

            if (g_onOffDebugDoRelation)
            {         
                printf("		length expressionSet=%d\r\n", length);
            }

            SetNumberBinValue(0, tempXml->size, 0, (u8*)temp->mutaterValue, length);
        }

        if ((tempXml != NULL)
            && (strcmp(tempXml->typeName, "Flag") == 0)
            && (tempXml->isRelation == 1)
            && (strcmp(tempXml->relationType, "size") == 0))
        {
            if (g_onOffDebugDoRelation)
            {         
                printf("	do relation :xpath=%s\r\n", temp->xpathName);
            }

            // 目前只关心自己父亲底下的
            temp1 = BinElementFoundRelationofByName(temp,tempXml->RelationOf);

            if (temp1 == NULL)
            {
                printf("	Relation_of %s not found\r\n", tempXml->RelationOf);
                return;
            }

            if (g_onOffDebugDoRelation)
            {         
                printf("		reation of=%s\r\n", temp1->xpathName);
            }

            int length = GetPitsBufLength(temp1);

            if (g_onOffDebugDoRelation)
            {         
                printf("		length=%d\r\n", length);
            }

            length = DoExpressionSet(length, temp->mutatorElement->xmlElement->expressionSet);

            if (g_onOffDebugDoRelation)
            {         
                printf("		length expressionSet=%d\r\n", length);
            }

            temp->mutaterValueNumber = length;

            s64 aaa = temp->parent->mutaterValueNumber;

            SetSomeBitValue(&aaa, temp->mutaterValueNumber, tempXml->position, tempXml->size, temp->parent->mutatorElement->xmlElement->size);
            temp->parent->mutaterValueNumber = aaa;

            // 应该在这里把变异的值赋值给Flags
            SetNumberBinValue(0, temp->parent->mutatorElement->xmlElement->size, 0, (u8*)temp->parent->mutaterValue, aaa);
        }

        if ((tempXml != NULL)
            && (strcmp(tempXml->typeName, "String") == 0)
            && (tempXml->isRelation == 1)
            && (strcmp(tempXml->relationType, "size") == 0))
        {
            //
            temp1 = BinElementFoundRelationofByName(temp, tempXml->RelationOf);

            if (temp1 == NULL)
            {
                printf("	Relation_of %s not found\r\n", tempXml->RelationOf);
                return;
            }

            int length = GetPitsBufLength(temp1);

            length = DoExpressionSet(length, temp->mutatorElement->xmlElement->expressionSet);

            temp->mutaterValue = Inltoa(length, 0, 10);
            g_onerunMemory[g_onerunMemoryCount++] = temp->mutaterValue;

            temp->mutaterLength = strlen(temp->mutaterValue) + 1;
            // 有没有空格?

            if (tempXml->isnullTerminated != 1)
            {
                if (temp->mutaterLength > 0)
                {            
                    temp->mutaterLength = temp->mutaterLength -1;
                }
            }
        }

        int a = 5;
abcd:
        a = a + 5;
    }
    For_Tree_End(temp)
}

#ifdef __cplusplus
}
#endif
