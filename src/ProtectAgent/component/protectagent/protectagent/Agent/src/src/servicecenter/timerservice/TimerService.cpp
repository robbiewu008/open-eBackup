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
#include <algorithm>
#include <iostream>
#include "common/Log.h"
#include "servicecenter/servicefactory/include/ServiceFactory.h"
#include "servicecenter/timerservice/detail/TimerService.h"

using namespace std::chrono;
namespace timerservice {
namespace detail {
namespace {
    constexpr int32_t SYSTEM_TIME_CHECK_INTERVAL = 1000;
    constexpr int32_t SYSTEM_TIME_CHANGE_INTERVAL = 5;
}
bool TimerService::Initailize()
{
    std::unique_lock<std::mutex>  lock(m_lock);
    if (!m_systimeWatch) {
        m_systimeWatch = std::make_shared<std::thread>(
            [this]() {
                return MonitorSystemTime();
            }
        );
    }
    return true;
}

bool TimerService::Uninitailize()
{
    m_stop = true;
    if (m_systimeWatch) {
        m_systimeWatch->join();
        m_systimeWatch.reset();
    }
    return true;
}

std::shared_ptr<ITimer> TimerService::CreateTimer()
{
    auto timer = std::make_shared<Timer>();
    std::unique_lock<std::mutex> lock(m_lock);
    m_timers.push_back(timer);
    return timer;
}

void TimerService::DeleteTimer(std::shared_ptr<ITimer> timer)
{
    std::unique_lock<std::mutex> lock(m_lock);
    std::shared_ptr<Timer> timerToDel = std::dynamic_pointer_cast<Timer>(timer);
    m_timers.erase(std::remove(m_timers.begin(), m_timers.end(), timerToDel), m_timers.end());
    return;
}

void TimerService::MonitorSystemTime()
{
    DBGLOG("Start Monitor System Time thread.");
    auto start = std::chrono::system_clock::now();
    while (!m_stop) {
        SleepForMS(SYSTEM_TIME_CHECK_INTERVAL);
        auto end = std::chrono::system_clock::now();
        if (IsSystemTimeChange(start, end)) {
            NotifyTimers();
        }
        start = end;
    }
    DBGLOG("End Monitor System Time thread.");
}

void TimerService::NotifyTimers()
{
    WARNLOG("TimerService notify system time has changed or system occured freeze.");
    std::vector<std::shared_ptr<Timer>> tmp;
    {
        std::unique_lock<std::mutex>  lock(m_lock);
        tmp = m_timers;
    }
    for (const auto& timer : m_timers) {
        timer->SystemTimeChange();
    }
}

bool TimerService::IsSystemTimeChange(std::chrono::time_point<std::chrono::system_clock> start,
    std::chrono::time_point<std::chrono::system_clock> end)
{
    if (end > start) {
        if (time_point_cast<seconds>(end) - time_point_cast<seconds>(start) > seconds(SYSTEM_TIME_CHANGE_INTERVAL)) {
            return true;
        }
    } else if (start > end) {
        if (time_point_cast<seconds>(start) - time_point_cast<seconds>(end) > seconds(SYSTEM_TIME_CHANGE_INTERVAL)) {
            return true;
        }
    }
    return false;
}
}
}