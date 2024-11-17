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
#ifndef AGENT_VMWARENATIVE_TIMER_H
#define AGENT_VMWARENATIVE_TIMER_H

#include <chrono>
#include "common/Types.h"

namespace AGENT_VMWARENATIVE_TIMER {
struct TimePoint {
    TimePoint();
    std::chrono::high_resolution_clock::time_point value;
};

mp_int64 operator-(const TimePoint& lhs, const TimePoint& rhs);
mp_bool operator>(const TimePoint& lhs, const TimePoint& rhs);
mp_bool operator<(const TimePoint& lhs, const TimePoint& rhs);

class Timer {
    TimePoint m_t0;

public:
    Timer();
    virtual ~Timer()
    {}
    inline mp_int64 Duration()
    {
        return TimePoint() - m_t0;
    }
};

class FixedTimer {
    TimePoint m_t0;
    TimePoint m_t1;
    bool m_measure;

public:
    FixedTimer();
    virtual ~FixedTimer()
    {}
    inline mp_int64 Duration()
    {
        if (m_measure) {
            m_measure = false;
            m_t1 = TimePoint();
        }
        return m_t1 - m_t0;
    }
};
}  // namespace AGENT_VMWARENATIVE_TIMER
#endif
