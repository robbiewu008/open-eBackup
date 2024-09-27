/*
��Ȩ���� (c) ��Ϊ�������޹�˾ 2012-2018

����:
wanghao 			w00296180
wangchengyun 	wwx412654


*/
#include "PCommon.h"

#ifdef __cplusplus
extern "C" {
#endif

void InitSeed(int initSeed, int startRange)
{
    if (initSeed == 0)
    {
        initSeed = HwTime();

        // ��ʱ����Ϊ���ӣ����ú��漴���µ�������
        HwSrand(initSeed); 
        initSeed = HwRand();
    }

    g_globalThead.seed = initSeed + startRange;
    HwSrand(g_globalThead.seed); 

    if (g_global.isLogOpen)
    {
        hw_printf("[*] g_global.seed = %d\r\n", g_globalThead.seed);
    }
}

// ����������ڵ���10ʱ��ƽ������1.6��Ԫ��
// �����������100��ƽ����������
int GetRandomRate(void)
{
    if (g_globalThead.wholeRandomNum == 1)
    {
        return 100;
    }

    if (g_globalThead.wholeRandomNum == 2)
    {
        return 110 / g_globalThead.wholeRandomNum;
    }

    if (g_globalThead.wholeRandomNum == 3)
    {
        return 120 / g_globalThead.wholeRandomNum;
    }

    if (g_globalThead.wholeRandomNum == 4)
    {
        return 130 / g_globalThead.wholeRandomNum;
    }

    if (g_globalThead.wholeRandomNum == 5) 
    {
        return 140 / g_globalThead.wholeRandomNum;
    }
    
    if(g_globalThead.wholeRandomNum >= 100)
    {
        return 2;
    }

    if (g_globalThead.wholeRandomNum >= 10)
    {
        return 160 / g_globalThead.wholeRandomNum;
    }

    if (g_globalThead.wholeRandomNum >= 6)
    {
        return 150 / g_globalThead.wholeRandomNum;
    }

    return 100;
}

int GetIsMutated()
{
    // Ԫ�ظ�������100��ÿ�ξ�ƽ��������
    if (g_globalThead.wholeRandomNum >= 100)
    {
        return (HwRand() % g_globalThead.wholeRandomNum < 3) ? ENUM_YES : ENUM_NO;  
    }

    return (HwRand() % 100 < GetRandomRate()) ? ENUM_YES : ENUM_NO;   
}

// �����������������ʱ��ʹ�ã�����int�㹻
u32 GaussRandU32(u32 pos)
{
    u32 value = RAND_RANGE(1, 1 << pos);

    return value;
}

s32 GaussRandS32(u32 pos)
{
    s32 value = RAND_RANGE(-1 * (1 << (pos)), 1 << (pos));

    return value;
}

u64 GaussRandU64(u32 pos)
{
    u64 value = RAND_RANGE64(1, (0ULL | 1) << pos);

    return value;
}

s64 GaussRandS64(u32 pos)
{
    s64 value = RAND_RANGE64((0ULL | -1) * ((0ULL | 1) << pos), (0ULL | 1) << pos);

    return value;
}

#ifdef __cplusplus
}
#endif