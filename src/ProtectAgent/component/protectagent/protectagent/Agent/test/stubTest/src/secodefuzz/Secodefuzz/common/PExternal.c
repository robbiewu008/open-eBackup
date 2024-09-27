/*
��Ȩ���� (c) ��Ϊ�������޹�˾ 2012-2018

����:
wanghao 			w00296180
wangchengyun 	wwx412654

���ж��������Ľӿ�
��Ϊ����ṩ��lib���ܻᱻ�ü�ϵͳ����Сϵͳʹ�ã�����Ҫ���������ٵ�����
��������ж����������װ������ֲ����

�������������ļ���ֱ�ӵ����ⲿ��������ֱ�ӽ����������Ŀ���ֲ��
*/
#include "PCommon.h"

#ifdef __cplusplus
extern "C" {
#endif

// ��ϵͳ��ע�͵�LIB_DEFINE�꣬
// ���ڱ��ļ�����ֲ�±ߺ���

// malloc
// free
// memset
// memcpy
// memmove
// memcmp
// strtol
// strcmp

// �±ߺ������Բ�ʵ��
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

// �ں�̬�Ż����
// strtol
// kmalloc
// kfree

#undef  RAND_MAX
#define RAND_MAX 0x7fffffff

// ������1970�������
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

// �ں�̬�ݲ�֧��
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
#define Malloc_Max_Size 10000 // Ҫ��8��������֤�ֽڶ���
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

// size����ΪС��0,���ֵ���ܴ���MALLOC_MAX=0xFFFF
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


// ʵ��rand()�������������ҵĺ���ԭ�ͣ�����Բ�û��glibc��ߵ�ǿ
// ò��û�м��������ܶ��̻߳������⣬˭֪����
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
// ʵ��srand����
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

