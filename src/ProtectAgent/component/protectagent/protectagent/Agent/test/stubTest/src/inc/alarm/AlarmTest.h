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
#ifndef __AGENT_ALARM_TEST_H__
#define __AGENT_ALARM_TEST_H__

#define private public

#include "alarm/alarmdb.h"
#include "gtest/gtest.h"
#include "stub.h"

class CMpAlarmTest: public testing::Test{
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
public:
    Stub stub_cmp;
};

void CMpAlarmTest::SetUp() {}

void CMpAlarmTest::TearDown() {}

void CMpAlarmTest::SetUpTestCase() {}

void CMpAlarmTest::TearDownTestCase() {}

class CAlarmDBTest: public testing::Test{
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
public:
    Stub stub;
};

void CAlarmDBTest::SetUp() {}

void CAlarmDBTest::TearDown() {}

void CAlarmDBTest::SetUpTestCase() {}

void CAlarmDBTest::TearDownTestCase() {}

mp_int32 StubCConfigXmlParserGetValueInt32ReturnSuccess(mp_void* pthis, const mp_string& strSection, const mp_string& strKey, mp_int32& iValue)
{
    return MP_SUCCESS;
}

#endif
