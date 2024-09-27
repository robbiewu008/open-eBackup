/*
��Ȩ���� (c) ��Ϊ�������޹�˾ 2012-2018

����:
wanghao 			w00296180

��ģ��ʵ��kcov ��������

ʵ�ּ�¼�ں˴��븲���ʵĹ���

�����������֧�֣���Ҫע�͵�HAS_KCOV����


�±���������Ϊ��ģ��ʵ�ֵĶ����ṩ�ĸ����ʷ���������
�������������Ҳ��ʵ�ָ����ʷ�������afl����ʵ���±������������滻��ģ��
KcovIsHasNewFeature
KcovStartFeature
KcovEndFeature

*/

#include "PCommon.h"
#ifdef __cplusplus
extern "C" {
#endif

#ifdef HAS_KCOV

#define KCOV_INIT_TRACE                     _IOR('c', 1, unsigned long)
#define KCOV_DISABLE                            _IO('c', 101)
#define COVER_SIZE                          (64<<10)
#define KCOV_ENABLE                         _IO('c', 100)

#define KCOV_TRACE_PC  0
#define KCOV_TRACE_CMP 1

static int* g_kcov8BitCounters = NULL;
static uintptr_t *g_kcovPcs = NULL;
static int g_kcovIsHasInit = 0;

static int g_isHasNewFeature = 0;
static int g_kcovWeight = 0;
static int g_isPrintPc = 0;
static int g_hasCovPcNum = 0;
static int g_fd;
static unsigned long *g_cover = NULL;
static char g_buffer1[MAX_PC_DESCR_SIZE];
static char g_buffer2[MAX_PC_DESCR_SIZE];

int g_isStart = 0;

static void InitMalloc(void)
{
    if (g_kcovIsHasInit == 0)
    {
        g_kcovIsHasInit = 1;
        g_kcov8BitCounters = (int *)HwMalloc(sizeof(int)*MAX_KERNEL_PC_NUM);
        g_kcovPcs = (uintptr_t *)HwMalloc(sizeof(uintptr_t)*MAX_KERNEL_PC_NUM);

        HwMemset(g_kcov8BitCounters, 0, sizeof(int)*MAX_KERNEL_PC_NUM);
        HwMemset(g_kcovPcs, 0, sizeof(uintptr_t)*MAX_KERNEL_PC_NUM);
    }
}

int KcovIsHasNewFeature()
{
    int n;
    int i;
    int ret;

    if (g_isStart == 0)
    {
        return 0;
    }

    g_isStart = 0;

    /* Read number of PCs collected. */
    n = __atomic_load_n(&g_cover[0], __ATOMIC_RELAXED);

    for (i = 0; i < n; i++)
    {
        int* pc = (int *)(g_cover[i + 1]);
        int idx = (size_t)pc % (MAX_KERNEL_PC_NUM);

        // ֻ�Ե�һ�θ���Ŀǰ��������
        if (g_kcovPcs[idx] == 0)
        {
            g_isHasNewFeature = 1;
            g_kcovWeight += NewPc_Weight;
            g_hasCovPcNum++;

            if (g_global.isPrintPc)
            {
                FILE* fp;
                g_buffer2[0] = 0;
                hw_sprintf(g_buffer2, "addr2line -f  -e  /usr/src/linux-4.9/vmlinux  0x%lx", ((size_t)pc - 0x12000000));
                fp=popen(g_buffer2, "r");
                fgets(g_buffer1, sizeof(g_buffer1), fp);
                hw_printf("NEW_PC-0x%lx(idx-%d;CovRate-%d):%s", (size_t)pc, idx, g_hasCovPcNum, g_buffer1);
                pclose(fp);
            }

            g_kcovPcs[idx] = pc;
        }

        g_kcov8BitCounters[idx]++;
    }
    ret = g_isHasNewFeature;
    g_isHasNewFeature = 0;

    if (ioctl(g_fd, KCOV_DISABLE, 0))
    {
        perror("ioctl"), HwExit(EXIT_CODE);
    }

    return ret;
}

int KcovGetWeight(void)
{
    return g_kcovWeight;
}

// KCOV_TRACE_PC = 0,
/* Collecting comparison operands mode. */
// KCOV_TRACE_CMP = 1,
// ����ģʽ�����Ժ󷢾�.gcc���ڻ���֧�֣�clang֧��

void KcovStartFeature()
{
    if (g_isStart == 1)
    {
        return;
    }
    g_isStart =1;

    g_isHasNewFeature = 0;
    g_kcovWeight = 0;
    /* Enable coverage collection on the current thread. */
    if (ioctl(g_fd, KCOV_ENABLE, KCOV_TRACE_PC))
    {
            perror("ioctl KCOV_ENABLE"), HwExit(EXIT_CODE);
    }
  
    /* Reset coverage from the tail of the ioctl() call. */
    __atomic_store_n(&g_cover[0], 0, __ATOMIC_RELAXED);
}

void KcovEndFeature()
{
}

// http://www.spinics.net/lists/linux-mm/msg134419.html
// KCOV_MODE_DISABLED = 0,
// KCOV_MODE_INIT = 1,
// KCOV_MODE_TRACE = 1,
// KCOV_MODE_TRACE_PC = 2,
// KCOV_MODE_TRACE_CMP = 3,
// ����ģʽ�����Ժ󷢾�

void InitKcov(void)
{
    InitMalloc();
    
    HwMemset(g_kcov8BitCounters, 0, sizeof(int)*MAX_KERNEL_PC_NUM);
    HwMemset(g_kcovPcs, 0, sizeof(uintptr_t)*MAX_KERNEL_PC_NUM);
    g_hasCovPcNum = 0;

    g_fd = open("/sys/kernel/debug/kcov", O_RDWR);
    if (g_fd == -1)
    {
        perror("open kcov"), HwExit(EXIT_CODE);
    }

    /* Setup trace mode and trace size. */
    if (ioctl(g_fd, KCOV_INIT_TRACE, COVER_SIZE))
    {
        perror("ioctl KCOV_INIT_TRACE"), HwExit(EXIT_CODE);
    }

    /* Mmap buffer shared between kernel- and user-space. */
    g_cover = (unsigned long*)mmap(NULL, COVER_SIZE * sizeof(unsigned long), 
        PROT_READ | PROT_WRITE, MAP_SHARED, g_fd, 0);
    if ((void*)g_cover == MAP_FAILED)
    {
        perror("MAP_FAILED"), HwExit(EXIT_CODE);
    }
}

#else

int KcovIsHasNewFeature(void)
{
    return 0;
}

int KcovGetWeight(void)
{
    return 0;
}

void KcovStartFeature(void)
{
}

void KcovEndFeature(void)
{
}

void InitKcov(void)
{
}

#endif

#ifdef __cplusplus
}
#endif