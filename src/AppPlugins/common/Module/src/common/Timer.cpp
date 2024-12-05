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
#include "common/Timer.h"
//#include "config_reader/ConfigIniReader.h"
namespace {
constexpr int NUM_SECTOMILL = 1000;
}

using namespace std::chrono;

namespace Module {

TimePoint::TimePoint()
{
    value = steady_clock::now();
}

int64_t operator-(const TimePoint& lhs, const TimePoint& rhs)
{
    /*lint -e1084*/ /*lint -e1723*/ // this is how boost works, and boost is more stable than pclint...
    microseconds diff = duration_cast<microseconds>(lhs.value - rhs.value);
    int64_t diffus = static_cast<int64_t>(diff.count());
    return diffus;
    /*lint +e1723*/ /*lint +e1084*/
}

bool operator>(const TimePoint& lhs, const TimePoint& rhs)
{
    return lhs.value > rhs.value;
}

bool operator<(const TimePoint& lhs, const TimePoint& rhs)
{
    return lhs.value < rhs.value;
}


Timer::Timer() : m_t0()
{ }

FixedTimer::FixedTimer() : m_t0(), m_t1(), m_measure(true)
{ }

RetryTimer::RetryTimer()
{}

void RetryTimer::SetTime(int time, int interval)
{
    retryTime = time;
    retryInterval = interval;
}
}

