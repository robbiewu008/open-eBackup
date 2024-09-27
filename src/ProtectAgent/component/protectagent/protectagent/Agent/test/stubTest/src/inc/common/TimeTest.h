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
#ifndef _TIMETEST_H_
#define _TIMETEST_H_

#include "common/CMpTime.h"
#include "common/Log.h"
#include "common/ConfigXmlParse.h"
#include "gtest/gtest.h"
#include "stub.h"

typedef mp_void (CLogger::*CLoggerLogType)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...);
typedef mp_void (*StubCLoggerLogType)(mp_void* pthis);
mp_void StubCMpTimeLogVoid(mp_void* pthis);

class CMpTimeTest: public testing::Test{
public:
    Stub stub;
};

mp_tm* StubCMpTimeLocalTimeR(mp_time& pTime, mp_tm& pTm){
    return NULL;
}

mp_void StubCMpTimeLogVoid(mp_void* pthis){
    return;
}
#endif
