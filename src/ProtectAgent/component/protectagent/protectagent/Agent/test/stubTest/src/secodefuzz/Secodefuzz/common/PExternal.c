/*
版权所有 (c) 华为技术有限公司 2012-2018

作者:
wanghao 			w00296180
wangchengyun 	wwx412654

所有对外依赖的接口
因为最后提供的lib可能会被裁剪系统，最小系统使用，所以要做到尽量少的依赖
这里把所有对外的依赖封装，供移植方便

不允许在其他文件里直接调用外部函数，会直接降低整体代码的可移植性
*/
#include "PCommon.h"

#ifdef __cplusplus
extern "C" {
#endif

// 无系统库注释掉LIB_DEFINE宏，
// 并在本文件里移植下边函数

// malloc
// free
// memset
// memcpy
// memmove
// memcmp
// strtol
// strcmp

// 下边函数可以不实现
// srand
// rand
// time
// setitimer
// localtime
// printf
// exit
// fork
// wait
// WEXITSTATUS

// 内核态才会调用
// strtol
// kmalloc
// kfree

#undef  RAND_MAX
#define RAND_MAX 0x7fffffff

// 返回离1970年的秒数
int HwGetTime(void)
{
#ifdef LIB_DEFINE
    time_t t;
    int j;
    j = time(&t);
    return j;
#else
    return 0;
#endif
}

char* HwGetDate(void)
{
#ifdef LIB_DEFINE
    time_t timep;
    struct tm *p; 
    
    time(&timep);  
    p = localtime(&timep);
    g_global.date[0] = 0;
    
    hw_sprintf(g_global.date, "%d_%02d%02d_%02d_%02d_%02d", 1900 + p->tm_year, 1 + p->tm_mon, p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec);
    return g_global.date;
#else
    hw_sprintf(g_global.date, "2000_0000_00_00_00");
    return g_global.date;
#endif
}
int HwTime()
{
#ifdef LIB_DEFINE
    return (int)time(NULL);
#else
    return 1;
#endif
}

unsigned int alarm(unsigned int seconds);

// 内核态暂不支持
void HwSetTimer(int seconds) 
{
#ifdef _WIN32

#else
#ifdef __KERNEL__
#else
    struct itimerval tick; 
    tick.it_value.tv_sec = seconds;
    tick.it_value.tv_usec = 0; 
    tick.it_interval.tv_sec = seconds;  
    tick.it_interval.tv_usec = 0; 

    if (setitimer(ITIMER_REAL, &tick, 0))
    {
        hw_printf("\t call setitimer error!!!!\n");
        HwExit(EXIT_CODE);
    }
#endif
#endif
}

#ifdef malloc_self

#define Malloc_Num 20
#define Malloc_Max_Size 10000 // 要被8整除，保证字节对齐
char g_mallocBuf[Malloc_Num][Malloc_Max_Size] = {{0}};
int g_isUse[Malloc_Num] = {0};

char* HwMallocSelf(size_t size)
{
    if (size > Malloc_Max_Size)
    {
        hw_printf("memory size is not enough\r\n");
    }

    char* ptr = 0;
    int i;
    for (i = 0; i < Malloc_Num; i++)
    {
        if (g_isUse[i] == 0)
        {
            ptr = g_mallocBuf[i];
            g_isUse[i] = 1;
            break;
        }
    }

    if (ptr == 0)
    {
        hw_printf("no memory\r\n");
    }

    return ptr;
}
// free
void HwFreeSelf(void* ptemp)
{
    int i;
    for (i = 0; i < Malloc_Num; i++)
    {
        if(g_mallocBuf[i] == ptemp)
        {
            g_isUse[i] = 0;
            break;
        }
    }

    if (i == 20)
    {
        hw_printf("free invalid pointer\r\n");
    }
}
#endif

// size不能为小于0,最大值不能大于MALLOC_MAX=0xFFFF
// malloc
char* HwMalloc(size_t size)
{
    char* ptr;
    g_global.isSelfMalloc = 1;

    if (size == 0)
    {
        ptr = NULL;
    }
    else
    {
#ifdef malloc_self
        g_global.isSelfMalloc = 0;
        return HwMallocSelf(size);
#endif

#ifndef __KERNEL__
        ptr = (char *)malloc(size);
#else
        ptr = (char *)kmalloc(size, GFP_KERNEL);
#endif
        if (ptr <= 0)
        {
            hw_printf("\r\n*********malloc failed*********\r\n");
            ASSERT(1);
        }
    }

    g_global.isSelfMalloc = 0;
    return ptr;
}
// free
void HwFree(void* ptemp)
{
    g_global.isSelfMalloc = 1;
#ifdef malloc_self
    return HwFreeSelf(ptemp);
#endif

#ifndef __KERNEL__
    free(ptemp);
#else
    kfree(ptemp);
#endif
    g_global.isSelfMalloc = 0;
}

// memset
void *HwMemset(void *s, int ch, size_t n)
{
    return memset(s, ch, n);
}

// memcpy
void *HwMemcpy(void *dest, const void *src, size_t n)
{
    return memcpy(dest, src, n);
}
// memmove
void *HwMemMove(void *dest, const void *src, size_t n)
{
    return memmove(dest, src, n);
}

int HwMemCmp(const void *buf1, const void *buf2, unsigned int count)
{
    return memcmp(buf1, buf2, count);
}

long int HwStrToL(const char *nptr, char **endptr, int base)
{
#ifndef __KERNEL__
    return (strtol(nptr, endptr, base));
#else
    return (simple_strtol(nptr, endptr, base));
#endif
}

int HwStrCmp(const char *s1, const char *s2)
{
    return (strcmp(s1, s2));
}

int HwRANDMAX()
{
    return RAND_MAX;
}
// rand


// 实现rand()函数，在网上找的函数原型，随机性并没有glibc里边的强
// 貌似没有加锁，可能多线程会有问题，谁知道呢
int HwRand()
{
#ifdef LIB_DEFINE
    return rand();
#else
    g_globalThead.next = g_globalThead.next * 1103515245 + 12345;     
    return (*(unsigned  int *) & g_globalThead.next) & 0x7FFFFFFF;  
#endif
}
// srand
// 实现srand函数
void HwSrand(unsigned  int temp)
{ 
#ifdef LIB_DEFINE
    srand(temp);
#else
    g_globalThead.next = temp; 
#endif
}

int HwFork(void)
{ 
    int pid = 1;
#ifdef LIB_DEFINE

#ifndef _WIN32
    pid = fork(); 
#endif

#else
    pid = 1;
#endif
    return pid;
}

int HwWait(void)
{ 
    int status = 0;
#ifdef LIB_DEFINE

#ifndef _WIN32
    wait(&status);
#endif

#else
    status = 0;
#endif
    return status;
}

void HwExit(int no)
{ 
#ifdef LIB_DEFINE

#ifndef _WIN32
    exit(no);
#endif

#endif
    return;
}

int HwWEXITSTATUS(int status)
{ 
    int errorNo = 0;
    
#ifdef LIB_DEFINE

#ifndef _WIN32
    errorNo = WEXITSTATUS(status);
#endif

#else
    errorNo = 0;
#endif

    return errorNo;
}

void HwPthread_create(void (*fun)(void))
{ 
#ifdef LIB_DEFINE

#ifndef _WIN32
    pthread_t thid1;
    if(pthread_create(&thid1, NULL, (void *)fun, NULL) != 0) 
    {
	printf("thread creation failed\n");
	HwExit(1);
    }
#endif

#endif
    return;
}

int HWGetPeakRssMb(void)
{ 

#ifdef LIB_DEFINE
    
#ifndef _WIN32

    struct rusage usage;
    if (getrusage(RUSAGE_SELF, &usage))
    {
        return 0;
    }

    return usage.ru_maxrss >> 10;

#endif

#endif
    return 0;

}

void HWSleep(int time)
{ 

#ifdef LIB_DEFINE
    
#ifndef _WIN32
    sleep(time);
#else
    Sleep(time);
#endif

#endif

}


#ifdef __cplusplus
}
#endif

