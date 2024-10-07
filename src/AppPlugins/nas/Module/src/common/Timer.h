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
#ifndef MODULE_TIME_H
#define MODULE_TIME_H

#include <chrono>
#include <thread>
#include "common/Thread.h"

namespace Module {

struct TimePoint {
    TimePoint();
    std::chrono::time_point<std::chrono::steady_clock> value;
};

int64_t operator-(const TimePoint& lhs, const TimePoint& rhs);
bool operator>(const TimePoint& lhs, const TimePoint& rhs);
bool operator<(const TimePoint& lhs, const TimePoint& rhs);

const uint32_t TO_SECONDS = 1000;

class Timer {
    TimePoint m_t0;
public:
    Timer();
    ~Timer() {};
    inline int64_t Duration()
    {
        return DurationAsMicroSeconds() / TO_SECONDS;
    }

    inline int64_t DurationAsMicroSeconds()
    {
        return TimePoint() - m_t0;
    }

    inline void Reset()
    {
        m_t0 = TimePoint();
    }
};


class FixedTimer {
    TimePoint m_t0;
    TimePoint m_t1;
    bool m_measure;
public:
    virtual ~FixedTimer() = default;
    FixedTimer();
    
    inline int64_t Duration()
    {
        if (m_measure) {
            m_measure = false;
            m_t1 = TimePoint();
        }
        return m_t1 - m_t0;
    }
};

class RetryTimer {
public:
    RetryTimer();
    ~RetryTimer() = default;
    void SetTime(int time, int interval);
    template<class R, class T>
    bool DoActionWithRetry(R f, T testfunc)
    {
        auto beginTime = std::chrono::steady_clock::now();
        bool exit = false;
        while (!exit) {
            if (testfunc(f())) {
                return true;
            }
            auto curTime = std::chrono::steady_clock::now();
            int diff = std::chrono::duration_cast<std::chrono::milliseconds>(curTime - beginTime).count();
            if (diff >= retryTime) {
                exit = true;
                continue;
            }
            Module::SleepFor(std::chrono::milliseconds(retryInterval));
        }
        return false;
    }

protected:
    int retryTime = 0;
    int retryInterval = 0;
};

} // namespace Module

#endif // MODULE_TIME_H
