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
#include "taskmanager/externaljob/JobTimer.h"

void JobTimer::StopTimer()
{
    m_startFlag = false;
}

void JobTimer::StartTimer()
{
    m_startFlag = true;
    m_startTime = std::chrono::steady_clock::now();
}

bool JobTimer::IsOverInterval()
{
    if (!m_startFlag) {
        StartTimer();
        return false;
    }
    std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(now - m_startTime);
    if (duration.count() > m_interval) {
        m_startTime = std::chrono::steady_clock::now();
        return true;
    }
    return false;
}