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
#ifndef INTERVAL_TIMER_H
#define INTERVAL_TIMER_H
#include <chrono>

class JobTimer {
public:
    JobTimer(int interval): m_interval(interval) {}
    void StopTimer();
    void StartTimer();
    bool IsOverInterval();

private:
    bool m_startFlag = false;
    int m_interval;
    std::chrono::steady_clock::time_point m_startTime;
};
#endif