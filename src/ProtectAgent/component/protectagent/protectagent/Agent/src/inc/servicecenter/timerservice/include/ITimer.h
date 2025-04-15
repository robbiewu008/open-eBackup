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
#ifndef ITIMER_H_
#define ITIMER_H_
#include <functional>

namespace timerservice {
using TimeoutExecuter = std::function<bool(void)>;
class ITimer {
public:
    virtual ~ITimer() = default;

    // start timer thread
    virtual bool Start() = 0;
    // stop timer thread
    virtual bool Stop() = 0;
    /**
     * @func: when time interval satisfied, func is called, when false return, func will not call again by ITimer
     * @ms: time out interval, millisecond
     * @ret: timerId, for RemoveTimeoutExecutor
     */
    virtual int32_t AddTimeoutExecutor(const TimeoutExecuter& func, int32_t ms) = 0;
    /**
     * @timeId: the id of func remove, return value of AddTimeoutExecutor
     */
    virtual bool RemoveTimeoutExecutor(int32_t timeId) = 0;
};
}  // namespace timerservice
#endif