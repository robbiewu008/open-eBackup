/*
版权所有 (c) 华为技术有限公司 2012-2018

作者:
wanghao 			w00296180

本模块注册新号函数，exit退出函数，asan退出函数

只初始化一次
对外只提供初始化函数，其他均为模块内部函数

去掉本模块功能，只要注释掉HAS_SIGNAL的声明即可

*/
#include "PCommon.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef HAS_SIGNAL

static void ExitHandler(void) 
{
    if (g_global.isSIGINT) // 来自主动退出，就不打印样本了
    {
        return;
    }

    if (g_global.isCrash) // 来自crash 退出，就不打印样本了
    {
        return;
    }

    // 里边已经判断，如果不在测试例里边退出不会有打印的
    int num= CorpusShowCur();
    if (num)
    {
        LlvmHookPrintStackTrace();
        hw_printf("\r\n  ********The program exit for test fail!!!********  \r\n ");
    }
    else
    {
        hw_printf("\r\n  ********The program exit for run complete !!!********  \r\n ");
    }

    if (g_globalThead.isPass == 0)
    {
        return;
    }
    
    HwExit(0);
}

#ifndef _MSC_VER 
static void CrashHandler(int aaa, siginfo_t* bbb, void* ccc) 
{
    hw_printf("\r\n  ********The program exit for crash !!!********  \r\n ");
    CorpusShowCur();
    LlvmHookPrintStackTrace();
    g_global.isCrash = 1;
    HwExit(aaa);
}

static void SIGINTHandler(int aaa, siginfo_t* bbb, void* ccc) 
{
#ifndef SUPPORT_M_THREAD 
    if (g_globalThead.isDoingLeak)
    {
        hw_printf("\r\n  ********SIGINT Handler !!!********  \r\n ");
        hw_printf("\r\n  ********For CTRL +C !!!********  \r\n ");
        hw_printf("\r\n  ********is doing leak, so not exit********  \r\n ");
        return;
    }
#endif

    hw_printf("\r\n  ********SIGINT Handler !!!********  \r\n ");
    hw_printf("\r\n  ********The program exit for CTRL +C !!!********  \r\n ");
    LlvmHookPrintStackTrace();
    g_global.isSIGINT = 1;
    HwExit(aaa);
}

static void SIGTERMHandler(int aaa, siginfo_t* bbb, void* ccc) 
{
    hw_printf("\r\n  ********SIGTERM Handler !!!********  \r\n ");
    CrashHandler(aaa, bbb, ccc);
}

static void SIGSEGVHandler(int aaa, siginfo_t* bbb, void* ccc) 
{
    hw_printf("\r\n  ********SIGSEGV Handler !!!********  \r\n ");
    CrashHandler(aaa, bbb, ccc);
}

static void SIGBUSHandler(int aaa, siginfo_t* bbb, void* ccc) 
{
    hw_printf("\r\n  ********SIGBUS Handler !!!********  \r\n ");
    CrashHandler(aaa, bbb, ccc);
}

static void SIGABRTHandler(int aaa, siginfo_t* bbb, void* ccc) 
{
    hw_printf("\r\n  ********SIGABRT Handler !!!********  \r\n ");
    CrashHandler(aaa, bbb, ccc);
}

static void SIGILLHandler(int aaa, siginfo_t* bbb, void* ccc) 
{
    hw_printf("\r\n  ********SIGILL Handler !!!********  \r\n ");
    CrashHandler(aaa, bbb, ccc);
}

static void SIGFPEHandler(int aaa, siginfo_t* bbb, void* ccc) 
{
    hw_printf("\r\n  ********SIGFPE Handler !!!********  \r\n ");
    CrashHandler(aaa, bbb, ccc);
}

static void SIGXFSZHandler(int aaa, siginfo_t* bbb, void* ccc) 
{
    hw_printf("\r\n  ********SIGXFSZ Handler !!!********  \r\n ");
    CrashHandler(aaa, bbb, ccc);
}

