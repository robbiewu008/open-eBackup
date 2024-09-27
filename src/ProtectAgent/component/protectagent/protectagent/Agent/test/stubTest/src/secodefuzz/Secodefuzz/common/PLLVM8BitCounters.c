/*
��Ȩ���� (c) ��Ϊ�������޹�˾ 2012-2018

����:
wanghao 			w00296180

��ģ��ʵ��֧��ѭ����串�Ƿ����Ĺ���

*/

#include "PCommon.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef HAS_TRACE_PC

void LlvmRecord8BitCounters(int idx, uintptr_t pc)
{
    g_globalThead.tracepcModule.bit8PerLoopCounters[idx]++;
    // ��������յ�
    if ((g_globalThead.tracepcModule.bit8PerLoopCounters[idx] == 1) && (g_globalThead.tracepcModule.bit8PcIdxHasBeenRecorded[idx] == 0))
    {
        // ����pcָ��ĺͣ�Ϊ��·������
        g_globalThead.tracepcModule.bit8PcsSum += pc;

        g_globalThead.tracepcModule.bit8PcIdxHasBeenRecorded[idx] = 1;
        g_globalThead.tracepcModule.bit8PcIdxTable[g_globalThead.tracepcModule.bit8PcIdxTatol++] = idx;
    }

    // �·��ֵ�ѭ����䣬Ҳ������ͬһ�����������ö�Σ�����
    if ((g_globalThead.tracepcModule.bit8PerLoopCounters[idx] != 1)&&(g_globalThead.tracepcModule.bit8LoopIdxHasBeenRecorded[idx] == 0))
    {
        g_globalThead.tracepcModule.bit8LoopIdxHasBeenRecorded[idx] = 1;
        g_globalThead.tracepcModule.bit8LoopIdxTable[g_globalThead.tracepcModule.bit8LoopIdxTatol++] = idx; // ��¼�������а�
    }
}

