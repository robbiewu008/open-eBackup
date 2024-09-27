/*
版权所有 (c) 华为技术有限公司 2012-2018

作者:
wanghao 			w00296180

本模块实现运行单测试用例后进行内存泄露检测
*/

#include "PCommon.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef HAS_LEAK_CHECK

__attribute__((weak)) int __lsan_do_recoverable_leak_check(void);
__attribute__((weak)) int __lsan_do_leak_check(void);
__attribute__((weak)) int __lsan_disable(void);
__attribute__((weak)) int __lsan_enable(void);
__attribute__((weak)) void __sanitizer_print_memory_profile(size_t a, size_t b);
__attribute__((weak)) int __sanitizer_install_malloc_and_free_hooks(void (*malloc_hook)(const volatile void *, size_t), void (*free_hook)(const volatile void *));

// Crash on a single malloc that exceeds the rss limit.
void HandleMalloc(size_t size) 
{
    if (!g_global.mallocLimitMb || (size >> 20) < (size_t)g_global.mallocLimitMb)
    {
        return;
    }

    hw_printf("\r\n  ********The program exit for out-of-memory (malloc(%zd))!!!********  \r\n ", size);
    hw_printf("   To change the out-of-memory limit please call DT_SetStopOutOfMemory next DT_SetCheckOutOfMemory\n\n");

    CorpusShowCur();
    LlvmHookPrintStackTrace();
    g_global.isCrash = 1;
    HwExit(EXIT_CODE_MallocLimit);
}

ATTRIBUTE_INTERFACE
ATT_NO_SANITIZE_ALL
void MallocHook(const volatile void *ptr, size_t size) 
{
    if ((g_globalThead.isRunningTestCode == 0) 
        || (g_global.startCheckOutOfMemory == 0) 
        ||(g_global.isSelfMalloc ==1))
    {
        return;
    }

    size_t N = g_global.mallocs++;
    HandleMalloc(size);
    if (g_global.mallocDebug) 
    {
        hw_printf("malloc[%zd] %p %zd\n", N, ptr, size);
        LlvmHookPrintStackTrace();
    }
}

ATTRIBUTE_INTERFACE
ATT_NO_SANITIZE_ALL
void FreeHook(const volatile void *ptr) 
{
    if ((g_globalThead.isRunningTestCode == 0) 
        || (g_global.startCheckOutOfMemory == 0) 
        ||(g_global.isSelfMalloc ==1))
    {
        return;
    }
    
    size_t N = g_global.frees++;
    if (g_global.mallocDebug)
    {
        hw_printf("free[%zd]   %p\n", N, ptr);
        LlvmHookPrintStackTrace();
    }
}

void MemoryAddCallBack(void)
{
    if (g_global.hasAddMallocCallBack == 1) 
    {
        return; 
    }
    g_global.hasAddMallocCallBack = 1;
    
    if (__sanitizer_install_malloc_and_free_hooks)
    {
        __sanitizer_install_malloc_and_free_hooks(MallocHook, FreeHook);

    }
    return;
}

void LlvmEnableLeakCheck(int count)
{
    g_global.enableDoLeakCheck = 1;
    g_globalThead.doLeakCount = count;
    MemoryAddCallBack();
    g_global.startCheckOutOfMemory = 1;
}

void LlvmDoLeakCheck(void)
{
    //显然没有泄露
    if (g_global.mallocs == g_global.frees)
    {
        return;
    }
    else
    {
        if (g_globalThead.doLeakCount > 990)
        {
            hw_printf(" look leak has occur: malloc count is %d, free count is %d\n\n", g_global.mallocs, g_global.frees);
            hw_printf(" you can call DT_Enable_Leak_Check(0,0) to close it\n\n");
        }
        //fflush(stdout);
    }
    
    if (g_global.enableDoLeakCheck == 0)
    {
        return;
    }
    if (g_globalThead.doLeakCount == 0)
    {
        return;
    }
    g_globalThead.doLeakCount--;
    
    int i = 0;
    
    g_globalThead.isDoingLeak = 1;
    if (__lsan_do_recoverable_leak_check)
    {
        i = __lsan_do_recoverable_leak_check();
    }

    g_globalThead.isDoingLeak = 0;
    
    if (i)
    {   
        if (__lsan_disable)
        {
            __lsan_disable();
        }
        // i= 0/0;
        HwExit(EXIT_CODE_LEAK);
    }
        
    if (__lsan_disable)
    {
        __lsan_disable();
    }

    if (__lsan_enable)
    {
        __lsan_enable();
    }
}

void MemoryRssLimitCallback(void) 
{
    if (__sanitizer_print_memory_profile)
    {
        __sanitizer_print_memory_profile(95, 8);
    }
    int rss = HWGetPeakRssMb();

    hw_printf("\r\n  ********The program exit for out-of-memory (used: %dMb; limit: %dMb)!!!********  \r\n ", rss, g_global.rssLimitMb);
    hw_printf("   To change the out-of-memory limit please call DT_SetStopOutOfMemory next DT_SetCheckOutOfMemory\n\n");

    CorpusShowCur();
    LlvmHookPrintStackTrace();
    g_global.isCrash = 1;
    HwExit(EXIT_CODE_RssLimit);
}

//out-of-memory
void MemoryRssThread(void) 
{
    while (g_global.startCheckOutOfMemory) 
    {
        HWSleep(1);
        int rss = HWGetPeakRssMb();
        if (rss > g_global.rssLimitMb)
        {
           MemoryRssLimitCallback();
        }
    }
    return;
}

void MemoryStart(int mallocLimitMb, int rssLimitMb) 
{
    g_global.mallocLimitMb = mallocLimitMb;
    g_global.rssLimitMb = rssLimitMb;
    
    if (g_global.startCheckOutOfMemory == 1)
    {
        //已经开始,不重复调用
        return;
    }
    g_global.startCheckOutOfMemory = 1;

    HwPthread_create(MemoryRssThread);
    MemoryAddCallBack();
}

void MemoryStop(void) 
{
    g_global.startCheckOutOfMemory = 0;
    g_global.mallocs = 0;
    g_global.frees = 0;
}

#else

void MemoryStart(int mallocLimitMb, int rssLimitMb) 
{

}

void MemoryStop(void) 
{

}

void LlvmEnableLeakCheck(int count)
{
}

void LlvmDoLeakCheck(void)
{
}

#endif

#ifdef __cplusplus
}
#endif
