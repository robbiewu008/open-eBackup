/*
* This file is a part of the open-eBackup project.
* This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this file, You can obtain one at
* http://mozilla.org/MPL/2.0/.
*
* Copyright (c) [2024] Huawei Technologies Co.,Ltd.
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*/
#include "CTime.h"

#ifndef WIN32
#include <sys/time.h>
#endif
#include "log/Log.h"

namespace Module {

using namespace std;

void CTime::Now(time_t& pTime)
{
    time(&pTime);
}

tm* CTime::LocalTimeR(time_t& pTime, tm& pTm)
{
#ifdef WIN32
    localtime_s(&pTm, &pTime);
#else
    localtime_r(&pTime, &pTm);
#endif

    return &pTm;
}

void CTime::GetTimeOfDay(timeval& tp)
{
#ifdef WIN32
    LARGE_INTEGER liQPF;  // CPU Frequency
    LARGE_INTEGER lPerformanceCount;

    QueryPerformanceFrequency(&liQPF);  // get cpu Frequency
    // retrieves the current value of the high-resolution performance counter
    QueryPerformanceCounter(&lPerformanceCount);

    // calc time (sec + usec)
    tp.tv_sec = (unsigned int)(lPerformanceCount.QuadPart / liQPF.QuadPart);
    tp.tv_usec = (unsigned int)(
        ((lPerformanceCount.QuadPart - liQPF.QuadPart * tp.tv_sec) * SECOND_AND_MICROSECOND_TIMES) / liQPF.QuadPart);

    return;
#else
    (void) gettimeofday(&tp, NULL);
    return;
#endif
}

uint64_t CTime::GetTimeUsec()
{
    struct timeval now;
    uint64_t now_usec;
    GetTimeOfDay(now);

    now_usec = (uint64_t)now.tv_sec * SECOND_AND_MICROSECOND_TIMES + (uint64_t)now.tv_usec;

    return now_usec;
}

uint64_t CTime::GetTimeSec()
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
    return uint64_t(nowspec.tv_sec);
#endif
}

string CTime::GetTimeString(time_t& pTime)
{
    tm stTime;
    char acTime[NOW_TIME_LENGTH] = {0};
    tm* stLocalTime = LocalTimeR(pTime, stTime);
    if (stLocalTime == NULL) {
        COMMLOG(OS_LOG_ERROR, "stLocalTime is NULL.");
        return "";
    }

    /*lint -e1055 -e746*/
    strftime(acTime, sizeof(acTime), "%Y-%m-%d %X", stLocalTime);
    string strAcTime = acTime;
    return strAcTime;
}

double CTime::GenSeconds()
{
    time_t now;
    struct tm beginTime;
    struct tm* pDiffTime = NULL;

    time(&now);
    pDiffTime = localtime(&now);
    if (pDiffTime == NULL) {
        COMMLOG(OS_LOG_ERROR, "Get localtime failed.");
        return FAILED;
    }

    beginTime = *pDiffTime;
    beginTime.tm_hour = 0;
    beginTime.tm_min = 0;
    beginTime.tm_sec = 0;
    beginTime.tm_mon = 0;
    beginTime.tm_mday = 1;

    return difftime(now, mktime(&beginTime));
}

void CTime::DoSleep(uint32_t ms)
{
#ifdef WIN32
    Sleep(ms);
#else
    struct timeval stTimeOut;
    const int32_t timeUnitChange = 1000;
    stTimeOut.tv_sec  = ms / timeUnitChange;
    stTimeOut.tv_usec = (ms % timeUnitChange) * timeUnitChange;
    (void)select(0, NULL, NULL, NULL, &stTimeOut);
#endif
}

double CTime::Difftime(time_t end, time_t begin)
{
    return difftime(end, begin);
}

} // namespace Module
