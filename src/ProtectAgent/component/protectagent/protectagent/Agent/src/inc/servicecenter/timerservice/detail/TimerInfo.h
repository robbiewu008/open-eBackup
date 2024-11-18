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
#ifndef TIMER_INFO_H_
#define TIMER_INFO_H_
namespace timerservice {
namespace detail {
struct TimerInfo {
public:
    TimerInfo(
        std::chrono::time_point<std::chrono::steady_clock> timepoint, int32_t timeId, int32_t ms,
            const TimeoutExecuter& func)
        : m_timePoint(timepoint), m_timeId(timeId), m_ms(ms), m_func(func)
    {}
    std::chrono::time_point<std::chrono::steady_clock> m_timePoint;
    int32_t m_timeId {0};
    int32_t m_ms {0};
    TimeoutExecuter m_func;
};
}  // namespace detail
}  // namespace timerservice
#endif