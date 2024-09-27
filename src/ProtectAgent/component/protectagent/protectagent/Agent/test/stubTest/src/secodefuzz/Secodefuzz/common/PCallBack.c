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

void CallBackAddCrash(void (*fun)(void))
{
    g_global.crashCallBack = fun;
}


void CallBackAddTestCase(void (*fun)(void))
{
    g_global.testCaseCallBack = fun;
}


void CallBackRunCrash(void)
{
    if (g_global.crashCallBack)
    {
        g_global.crashCallBack();
    }
}

void CallBackRunTestCase(void)
{
    if (g_global.testCaseCallBack)
    {
        g_global.testCaseCallBack();
    }
}

#ifdef __cplusplus
}
#endif