void LlvmDo8BitCounters(void)
{
    int i;

    for (i = 0; i < g_globalThead.tracepcModule.bit8LoopIdxTatol; i++)
    {
        int counter = g_globalThead.tracepcModule.bit8PerLoopCounters[g_globalThead.tracepcModule.bit8LoopIdxTable[i]];
        char bitCounters = g_globalThead.tracepcModule.bit8Loops[g_globalThead.tracepcModule.bit8LoopIdxTable[i]];
        int bit = 0;

        if(counter < 2) 
        {
            continue;
        }

        if (counter >= 65535) 
        {
            bit = 7;
        }
        else if (counter >= 1024) 
        {
            bit = 6;
        }
        else if (counter >= 256) 
        {
            bit = 5;
        }
        else if (counter >= 255) 
        {
            bit = 4;
        }
        else if (counter >= 16) 
        {
            bit = 3;
        }
        else if (counter >= 4) 
        {
            bit = 2;
        }
        else if (counter >= 3) 
        {
            bit = 1;
        }
        else if (counter >= 2) 
        {
            bit = 0;
        }

        // ����ѭ��������Ϊ��·������
        g_globalThead.tracepcModule.bit8LoopSum += bit + 1;

        if ((bitCounters & (1 << bit)) == 0)
        {
            g_globalThead.tracepcModule.bit8LoopTatol++;
            if (g_global.isPrintPc)
            {
                hw_printf("\r\nloop++,idx is %d; bit is %d, total is %d\r\n", g_globalThead.tracepcModule.bit8LoopIdxTable[i], bit, g_globalThead.tracepcModule.bit8LoopTatol);
            }

            if (g_global.loopModeWeight >0)
            {
                g_globalThead.tracepcModule.isHasNewFeature = 1;
                g_globalThead.tracepcModule.weight += g_global.loopModeWeight;
            }
            g_globalThead.tracepcModule.bit8Loops[g_globalThead.tracepcModule.bit8LoopIdxTable[i]] = 
            g_globalThead.tracepcModule.bit8Loops[g_globalThead.tracepcModule.bit8LoopIdxTable[i]] | (1 << bit);
        }
    }

    // ����·��hash
    g_globalThead.tracepcModule.bit8PcsSum ^= g_globalThead.tracepcModule.bit8LoopSum;

    for (i = 0; i < g_globalThead.tracepcModule.bit8PcIdxTatol; i++)
    {
        g_globalThead.tracepcModule.bit8PerLoopCounters[g_globalThead.tracepcModule.bit8PcIdxTable[i]] = 0;
        g_globalThead.tracepcModule.bit8LoopIdxHasBeenRecorded[g_globalThead.tracepcModule.bit8PcIdxTable[i]] = 0;
        g_globalThead.tracepcModule.bit8PcIdxHasBeenRecorded[g_globalThead.tracepcModule.bit8PcIdxTable[i]] = 0;

        g_globalThead.tracepcModule.bit8LoopIdxTable[i] = 0;
        g_globalThead.tracepcModule.bit8PcIdxTable[i] = 0;
    }

    for (i = 0; i < g_globalThead.tracepcModule.bit8HashTatol; i++)
    {
        // �Ѿ�����
        if (g_globalThead.tracepcModule.bit8Hashs[i] == g_globalThead.tracepcModule.bit8PcsSum)
        {
            break;
        }
    }

    // ����������洢
    if ((i == g_globalThead.tracepcModule.bit8HashTatol) && (i < MAX_Corpus_Hash_Num) &&  (g_globalThead.tracepcModule.bit8PcsSum != 0))
    {
        if (g_global.hashModeWeight > 0)
        {
            g_globalThead.tracepcModule.isHasNewFeature = 1;
            g_globalThead.tracepcModule.weight += g_global.hashModeWeight;
        }
        g_globalThead.tracepcModule.bit8Hashs[i] = g_globalThead.tracepcModule.bit8PcsSum;
        g_globalThead.tracepcModule.bit8HashTatol++;

        if (g_global.isPrintPc)
        {
            hw_printf("\r\nhash++,hash is %p; total is %d\r\n", (char*)g_globalThead.tracepcModule.bit8PcsSum, g_globalThead.tracepcModule.bit8HashTatol);
        }
    }

    g_globalThead.tracepcModule.bit8LoopIdxTatol = 0;
    g_globalThead.tracepcModule.bit8PcIdxTatol = 0;

    g_globalThead.tracepcModule.bit8PcsSum = 0;
    g_globalThead.tracepcModule.bit8LoopSum = 0;
}

int LlvmGetLoopTatol(void)
{
    return g_globalThead.tracepcModule.bit8LoopTatol;
}

int LlvmGetHashTatol(void)
{
    return g_globalThead.tracepcModule.bit8HashTatol;
}

