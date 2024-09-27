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
#ifndef MODULE_CTIME_H
#define MODULE_CTIME_H

#include "define/Types.h"
#include "define/Defines.h"

namespace Module {

const int32_t SECOND_AND_MICROSECOND_TIMES = 1000000; // (1000 * 1000);
const int32_t SECOND_AND_MILLISECOND_TIMES = 1000;
const int32_t DATE_YEAR_1900 = 1900;
const int32_t DATE_MONTH_1 = 1;
static const uint8_t NOW_TIME_LENGTH = 200;

#ifndef CLOCK_MONOTONIC
#define CLOCK_ID CLOCK_REALTIME
#else
#define CLOCK_ID CLOCK_MONOTONIC
#endif

class AGENT_API CTime {
public:
    static void Now(time_t& pTime);
    static tm* LocalTimeR(time_t& pTime, tm& pTm);
    static void GetTimeOfDay(timeval& tp);
    static uint64_t GetTimeUsec();
    static uint64_t GetTimeSec();
    static std::string GetTimeString(time_t& pTime);
    static double GenSeconds();
    static void DoSleep(uint32_t ms);
    static double Difftime(time_t end, time_t begin);
    
};

} // namespace Module

#endif  // CTIME_H
