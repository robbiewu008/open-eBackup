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
#ifndef __AGENT_ALARM_TRAP_TEST_H__
#define __AGENT_ALARM_TRAP_TEST_H__

#define private public
#define protected public

#include "gtest/gtest.h"
#include "stub.h"

class CAlarmTrapTest: public testing::Test{
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
public:
    Stub m_stub;
};

void CAlarmTrapTest::SetUp() {}

void CAlarmTrapTest::TearDown() {}

void CAlarmTrapTest::SetUpTestCase() {}

void CAlarmTrapTest::TearDownTestCase() {}

#endif
