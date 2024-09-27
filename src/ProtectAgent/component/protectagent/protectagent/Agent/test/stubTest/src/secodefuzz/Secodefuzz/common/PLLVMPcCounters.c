/*
版权所有 (c) 华为技术有限公司 2012-2018

作者:
wanghao 			w00296180

本模块实现编译器回调函数

实现记录覆盖率的功能

如果编译器不支持，需要注释掉HAS_LLVM声明


下边两个函数为本模块实现的对外提供的覆盖率反馈函数，
如果有其他方法也能实现覆盖率反馈，如afl，则实现下边两函数即可替换本模块
LlvmTracePcIsHasNewFeature
LlvmTracePcStartFeature
LlvmTracePcEndFeature

*/

#include "PCommon.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef HAS_TRACE_PC

__attribute__((weak)) void __sanitizer_symbolize_pc(void *, const char *fmt, char *out_buf, size_t out_buf_size);
__attribute__((weak)) void __sanitizer_dump_coverage(const uintptr_t *, uintptr_t);

static void InitMalloc(void)
{
    if (g_globalThead.tracepcModule.tracePcIsHasMalloc == 0)
    {
        g_globalThead.tracepcModule.tracePcPcs = (uintptr_t *)HwMalloc(sizeof(uintptr_t)*MAX_PC_NUM);
        g_globalThead.tracepcModule.tracePcEdges = (uintptr_t *)HwMalloc(sizeof(uintptr_t)*MAX_PC_NUM);
        g_globalThead.tracepcModule.tracePcPcsTemp = (uintptr_t *)HwMalloc(sizeof(uintptr_t)*MAX_PC_NUM);

        HwMemset(g_globalThead.tracepcModule.tracePcPcs, 0, sizeof(uintptr_t)*MAX_PC_NUM);
        HwMemset(g_globalThead.tracepcModule.tracePcEdges, 0, sizeof(uintptr_t)*MAX_PC_NUM);
        HwMemset(g_globalThead.tracepcModule.tracePcPcsTemp, 0, sizeof(uintptr_t)*MAX_PC_NUM);
        g_globalThead.tracepcModule.tracePcIsHasMalloc = 1;
    }   
}

void LlvmRecordGuards(uint32_t *start, uint32_t *stop)
{
    uint32_t *p;
    if (start == stop || *start) 
    {
        return;
    }

    for (p = start; p < stop; p++) 
    {
        g_global.numGuards++;
        if (g_global.numGuards == MAX_PC_NUM) 
        {
            hw_printf(
                  "WARNING: The binary has too many instrumented PCs.\n"
                  "         You may want to reduce the size of the binary\n"
                  "         for more efficient fuzzing and precise coverage data\n");
        }
        *p = g_global.numGuards % MAX_PC_NUM;
    }
    g_global.modules_Start[g_global.numModules] = start;
    g_global.modules_Stop[g_global.numModules] = stop;
    g_global.numModules++;
}

void LlvmDoTracePc(int idx, uintptr_t pc)
{
    if (g_globalThead.tracepcModule.isEnableFeature == 0)
    {
        return;
    }

    LlvmRecordPcCounters(idx, pc);
    LlvmRecord8BitCounters(idx, pc);

    {
        g_globalThead.tracepcModule.pc2Sum += pc;

        int idx_sum = (size_t)g_globalThead.tracepcModule.pc2Sum % (MAX_PC_NUM);
        LlvmRecordPcCountersEdga(idx_sum, g_globalThead.tracepcModule.pc2Sum);
        g_globalThead.tracepcModule.pc2Sum = pc;
    }
}

void LlvmRecordPcCounters(int idx, uintptr_t pc)
{
    // 只对第一次负责，目前忽略其他
    if (g_globalThead.tracepcModule.tracePcPcs[idx] == 0)
    {
        if (g_global.blockModeWeight > 0)
        {
            g_globalThead.tracepcModule.isHasNewFeature = 1;
            g_globalThead.tracepcModule.weight += g_global.blockModeWeight;
        }
        g_globalThead.tracepcModule.tracePcPcTatol++;

        if (g_global.isPrintPc)
        {
            hw_printf("\r\npc++, ps is %p\r\n", (char*)pc);
            g_globalThead.pcDescr[0] = 0;
            if (__sanitizer_symbolize_pc)
            {
                __sanitizer_symbolize_pc((void*)pc, "%p %F %L", g_globalThead.pcDescr, sizeof(g_globalThead.pcDescr));
            }

            g_globalThead.pcDescr[sizeof(g_globalThead.pcDescr) - 1] = 0;
            hw_printf("\tNEW_PC(idx-%d;CovRate-%d\\%d): %s\n", idx, g_globalThead.tracepcModule.tracePcPcTatol, g_global.numGuards, g_globalThead.pcDescr);
        }

        g_globalThead.tracepcModule.tracePcPcs[idx] = pc;
    }
}

