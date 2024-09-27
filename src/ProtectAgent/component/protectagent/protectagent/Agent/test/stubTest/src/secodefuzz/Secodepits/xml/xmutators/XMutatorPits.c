/*
版权所有 (c) 华为技术有限公司 2012-2018

实现pits变异
*/

#include "../XML.h"

#ifdef __cplusplus
extern "C" {
#endif

char tempBlockBuf[OUT_BUF_MAX_LENGTH];

// 得到元素的长度，block等就是所有子元素的长度
int  GetPitsBufLength(SBinElement* parent)
{
    int length = 0;

    char* name1 = parent->mutatorElement->xmlElement->typeName;

    if ((strcmp(name1, "Number") == 0)
            || (strcmp(name1, "String") == 0)
            || (strcmp(name1, "Blob") == 0)
            || (strcmp(name1, "Flags") == 0)
            || (strcmp(name1, "Padding") == 0))
    {   
        return parent->mutaterLength;
    }

    SBinElement* temp = parent->children;

    For_Tree_child_Start(temp)
    {
        char* name = temp->mutatorElement->xmlElement->typeName;
        if ((strcmp(name, "Number") == 0)
            || (strcmp(name, "String") == 0)
            || (strcmp(name, "Blob") == 0)
            || (strcmp(name, "Flags") == 0)
            || (strcmp(name, "Padding") == 0))
        {
            // 有变异值则增加
            if (temp->mutaterLength != 0 )
            {
                if ((length + temp->mutaterLength) < OUT_BUF_MAX_LENGTH)
                {
                    length = length + temp->mutaterLength;
                }
                else
                {
                    goto ffff;
                }
            }
        }
    }
    For_Tree_child_End(temp, parent)

ffff:
    return length;

}

// 前一万次不变异
int GetPitsBuf(SBinElement* parent, char* buf)
{
    int length = 0;
    char* name1 = parent->mutatorElement->xmlElement->typeName;

    if ((strcmp(name1, "Number") == 0)
            || (strcmp(name1, "String") == 0)
            || (strcmp(name1, "Blob") == 0)
            || (strcmp(name1, "Flags") == 0)
            || (strcmp(name1, "Padding") == 0))
    {  
        memcpy(buf,parent->mutaterValue, parent->mutaterLength);
        return parent->mutaterLength;
    }

    SBinElement* temp = parent->children;

    int seed = DT_Get_RandomSeed();
    int isMutator = 1;

    // 暂时注释掉
    // if(g_onOffDebugMutatorPits)
    // printf("*********************seed is %d\r\n",seed);

    // 如果seed为0，不变异
    if (seed == 0)
    {
        isMutator = 0;
    }
    else
    {
        DT_Set_RandomSeed(seed);
    }

    // 变异出选择哪个变异算法
    int mutator = (DT_Get_RandomVlaue() % g_algorithmFrequency);

    char* blockXpathName = NULL;
    char* blockXpathName1 = NULL;

    int tempElementBufLength = 0;
    char* tempElementBuf = NULL;
    char* tempElementName = NULL;
    int isElementChangeStart = 0;

    int blockStep = 0;
    int tempBlockLength = 0;

    For_Tree_child_Start(temp)
    {
        char* name = temp->mutatorElement->xmlElement->typeName;

        // buf截断
        if ((isMutator == 1) && (mutator == 1) && (g_pitsMutator.mutatorBufCut == 1))
        {
            if ((DT_Get_RandomVlaue() % g_mutatorFrequency) == 1)
            {
                if (g_onOffDebugMutatorPits)
                {
                    printf("*********************buf cut from %s\r\n", temp->xpathName);
                }

                break;
            }
        }

        //先选取个block,以后用不用再说
        if ((isMutator == 1) && (strcmp(name, "Block") == 0) && (temp->mutatorElement->xmlElement->isTransformer != 1))
        {
            if (((DT_Get_RandomVlaue() % g_mutatorFrequency) == 1) && (blockStep == 0))
            {
                blockXpathName = temp->xpathName;
                blockStep = 1;
                tempBlockLength = 0;
            }
        }

        if ((strcmp(name, "Number") == 0)
            || (strcmp(name, "String") == 0)
            || (strcmp(name, "Blob") == 0)
            || (strcmp(name, "Flags") == 0)
            || (strcmp(name, "Padding") == 0)
            || (temp->mutatorElement->xmlElement->isTransformer == 1)) // block transformer 视为一个元素了transformer
        {
            // 有变异值则增加
            if (temp->mutaterLength != 0 )
            {
                // block删除
                if ((isMutator == 1) && (mutator == 2) && (g_pitsMutator.mutatorBlockDelete == 1) && (blockStep ==1))
                {
                    char *rel = MyMemmem(temp->xpathName, strlen(temp->xpathName), blockXpathName, strlen(blockXpathName)); 
                    if (rel != NULL)
                    {
                        goto abcd;
                    }
                    else  // 结束
                    {
                        blockStep = 0 ;
                        if (g_onOffDebugMutatorPits)
                        {
                            printf("*********************delete block %s\r\n", blockXpathName);
                        }
                    }
                }

                // block复制
                if ((isMutator == 1) && (mutator == 3) && (g_pitsMutator.mutatorBlockCopy == 1) && (blockStep ==1))
                {
                    char *rel = MyMemmem(temp->xpathName, strlen(temp->xpathName), blockXpathName, strlen(blockXpathName)); 
                    if (rel !=NULL)
                    {
                        if ((tempBlockLength + temp->mutaterLength) < OUT_BUF_MAX_LENGTH)
                        {
                            memcpy(tempBlockBuf + tempBlockLength, temp->mutaterValue, temp->mutaterLength);
                            tempBlockLength = tempBlockLength + temp->mutaterLength;
                        }
                        else
                        {
                            goto ffff;
                        }
                    }
                    else // 结束
                    {
                        // 11代表最大变异2047作为复制次数
                        int a = GaussRandU32(DT_Get_RandomVlaue() % 11);
                        int i = 0;
                        for (i = 0; i < a; i++)
                        {
                            if ((length + tempBlockLength) < OUT_BUF_MAX_LENGTH)
                            {
                                memcpy(buf + length, tempBlockBuf, tempBlockLength);
                                length = length + tempBlockLength;
                            }
                            else
                            {
                                goto ffff;
                            }
                        }
                        blockStep = 0 ;
                        if (g_onOffDebugMutatorPits)
                        {
                            printf("*********************copy block %s,%d count\r\n", blockXpathName, a);
                        }
                    }
                }

                // block交换
                if ((isMutator == 1) && (mutator == 4) && (g_pitsMutator.mutatorBlockChange == 1) && (blockStep == 1))
                {
                    char *rel = MyMemmem(temp->xpathName, strlen(temp->xpathName), blockXpathName, strlen(blockXpathName)); 
                    if (rel !=NULL)
                    {
                        if ((tempBlockLength + temp->mutaterLength) < OUT_BUF_MAX_LENGTH)
                        {
                            memcpy(tempBlockBuf + tempBlockLength, temp->mutaterValue, temp->mutaterLength);
                            tempBlockLength = tempBlockLength + temp->mutaterLength;
                        }
                        else
                        {
                            goto ffff;
                        }
                        goto abcd;
                    }
                    else // 结束
                    {
                        blockXpathName1 = temp->xpathName;
                        blockStep = 2 ;
                    }
                }

                // block交换
                if ((isMutator == 1) && (mutator == 4) && (g_pitsMutator.mutatorBlockChange == 1) && (blockStep == 2))
                {
                    char *rel = MyMemmem(temp->xpathName, strlen(temp->xpathName), blockXpathName1, strlen(blockXpathName1)); 
                    if (rel !=NULL)
                    {
                        // 什么也不做
                    }
                    else // 结束
                    {
                        if ((length + tempBlockLength) < OUT_BUF_MAX_LENGTH)
                        {
                            memcpy(buf + length, tempBlockBuf, tempBlockLength);
                            length = length + tempBlockLength;
                        }
                        else
                        {
                            goto ffff;
                        }

                        blockStep = 0;
                        if (g_onOffDebugMutatorPits)
                        {
                            printf("*********************chang block %s and %s\r\n", blockXpathName, temp->xpathName);
                        }
                    }
                }

                // 元素删除
                if ((isMutator == 1) && (mutator == 10) && (g_pitsMutator.mutatorElementDelete == 1))
                {
                    if ((DT_Get_RandomVlaue() % g_mutatorFrequency) == 1)
                    {
                        if (g_onOffDebugMutatorPits)
                        {
                            printf("*********************delete element %s\r\n", temp->xpathName);
                        }

                        goto abcd;
                    }
                }


                // 元素交换,目前只与相邻交换
                if ((isMutator == 1) && (mutator == 11) && (g_pitsMutator.mutatorElementChange == 1))
                {
                    if (isElementChangeStart == 1)
                    {
                        if ((length + tempElementBufLength) < OUT_BUF_MAX_LENGTH)
                        {
                            memcpy(buf + length, tempElementBuf, tempElementBufLength);
                            length = length + tempElementBufLength;
                        }
                        else
                        {
                            goto ffff;
                        }
                        isElementChangeStart = 0;
                        if(g_onOffDebugMutatorPits)
                        {
                            printf("*********************change element %s, and %s\r\n", temp->xpathName, tempElementName);
                        }
                    }

                    if ((DT_Get_RandomVlaue() % g_mutatorFrequency) == 1)
                    {
                        if (isElementChangeStart == 0)
                        {
                            tempElementBufLength = temp->mutaterLength;
                            tempElementBuf = temp->mutaterValue;
                            tempElementName = temp->xpathName;
                            isElementChangeStart = 1;
                            goto abcd;
                        }
                    }
                }

                if ((length + temp->mutaterLength) < OUT_BUF_MAX_LENGTH)
                {
                    memcpy(buf + length, temp->mutaterValue, temp->mutaterLength);
                    length=length +temp->mutaterLength;
                }
                else
                {
                    goto ffff;
                }

                // 元素复制
                if ((isMutator == 1) && (mutator == 12) && (g_pitsMutator.mutatorElementCopy == 1))
                {
                    // 选中本元素了
                    if (DT_Get_RandomVlaue() % g_mutatorFrequency == 1)
                    {
                        // 11代表最大变异2047作为复制次数
                        int a = GaussRandU32(DT_Get_RandomVlaue() % 11);
                        int i = 0;
                        for (i = 0; i < a; i++)
                        {
                            if ((length + temp->mutaterLength) < OUT_BUF_MAX_LENGTH)
                            {
                                memcpy(buf + length, temp->mutaterValue, temp->mutaterLength);
                                length = length + temp->mutaterLength;
                            }
                            else
                            {
                                goto ffff;
                            }
                        }
                        if (g_onOffDebugMutatorPits)
                        {
                            printf("*********************copy element %s, %d count\r\n", temp->xpathName, a);
                        }
                    }
                }
            }
        }
        // 为了编译不出错误
        int a1 = 10;
abcd:
        a1 = a1;

        //跳过儿子们
        if (temp->mutatorElement->xmlElement->isTransformer == 1)
        {
            if(temp->next != NULL)
            {
                temp =temp->next;
            }
            else
            {
                temp=temp->parent;
                if(temp == parent)
                {
                    break;
                }
                while(temp->next ==NULL)
                {
                    temp =temp->parent;
                    if(temp == parent)
                    {
                        break;
                    }
                }
                if(temp == parent)
                {
                    break;
                }
                temp=temp->next;
            }

            continue;
        }
        
    }
    For_Tree_child_End(temp, parent)

ffff:
    return length;
}

int GetPitsBufNoMutator(SBinElement* parent, char* buf)
{       
    int length = 0;
    char* name1 = parent->mutatorElement->xmlElement->typeName;

    if ((strcmp(name1, "Number") == 0)
            || (strcmp(name1, "String") == 0)
            || (strcmp(name1, "Blob") == 0)
            || (strcmp(name1, "Flags") == 0)
            || (strcmp(name1, "Padding") == 0))
    {   
        if (parent->mutaterLength >0)
        {
            memcpy(buf,parent->mutaterValue, parent->mutaterLength);
        }
        
        return parent->mutaterLength;
    }

    SBinElement* temp = parent->children;
    For_Tree_child_Start(temp)
    {
        char* name = temp->mutatorElement->xmlElement->typeName;

        if ((strcmp(name, "Number") == 0)
            || (strcmp(name, "String") == 0)
            || (strcmp(name, "Blob") == 0)
            || (strcmp(name, "Flags") == 0)
            || (strcmp(name, "Padding") == 0))
        {
            // 有变异值则增加
            if (temp->mutaterLength != 0 )
            {
                if ((length + temp->mutaterLength) < OUT_BUF_MAX_LENGTH)
                {
                    memcpy(buf + length,temp->mutaterValue, temp->mutaterLength);
                    length = length + temp->mutaterLength; 
                }
                else
                {
                    goto ffff;
                }
            }
        }
    }
    For_Tree_child_End(temp, parent)
    
    ffff:
    return length;
}

#ifdef __cplusplus
}
#endif
