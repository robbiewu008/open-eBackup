/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file Time.cpp
 * @brief  Contains function declarations Time
 * @version 1.0.0
 * @date 2020-08-01
 * @author wangguitao 00510599
 */
#ifndef WIN32
#include <sys/time.h>
#endif
#include "common/CMpTime.h"
#include "common/Log.h"
/* ------------------------------------------------------------
Description  : 获取当前时间
Output       : pTime -- 当前时间
------------------------------------------------------------- */
mp_void CMpTime::Now(mp_time& pTime)
{
    time(&pTime);
}
/* ------------------------------------------------------------
Description  :将时间秒数转化为本地时间
------------------------------------------------------------- */
mp_tm* CMpTime::LocalTimeR(mp_time& pTime, mp_tm& pTm)
{
#ifdef WIN32
    localtime_s(&pTm, &pTime);
#else
    localtime_r(&pTime, &pTm);
#endif

    return &pTm;
}
/* ------------------------------------------------------------
Description  :精确 获取当前时间
------------------------------------------------------------- */
mp_void CMpTime::GetTimeOfDay(timeval& tp)
{
#ifdef WIN32
    LARGE_INTEGER liQPF;  // CPU Frequency
    LARGE_INTEGER lPerformanceCount;

    QueryPerformanceFrequency(&liQPF);  // get cpu Frequency
    // retrieves the current value of the high-resolution performance counter
    QueryPerformanceCounter(&lPerformanceCount);

    // calc time (sec + usec)
    tp.tv_sec = (mp_uint32)(lPerformanceCount.QuadPart / liQPF.QuadPart);
    tp.tv_usec = (mp_uint32)(
        ((lPerformanceCount.QuadPart - liQPF.QuadPart * tp.tv_sec) * SECOND_AND_MICROSECOND_TIMES) / liQPF.QuadPart);

    return;
#else
    (mp_void) gettimeofday(&tp, NULL);
    return;
#endif
}
/* ------------------------------------------------------------
Description  : 获取当前时间(us级)
------------------------------------------------------------- */
mp_uint64 CMpTime::GetTimeUsec()
{
    struct timeval now;
    mp_uint64 now_usec;
    GetTimeOfDay(now);

    now_usec = (mp_uint64)now.tv_sec * SECOND_AND_MICROSECOND_TIMES + (mp_uint64)now.tv_usec;

    return now_usec;
}

/* ------------------------------------------------------------
Function Name: GetTimeSec
Description  : 获取当前时间（从系统启动开始计时），不受用户更改系统时间的影响，单位是秒
Others       :------------------------------------------------------------- */
mp_uint64 CMpTime::GetTimeSec()
{
#ifdef WIN32
    LARGE_INTEGER proc_freq;
    LARGE_INTEGER proc_counter;
    if (!QueryPerformanceFrequency(&proc_freq)) {
        return 0;
    }
    if (!QueryPerformanceCounter(&proc_counter)) {
        return 0;
    }

    return long(proc_counter.QuadPart / proc_freq.QuadPart);
#else
    struct timespec nowspec;
    clock_gettime(CLOCK_ID, &nowspec);
    return mp_uint64(nowspec.tv_sec);
#endif
}
/* ------------------------------------------------------------
Description  : 获取当前时间并格式化输出
Output       : pTime -- 当前时间
------------------------------------------------------------- */
mp_string CMpTime::GetTimeString(mp_time& pTime)
{
    mp_tm stTime;
    mp_tm* stLocalTime = LocalTimeR(pTime, stTime);
    if (stLocalTime == NULL) {
        COMMLOG(OS_LOG_ERROR, "stLocalTime is NULL.");
        return "";
    }

    mp_char acTime[NOW_TIME_LENGTH] = {0};
    strftime(acTime, sizeof(acTime), "%Y-%m-%d %X", stLocalTime);
    mp_string strAcTime = acTime;
    return strAcTime;
}

/* ------------------------------------------------------------
Function Name: GenSeconds
Description  : 获取从今天0时开始的时间秒
Others       :-------------------------------------------------------- */
mp_double CMpTime::GenSeconds()
{
    mp_time now;
    struct tm* pDiffTime = NULL;

    time(&now);
    pDiffTime = localtime(&now);
    if (pDiffTime == NULL) {
        COMMLOG(OS_LOG_ERROR, "Get localtime failed.");
        return MP_FAILED;
    }

    struct tm beginTime = *pDiffTime;
    beginTime.tm_hour = 0;
    beginTime.tm_min = 0;
    beginTime.tm_sec = 0;
    beginTime.tm_mon = 0;
    beginTime.tm_mday = 1;

    return difftime(now, mktime(&beginTime));
}

/*------------------------------------------------------------
Description  :睡眠函数
Input        : ms -- 时间
Output       : 无
Return       : 无
Create By    : 无
Modification : 无
-------------------------------------------------------------*/
mp_void CMpTime::DoSleep(mp_uint32 ms)
{
#ifdef WIN32
    Sleep(ms);
#else
    struct timeval stTimeOut;
    const mp_int32 timeUnitChange = 1000;
    stTimeOut.tv_sec  = ms / timeUnitChange;
    stTimeOut.tv_usec = (ms % timeUnitChange) * timeUnitChange;
    (mp_void)select(0, NULL, NULL, NULL, &stTimeOut);
#endif
}

mp_double CMpTime::Difftime(mp_time end, mp_time begin)
{
    return difftime(end, begin);
}

