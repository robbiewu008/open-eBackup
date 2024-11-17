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
#include "dataprocess/vmwarenative/AgentTimer.h"

namespace AGENT_VMWARENATIVE_TIMER {
TimePoint::TimePoint()
{
    value = std::chrono::high_resolution_clock::now();
}

mp_int64 operator-(const TimePoint& lhs, const TimePoint& rhs)
{
    std::chrono::milliseconds diff = std::chrono::duration_cast<std::chrono::milliseconds>(lhs.value - rhs.value);
    return diff.count();
}

mp_bool operator>(const TimePoint& lhs, const TimePoint& rhs)
{
    return lhs.value > rhs.value;
}

mp_bool operator<(const TimePoint& lhs, const TimePoint& rhs)
{
    return lhs.value < rhs.value;
}

Timer::Timer() : m_t0()
{}

FixedTimer::FixedTimer() : m_t0(), m_t1(), m_measure(true)
{}
}  // namespace AGENT_VMWARENATIVE_TIMER