void Init8BitCounters(void)
{
    
   
    if (g_globalThead.tracepcModule.bit8IsHasMalloc == 0)
    {
        // �������ܲŷ����ڴ棬�����ڴ�����������Խ�ʡ�ڴ�
        g_globalThead.tracepcModule.bit8Loops = (char *)HwMalloc(sizeof(char)*MAX_PC_NUM);
        g_globalThead.tracepcModule.bit8PerLoopCounters = (int *)HwMalloc(sizeof(int)*MAX_PC_NUM);
        g_globalThead.tracepcModule.bit8LoopIdxHasBeenRecorded = (char *)HwMalloc(sizeof(char)*MAX_PC_NUM);
        g_globalThead.tracepcModule.bit8LoopIdxTable = (int *)HwMalloc(sizeof(int)*MAX_PC_NUM);
        g_globalThead.tracepcModule.bit8PcIdxHasBeenRecorded = (char *)HwMalloc(sizeof(char)*MAX_PC_NUM);
        g_globalThead.tracepcModule.bit8PcIdxTable = (int *)HwMalloc(sizeof(int)*MAX_PC_NUM);
        g_globalThead.tracepcModule.bit8Hashs = (uintptr_t *)HwMalloc(sizeof(uintptr_t)*MAX_Corpus_Hash_Num);
        g_globalThead.tracepcModule.bit8IsHasMalloc = 1;
    }
    
    HwMemset(g_globalThead.tracepcModule.bit8Loops, 0, sizeof(char)*MAX_PC_NUM);
    HwMemset(g_globalThead.tracepcModule.bit8PerLoopCounters, 0, sizeof(int)*MAX_PC_NUM);
    HwMemset(g_globalThead.tracepcModule.bit8LoopIdxHasBeenRecorded, 0, sizeof(char)*MAX_PC_NUM);
    HwMemset(g_globalThead.tracepcModule.bit8LoopIdxTable, 0, sizeof(int)*MAX_PC_NUM);
    HwMemset(g_globalThead.tracepcModule.bit8PcIdxHasBeenRecorded, 0, sizeof(char)*MAX_PC_NUM);
    HwMemset(g_globalThead.tracepcModule.bit8PcIdxTable, 0, sizeof(int)*MAX_PC_NUM);
    HwMemset(g_globalThead.tracepcModule.bit8Hashs, 0, sizeof(uintptr_t)*MAX_Corpus_Hash_Num);
    g_globalThead.tracepcModule.bit8LoopIdxTatol = 0;
    g_globalThead.tracepcModule.bit8PcIdxTatol = 0;
    g_globalThead.tracepcModule.bit8HashTatol = 0;
    g_globalThead.tracepcModule.bit8LoopTatol = 0;
}

void LlvmClean8BitCounters(void)
{
    g_globalThead.tracepcModule.bit8IsHasMalloc = 0;
    if (g_globalThead.tracepcModule.bit8Loops)
    {
        HwFree(g_globalThead.tracepcModule.bit8Loops);
        g_globalThead.tracepcModule.bit8Loops = NULL;
    }
    if (g_globalThead.tracepcModule.bit8PerLoopCounters)
    {
        HwFree(g_globalThead.tracepcModule.bit8PerLoopCounters);
        g_globalThead.tracepcModule.bit8PerLoopCounters = NULL;
    }
    if (g_globalThead.tracepcModule.bit8LoopIdxHasBeenRecorded)
    {
        HwFree(g_globalThead.tracepcModule.bit8LoopIdxHasBeenRecorded);
        g_globalThead.tracepcModule.bit8LoopIdxHasBeenRecorded = NULL;
    }
    if (g_globalThead.tracepcModule.bit8LoopIdxTable)
    {
        HwFree(g_globalThead.tracepcModule.bit8LoopIdxTable);
        g_globalThead.tracepcModule.bit8LoopIdxTable = NULL;
    }
    if (g_globalThead.tracepcModule.bit8PcIdxHasBeenRecorded)
    {
        HwFree(g_globalThead.tracepcModule.bit8PcIdxHasBeenRecorded);
        g_globalThead.tracepcModule.bit8PcIdxHasBeenRecorded = NULL;
    }
    if (g_globalThead.tracepcModule.bit8PcIdxTable)
    {
        HwFree(g_globalThead.tracepcModule.bit8PcIdxTable);
        g_globalThead.tracepcModule.bit8PcIdxTable = NULL;
    }
    if (g_globalThead.tracepcModule.bit8Hashs)
    {
        HwFree(g_globalThead.tracepcModule.bit8Hashs);
        g_globalThead.tracepcModule.bit8Hashs = NULL;
    }
}

#else

void LlvmRecord8BitCounters(int Idx, uintptr_t PC)
{
}

void LlvmDo8BitCounters(void)
{
}

void Init8BitCounters(void)
{
}

int LlvmGetLoopTatol(void)
{
    return 0;
}

int LlvmGetHashTatol(void)
{
    return 0;
}

void LlvmClean8BitCounters(void)
{
    
}

#endif

#ifdef __cplusplus
}
#endif