/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 * @file ScannerTime.h
 * @date 12/27/2022
 * @author c00661662
 * @brief
 */

#ifndef FS_SCANNER_TIME_UTILS_H
#define FS_SCANNER_TIME_UTILS_H

#ifdef WIN32
#include <chrono>
#else
#include <sys/time.h>
#endif

#include "securec.h"

namespace {
    constexpr int USEC_PER_SEC = 1000 * 1000;
}

class Time {
public:
    Time() : m_timeStamp(0)
    {}

    explicit Time(int64_t timeStamp) : m_timeStamp(timeStamp) {}

    std::string ToString() const
    {
        char buf[32] = {0};
        int64_t seconds = m_timeStamp / USEC_PER_SEC;
        int64_t microseconds = m_timeStamp % USEC_PER_SEC;
        snprintf_s(buf, sizeof(buf), sizeof(buf)-1, "%ld.%06ld", seconds, microseconds);
        return buf;
    }

    int64_t MicroSeconds() const
    {
        return m_timeStamp;
    }

    time_t Seconds() const
    {
        return static_cast<time_t>(m_timeStamp / USEC_PER_SEC);
    }

    static Time Now()
    {
#ifdef WIN32
        /* cross-platform microseconds timestamp api */
        using namespace std::chrono;
        uint64_t microSecs = duration_cast<microseconds>(system_clock::now().time_since_epoch()).count();
        return Time(microSecs);
#else
        struct timeval tv;
        gettimeofday(&tv, NULL);
        int64_t seconds = tv.tv_sec;
        return Time(seconds * USEC_PER_SEC + tv.tv_usec);
#endif
    }

    static Time UnixTime(time_t unixTimeStamp)
    {
        return Time(static_cast<int64_t>(unixTimeStamp) * USEC_PER_SEC);
    }

private:
    int64_t m_timeStamp;
};

inline bool operator<(Time lhs, Time rhs)
{
    return lhs.MicroSeconds() < rhs.MicroSeconds();
}

inline bool operator==(Time lhs, Time rhs)
{
    return lhs.MicroSeconds() == rhs.MicroSeconds();
}

inline double timeDifference(Time high, Time low)
{
    int64_t diff = high.MicroSeconds() - low.MicroSeconds();
    return static_cast<double>(diff) / USEC_PER_SEC;
}

inline Time addTime(Time time, double seconds)
{
    int64_t delta = static_cast<int64_t>(seconds * USEC_PER_SEC);
    return Time(time.MicroSeconds() + delta);
}

#endif
