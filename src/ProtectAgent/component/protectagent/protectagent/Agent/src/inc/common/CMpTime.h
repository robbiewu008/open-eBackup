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
#ifndef AGENT_TIME_H
#define AGENT_TIME_H

#include "common/Types.h"
#include "common/Defines.h"

const mp_int32 SECOND_AND_MICROSECOND_TIMES = 1000000; // (1000 * 1000);
const mp_int32 SECOND_AND_MILLISECOND_TIMES = 1000;
const mp_int32 DATE_YEAR_1900 = 1900;
const mp_int32 DATE_MONTH_1 = 1;
static const mp_uchar NOW_TIME_LENGTH = 200;

#ifndef CLOCK_MONOTONIC
#define CLOCK_ID CLOCK_REALTIME
#else
#define CLOCK_ID CLOCK_MONOTONIC
#endif

class AGENT_API CMpTime {
public:
    static mp_void Now(mp_time& pTime);
    static mp_tm* LocalTimeR(mp_time& pTime, mp_tm& pTm);  // 线程安全
    static mp_void GetTimeOfDay(timeval& tp);
    static mp_uint64 GetTimeUsec();
    static mp_uint64 GetTimeSec();
    static mp_string GetTimeString(mp_time& pTime);
    static mp_double GenSeconds();
    static mp_void DoSleep(mp_uint32 ms);
    static mp_double Difftime(mp_time end, mp_time begin);
    
};

#endif  // __AGENT_TIME_H__
