/*
��Ȩ���� (c) ��Ϊ�������޹�˾ 2012-2018

����:
wanghao 			w00296180

��ģ���ṩ��������


*/
#include "PCommon.h"

#ifdef __cplusplus
extern "C" {
#endif


int CorpusPriorityAdd(int pos, int weight)
{
    int i =0;

    //������ȼ�����
    if (weight >= NewLoop_Weight)
    {
        int tempWeight = weight;

        //���100������Ҳûɶ��
        if (tempWeight > 100)
        {
            tempWeight = 100;
        }

        // ����Ȩ�ظ�����pos������
        for (i =0; i < tempWeight; i++)
        {
        
            g_globalThead.corpusModule.priorityCorpus[g_globalThead.corpusModule.priorityCorpusNum] = pos;
            g_globalThead.corpusModule.priorityCorpusNum++;

            // ���ʱ�ع�����̭���������еĳ�Ա
            if (g_globalThead.corpusModule.priorityCorpusNum == MAX_Corpus_Priority_Num)
            {
                g_globalThead.corpusModule.priorityCorpusNum = 0;
            }

        }

    }

    //�����ȼ�����
    if (weight > NewEdge_Weight)
    {
        int tempWeight = weight;

        //���100������Ҳûɶ��
        if (tempWeight > 100)
        {
            tempWeight = 100;
        }

        // ����Ȩ�ظ�����pos������
        for (i =0; i < tempWeight; i++)
        {
        
            g_globalThead.corpusModule.midPriorityCorpus[g_globalThead.corpusModule.midPriorityCorpusNum] = pos;
            g_globalThead.corpusModule.midPriorityCorpusNum++;

            // ���ʱ�ع�����̭���������еĳ�Ա
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

    // �ٷ�֮30��Ȩ�ظߵ�������ѡȡ
    if ((choice < 30) && (g_globalThead.corpusModule.priorityCorpusNum > 0))
    {
        int pos1 = 0;
        pos1 = RAND_RANGE(0, g_globalThead.corpusModule.priorityCorpusNum - 1);
        return g_globalThead.corpusModule.priorityCorpus[pos1];
    }
     // �ٷ�֮30��Ȩ���е�������ѡȡ
    else if ((choice < 60) && (g_globalThead.corpusModule.midPriorityCorpusNum > 0))
    {
        int pos1 = 0;
        pos1 = RAND_RANGE(0, g_globalThead.corpusModule.midPriorityCorpusNum - 1);
        return g_globalThead.corpusModule.midPriorityCorpus[pos1];
    }
    else // �ٷ�֮40������������ѡȡ
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

    //��1��ʼ����0������Ϊ������������Զ����̭
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

