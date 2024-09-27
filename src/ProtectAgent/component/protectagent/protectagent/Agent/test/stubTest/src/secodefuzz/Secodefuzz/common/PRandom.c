/*
版权所有 (c) 华为技术有限公司 2012-2018

作者:
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

        // 将时间作为种子，设置后随即出新的种子来
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

// 如果参数大于等于10时，平均变异1.6个元素
// 如果参数大于100，平均变异两个
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
    // 元素个数超过100，每次就平均变两个
    if (g_globalThead.wholeRandomNum >= 100)
    {
        return (HwRand() % g_globalThead.wholeRandomNum < 3) ? ENUM_YES : ENUM_NO;  
    }

    return (HwRand() % 100 < GetRandomRate()) ? ENUM_YES : ENUM_NO;   
}

// 仅用来做数量变异的时候使用，所以int足够
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