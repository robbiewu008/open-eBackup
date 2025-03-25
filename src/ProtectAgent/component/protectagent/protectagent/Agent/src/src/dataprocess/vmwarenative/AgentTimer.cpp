/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file AgentTimer.cpp
 * @brief  Contains function declarations about AgentTimer
 * @version 1.0.0
 * @date 2020-08-01
 * @author wangguitao 00510599
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