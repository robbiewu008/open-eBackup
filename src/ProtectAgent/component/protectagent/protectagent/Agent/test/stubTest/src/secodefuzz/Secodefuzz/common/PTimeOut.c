/*
版权所有 (c) 华为技术有限公司 2012-2018

作者:
wanghao 			w00296180

本模块提供检查测试例执行超时bug的功能
并提供设置测试例运行时间的能力

*/
#include "PCommon.h"

#ifdef __cplusplus
extern "C" {
#endif

void RunningSetTimeSecond(int second)
{
    g_global.runningTimeSecond = second;
}

void RunningTimeGetStart(void)
{
    if (g_global.runningTimeSecond == 0)
    {
        return;
    }
    
    g_globalThead.runningTimeStartTime = HwGetTime();
}

int RunningTimeIsOver(void)
{
    if (g_global.runningTimeSecond == 0)
    {
        return 0;
    }

    if ((g_globalThead.corpusModule.corpusM->runCount % SWITCH_CORPUS_COUNT)
        != (SWITCH_CORPUS_COUNT -1))
    {
        return 0;
    }
    
    int tempSecond = HwGetTime() - g_globalThead.runningTimeStartTime;
    if (tempSecond > g_global.runningTimeSecond)
    {
        g_globalThead.isRunningtimeComplete = 1;
        return 1;
    }

    return 0;
}

void TimeOutSetSecond(int second)
{
    g_global.timeOutSecond = second;
}

void TimeOutset(void)
{
    HwSetTimer(g_global.timeOutSecond);
}

void TimeOutClean(void)
{
    HwSetTimer(0);
}

void InitTimeOut(void)
{
    g_globalThead.isRunningtimeComplete = 0;
}


#ifdef __cplusplus
}
#endif

