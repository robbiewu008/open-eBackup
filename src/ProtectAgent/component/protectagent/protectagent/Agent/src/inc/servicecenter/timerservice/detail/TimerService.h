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
#ifndef TIMER_SERVICE_H_
#define TIMER_SERVICE_H_
#include <memory>
#include <vector>
#include <mutex>
#include <thread>
#include <chrono>
#include "servicecenter/timerservice/include/ITimerService.h"
#include "servicecenter/timerservice/detail/Timer.h"

namespace timerservice {
namespace detail {
class TimerService : public ITimerService {
public:
    virtual ~TimerService()
    {
        Uninitailize();
    }
    virtual bool Initailize();
    virtual bool Uninitailize();
    virtual std::shared_ptr<ITimer> CreateTimer();
    virtual void DeleteTimer(std::shared_ptr<ITimer> timer);
private:
    void MonitorSystemTime();
    bool IsSystemTimeChange(std::chrono::time_point<std::chrono::system_clock> start,
        std::chrono::time_point<std::chrono::system_clock> end);
    void NotifyTimers();
private:
    bool m_stop{false};
    std::mutex m_lock;
    std::shared_ptr<std::thread> m_systimeWatch;
    std::vector<std::shared_ptr<Timer>> m_timers;
};
}
}
#endif