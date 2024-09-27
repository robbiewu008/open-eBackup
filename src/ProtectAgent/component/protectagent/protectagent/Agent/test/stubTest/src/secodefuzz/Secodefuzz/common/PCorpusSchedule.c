/*
版权所有 (c) 华为技术有限公司 2012-2018

作者:
wanghao 			w00296180

本模块提供样本调度


*/
#include "PCommon.h"

#ifdef __cplusplus
extern "C" {
#endif


int CorpusPriorityAdd(int pos, int weight)
{
    int i =0;

    //最高优先级队列
    if (weight >= NewLoop_Weight)
    {
        int tempWeight = weight;

        //最多100，多了也没啥用
        if (tempWeight > 100)
        {
            tempWeight = 100;
        }

        // 增加权重个数的pos到队列
        for (i =0; i < tempWeight; i++)
        {
        
            g_globalThead.corpusModule.priorityCorpus[g_globalThead.corpusModule.priorityCorpusNum] = pos;
            g_globalThead.corpusModule.priorityCorpusNum++;

            // 最大时回滚，淘汰最早进入队列的成员
            if (g_globalThead.corpusModule.priorityCorpusNum == MAX_Corpus_Priority_Num)
            {
                g_globalThead.corpusModule.priorityCorpusNum = 0;
            }

        }

    }

    //中优先级队列
    if (weight > NewEdge_Weight)
    {
        int tempWeight = weight;

        //最多100，多了也没啥用
        if (tempWeight > 100)
        {
            tempWeight = 100;
        }

        // 增加权重个数的pos到队列
        for (i =0; i < tempWeight; i++)
        {
        
            g_globalThead.corpusModule.midPriorityCorpus[g_globalThead.corpusModule.midPriorityCorpusNum] = pos;
            g_globalThead.corpusModule.midPriorityCorpusNum++;

            // 最大时回滚，淘汰最早进入队列的成员
            if (g_globalThead.corpusModule.midPriorityCorpusNum == MAX_Corpus_Priority_Num)
            {
                g_globalThead.corpusModule.midPriorityCorpusNum = 0;
            }

        }

    }

    
    return 0;
}

int CorpusSelect(void)
{
   int choice =RAND_RANGE(1, 100);

    // 百分之30在权重高的样本里选取
    if ((choice < 30) && (g_globalThead.corpusModule.priorityCorpusNum > 0))
    {
        int pos1 = 0;
        pos1 = RAND_RANGE(0, g_globalThead.corpusModule.priorityCorpusNum - 1);
        return g_globalThead.corpusModule.priorityCorpus[pos1];
    }
     // 百分之30在权重中的样本里选取
    else if ((choice < 60) && (g_globalThead.corpusModule.midPriorityCorpusNum > 0))
    {
        int pos1 = 0;
        pos1 = RAND_RANGE(0, g_globalThead.corpusModule.midPriorityCorpusNum - 1);
        return g_globalThead.corpusModule.midPriorityCorpus[pos1];
    }
    else // 百分之40在所有样本里选取
    {
        int pos1 = 0;
        pos1 = RAND_RANGE(0, g_globalThead.corpusModule.corpusM->corpusNum - 1);
        return pos1;
    }
}

int CorpusDiscard(void)
{
    int mincount = 0;
    int pos = 0;
    int i = 0;

    //从1开始，第0个样本为代码样本，永远不淘汰
    for (i = 1; i < g_globalThead.corpusModule.corpusM->corpusNum; i++)
    {
        if (mincount > g_globalThead.corpusModule.corpusM->corpus[i]->mutatorCount)
        {
            mincount = g_globalThead.corpusModule.corpusM->corpus[i]->mutatorCount;
            pos = i;
        }
    }
    return pos;
}

void CorpusParaValueGet(int corpusPos, int paraPos, char **buf, int *len)
{
    *buf = g_globalThead.corpusModule.corpusM->corpus[corpusPos]->para[paraPos].value;
    *len = g_globalThead.corpusModule.corpusM->corpus[corpusPos]->para[paraPos].len;
    
    return;
}

#ifdef __cplusplus
}
#endif