static void SIGTIMEHandler(int aaa, siginfo_t* bbb, void* ccc) 
{
    // 没有设置则啥也不错
    // 目前没区分这个定时器是不是产品线的，以后修改
    if (g_global.timeOutSecond == 0)
        return;
    
    hw_printf("\r\n  ********SIGTIME(time out) Handler !!!********  \r\n ");
    CrashHandler(aaa, bbb, ccc);
}
#endif

void AsanHandler() 
{
    hw_printf("\r\n  ********The program exit from asan !!!********  \r\n ");
#ifndef SUPPORT_M_THREAD 
    g_globalThead.isNeedRecordCorpus = 2;
#endif
    CorpusShowCur();
}

void AsanHandlerNoDie() 
{
    // 这个东东块要荒废了
    hw_printf("\r\n  ********New asan bug report !!!********  \r\n ");
#ifndef SUPPORT_M_THREAD 
    g_globalThead.isNeedRecordCorpus = 2;
#endif
    CorpusShowCur();
}

void AsanHandlerReport(char* report) 
{
    if (g_global.isEnableAsanReport == 0)
    {
        return;
    }

#ifdef SUPPORT_M_THREAD 
    return;
#endif
    
    hw_printf("\r\n  ********New asan bug report !!!!********  \r\n ");
    g_globalThead.isNeedRecordCorpus = 2;
    g_globalThead.isfoundfailed = 1;
    CorpusShowCur();

    // 将asan报告打印出来，利用pc指针和crash样本配对
    if (g_globalThead.isNeedRecordCrash != 0)
    {
        CorpusAsanReportWrite(report);
    }
}

#ifndef _MSC_VER 
static void SetSigaction(int signum, void (*callback)(int, siginfo_t *, void *)) 
{
  struct sigaction sigact = {};
    if (sigaction(signum, NULL, &sigact)) 
    {
        hw_printf("secodefuzz: sigaction failed with %d\n", 1);
        HwExit(EXIT_CODE);
    }
    
    if (sigact.sa_flags & SA_SIGINFO) 
    {
        if (sigact.sa_sigaction)
        {
            return;
        }
    } 
    else 
    {
        if (sigact.sa_handler != SIG_DFL && sigact.sa_handler != SIG_IGN && sigact.sa_handler != SIG_ERR)
        {
            return;
        }
    }

    sigact.sa_sigaction = callback;
    if (sigaction(signum, &sigact, 0)) 
    {
        hw_printf("secodefuzz: sigaction failed with %d\n", 1);
        HwExit(EXIT_CODE);
    }
}
#endif

void InitSignalCallback(void)
{
    if (g_global.hasSignalInit == 1)
    {
        return;
    }
    
    g_global.hasSignalInit = 1;

    #ifndef _MSC_VER 
    // 先一起都注册了，以后有问题再说
    SetSigaction(SIGINT, SIGINTHandler); // ctrl +c  还是不要注册这个了
    SetSigaction(SIGTERM, SIGTERMHandler);
    SetSigaction(SIGSEGV, SIGSEGVHandler);
    SetSigaction(SIGBUS, SIGBUSHandler);
    SetSigaction(SIGABRT, SIGABRTHandler);
    SetSigaction(SIGILL, SIGILLHandler);
    SetSigaction(SIGFPE, SIGFPEHandler);
    SetSigaction(SIGXFSZ, SIGXFSZHandler);
    SetSigaction(SIGALRM, SIGTIMEHandler);

    #endif

    LlvmHookRegisterAsanCallBack(AsanHandler);
    LlvmHookRegisterAsanCallBackno(AsanHandlerNoDie);
    LlvmHookRegisterAsanCallBackReport(AsanHandlerReport);

    atexit(ExitHandler);
    
    return;
}

#else

void InitSignalCallback(void)
{
    return;
}
#endif

#ifdef __cplusplus
}
#endif