void LlvmRecordPcCountersEdga(int idx, uintptr_t pcSum)
{
    // 只对第一次负责，目前忽略其他
    if (g_globalThead.tracepcModule.tracePcEdges[idx] == 0)
    {
        if (g_global.edgeModeWeight > 0)
        {
            g_globalThead.tracepcModule.isHasNewFeature = 1;
            g_globalThead.tracepcModule.weight += g_global.edgeModeWeight;
        }
        g_globalThead.tracepcModule.tracePcEdgeTatol++;

        g_globalThead.tracepcModule.tracePcEdges[idx] = pcSum;

        if (g_global.isPrintPc)
        {
            hw_printf("\r\nPcsEdga++,idx is %d; pcSum is %p, all edge is %d\r\n", idx, (char*)pcSum, g_globalThead.tracepcModule.tracePcEdgeTatol);
        }
    }
}

ALWAYS_INLINE uintptr_t GetPreviousInstructionPc(uintptr_t pc) 
{ 
    // see sanitizer_common GetPreviousInstructionPc for full implementation.
    return pc - 1;
}

void LlvmSetIsDumpCoverage(int isDumpCoverage)
{
    g_global.isDumpCoverage = isDumpCoverage;
}

void LlvmDumpCoverage(void)
{
    int i;

    if (g_global.isDumpCoverage == 0)
    {
        return;
    }

    for (i = 0; i < MAX_PC_NUM; i++)
    {
        if (g_globalThead.tracepcModule.tracePcPcs[i] != 0)
        {
            g_globalThead.tracepcModule.tracePcPcsTemp[i] = GetPreviousInstructionPc(g_globalThead.tracepcModule.tracePcPcs[i]);
        }
    }

    if (__sanitizer_dump_coverage)
    {
        __sanitizer_dump_coverage(g_globalThead.tracepcModule.tracePcPcsTemp, MAX_PC_NUM);
    }
}

void LlvmSetIsPrintNewPC(int isPrintPC)
{
    g_global.isPrintPc = isPrintPC;
}

void InitPcCounters(void)
{
    InitMalloc();
        
    HwMemset(g_globalThead.tracepcModule.tracePcPcs, 0, sizeof(uintptr_t) * MAX_PC_NUM);
    HwMemset(g_globalThead.tracepcModule.tracePcEdges, 0, sizeof(uintptr_t) * MAX_PC_NUM);
    g_globalThead.tracepcModule.tracePcPcTatol = 0;
    g_globalThead.tracepcModule.tracePcEdgeTatol = 0;
}

void LlvmCleanPcCounters(void)
{
    g_globalThead.tracepcModule.tracePcIsHasMalloc = 0;
    if (g_globalThead.tracepcModule.tracePcPcs)
    {
        HwFree(g_globalThead.tracepcModule.tracePcPcs);
        g_globalThead.tracepcModule.tracePcPcs = NULL;
    }
    if (g_globalThead.tracepcModule.tracePcEdges)
    {
        HwFree(g_globalThead.tracepcModule.tracePcEdges);
        g_globalThead.tracepcModule.tracePcEdges = NULL;
    }
    if (g_globalThead.tracepcModule.tracePcPcsTemp)
    {
        HwFree(g_globalThead.tracepcModule.tracePcPcsTemp);
        g_globalThead.tracepcModule.tracePcPcsTemp = NULL;
    }
}

int LlvmGetPcTatol(void)
{
    return g_globalThead.tracepcModule.tracePcPcTatol;
}

int LlvmGetEdgeTatol(void)
{
    return g_globalThead.tracepcModule.tracePcEdgeTatol;
}

#else

void LlvmRecordGuards(uint32_t *start, uint32_t *stop)
{
}

void LlvmDoTracePc(int idx, uintptr_t pc)
{
}

void LlvmRecordPcCounters(int idx, uintptr_t pc)
{
}

void LlvmRecordPcCountersEdga(int idx, uintptr_t pcSum)
{
}

void LlvmSetIsDumpCoverage(int is_dump_coverage)
{
}

void LlvmSetIsPrintNewPC(int isPrintPC)
{
}

void LlvmDumpCoverage(void)
{
}

void LlvmCleanPcCounters(void)
{

}

int LlvmGetPcTatol(void)
{
    return 0;
}

int LlvmGetEdgeTatol(void)
{
    return 0;
}

void InitPcCounters(void)
{
}

#endif

#ifdef __cplusplus
}
#endif