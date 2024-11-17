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
#ifndef TIMER_H_
#define TIMER_H_
#include <atomic>
#include <condition_variable>
#include <memory>
#include <vector>
#include <mutex>
#include <thread>
#include "servicecenter/timerservice/include/ITimer.h"
#include "servicecenter/timerservice/detail/TimerInfo.h"

namespace timerservice {
namespace detail {
class Timer : public ITimer {
public:
    virtual ~Timer();
    virtual bool Start();
    virtual bool Stop();
    virtual int32_t AddTimeoutExecutor(const TimeoutExecuter& func, int32_t ms);
    virtual bool RemoveTimeoutExecutor(int32_t timeId);
    void SystemTimeChange();

private:
    void TimeLoop();
    void WaitForever();
    bool StopThread();
    void HandleTimeout();
    void Sort();
    void WaitTimeout();

private:
    std::atomic<int32_t> m_timeId {0};
    bool m_stop {false};
    std::mutex m_lock;
    std::condition_variable m_cond;
    std::shared_ptr<std::thread> m_thread;
    std::vector<std::shared_ptr<TimerInfo>> m_infos;
};
}  // namespace detail
}  // namespace timerservice
#endif