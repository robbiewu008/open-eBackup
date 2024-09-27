/*
��Ȩ���� (c) ��Ϊ�������޹�˾ 2012-2018

����:
wanghao 			w00296180

��ģ��ע���ºź�����exit�˳�������asan�˳�����

ֻ��ʼ��һ��
����ֻ�ṩ��ʼ��������������Ϊģ���ڲ�����

ȥ����ģ�鹦�ܣ�ֻҪע�͵�HAS_SIGNAL����������

*/
#include "PCommon.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef HAS_SIGNAL

static void ExitHandler(void) 
{
    if (g_global.isSIGINT) // ���������˳����Ͳ���ӡ������
    {
        return;
    }

    if (g_global.isCrash) // ����crash �˳����Ͳ���ӡ������
    {
        return;
    }

    // ����Ѿ��жϣ�������ڲ���������˳������д�ӡ��
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
    // û��������ɶҲ����
    // Ŀǰû���������ʱ���ǲ��ǲ�Ʒ�ߵģ��Ժ��޸�
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
    // ���������Ҫ�ķ���
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

    // ��asan�����ӡ����������pcָ���crash�������
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
    // ��һ��ע���ˣ��Ժ���������˵
    SetSigaction(SIGINT, SIGINTHandler); // ctrl +c  ���ǲ�Ҫע�������
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