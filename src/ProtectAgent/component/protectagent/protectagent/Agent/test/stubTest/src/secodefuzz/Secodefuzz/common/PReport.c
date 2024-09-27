/*
版权所有 (c) 华为技术有限公司 2012-2018

作者:
wanghao 			w00296180

本模块提供记录报告功能

*/
#include "PCommon.h"

#ifdef __cplusplus
extern "C" {
#endif

int ReportGetTime(void)
{
    return HwGetTime();
}

void ReportSetRunningTestCaseName(char * name)
{
    if (name != NULL)
    {
        HwMemcpy(g_globalThead.runningTestcaseName, name, Instrlen(name) + 1);
    }
    else
    {
        g_globalThead.runningTestcaseName[0] = 0;
    }
}

void ReportSetPath(char* path)
{
    int size;

    if (path == NULL)
    {
        g_global.reportPath[0] = 0;
        return;
    }
    
    g_global.reportPath[0] = 0;
    size = hw_sprintf(g_global.reportPath, "%s_", path);
    
    size += hw_sprintf(g_global.reportPath + size, "%s", HwGetDate());

    char* temp_head = (char*)"##############################################\r\n";

    WriteToFileFail(temp_head, sizeof(temp_head), g_global.reportPath);

    temp_head = (char*)GetVersion();

    WriteToFileFail(temp_head, sizeof(temp_head), g_global.reportPath);

    temp_head = (char*)"\r\n";

    WriteToFileFail(temp_head, sizeof(temp_head), g_global.reportPath);

    temp_head = (char*)"##############################################\r\n";

    WriteToFileFail(temp_head, sizeof(temp_head), g_global.reportPath);

    temp_head = (char*)"#\r\n";

    WriteToFileFail(temp_head, sizeof(temp_head), g_global.reportPath);

    temp_head = (char*)"g_seed-run_count-run_time-ReadCorpus-NewCorpus-CovPcNum-TotalPcNum-covEdgeNum-covloopNum-covPathNum-memhook-numberhook----result----testcase_name----\r\n";

    WriteToFileFail(temp_head, sizeof(temp_head), g_global.reportPath);
}

void ReportSetFixPathName(char* pathName)
{
    if (pathName == NULL)
    {
        g_global.reportPath[0] = 0;
        return;
    }
    
    g_global.reportPath[0] = 0;
    hw_sprintf(g_global.reportPath, "%s", pathName);

    char* temp_head = (char*)"##############################################\r\n";

    WriteToFileFail(temp_head, sizeof(temp_head), g_global.reportPath);

    temp_head = (char*)GetVersion();

    WriteToFileFail(temp_head, sizeof(temp_head), g_global.reportPath);

    temp_head = (char*)"\r\n";

    WriteToFileFail(temp_head, sizeof(temp_head), g_global.reportPath);

    temp_head = (char*)"##############################################\r\n";

    WriteToFileFail(temp_head, sizeof(temp_head), g_global.reportPath);

    temp_head = (char*)"#\r\n";

    WriteToFileFail(temp_head, sizeof(temp_head), g_global.reportPath);

    temp_head = (char*)"g_seed-run_count-run_time-ReadCorpus-NewCorpus-CovPcNum-TotalPcNum-covEdgeNum-covloopNum-covPathNum-memhook-numberhook----result----testcase_name----\r\n";

    WriteToFileFail(temp_head, sizeof(temp_head), g_global.reportPath);
}


void ReportWriteSucceedTestCase(char *testCaseName, int seed, int runCount, int runTime, int readCorpusNum, int newCorpusNum)
{
    if (Instrlen(g_global.reportPath) == 0)
    {
        return;
    }

    char *txt;
    txt = HwMalloc(1000);
    txt[0] = 0;
    int size = 0;

    size += hw_sprintf(txt + size, "%6d%10d%8ds%11d%10d%9d%11d%11d%11d%11d%8d%11d   ...pass  %s\r\n", 
        seed, runCount, runTime, readCorpusNum, newCorpusNum,
        LlvmGetPcTatol(), g_global.numGuards, LlvmGetEdgeTatol(), LlvmGetLoopTatol(), LlvmGetHashTatol(), 
        LlvmDataMemGetCount(), LlvmDataNumberGetCount(),testCaseName);
    WriteToFileFail(txt, size, g_global.reportPath);
    HwFree(txt);
}

void ReportWriteFailedTestCase(char *testCaseName, int seed, int runCount, int runTime, int readCorpusNum, int newCorpusNum)
{
    if (Instrlen(g_global.reportPath) == 0)
    {
        return;
    }

    if (Instrlen(g_globalThead.runningTestcaseName) == 0)
    {
        return;
    }

    char *txt;
    txt = HwMalloc(1000);
    txt[0] = 0;
    int size = 0;

    size += hw_sprintf(txt + size, "%6d%10d%8ds%11d%10d%9d%11d%11d%11d%11d%8d%11d   ...failed %s\r\n",
        seed, runCount, runTime, readCorpusNum, newCorpusNum, 
         LlvmGetPcTatol(), g_global.numGuards, LlvmGetEdgeTatol(), LlvmGetLoopTatol(), LlvmGetHashTatol(), 
        LlvmDataMemGetCount(), LlvmDataNumberGetCount(), testCaseName);
    WriteToFileFail(txt, size, g_global.reportPath);
    HwFree(txt);
}

void ReportWriteFailedTestCase1(void)
{

#ifdef SUPPORT_M_THREAD 
    return;
#endif
     // 记录失败的测试例报告
    ReportWriteFailedTestCase(
            g_globalThead.testcaseName, fuzzSeed, fuzzi, 0, CorpusGetReadCorpusNum(), CorpusGetNewCorpusNum());
}

// 不同测试例之间，要调用初始化函数清内存
void InitReport(void)
{
}

#ifdef __cplusplus
}
#endif

