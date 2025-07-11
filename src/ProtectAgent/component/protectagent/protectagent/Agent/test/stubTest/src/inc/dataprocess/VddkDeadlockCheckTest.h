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
#ifndef __AGENT_VDDK_DEADLOCK_CHECK_TEST_H__
#define __AGENT_VDDK_DEADLOCK_CHECK_TEST_H__

#define private public
#include "dataprocess/vmwarenative/VddkDeadlockCheck.h"
#include "common/ConfigXmlParse.h"
#include "common/Log.h"
#include "gtest/gtest.h"
#include "stub.h"

class VddkDeadlockCheckTest: public testing::Test{
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
public:
    Stub stub;
};

void VddkDeadlockCheckTest::SetUp() {}

void VddkDeadlockCheckTest::TearDown() {}

void VddkDeadlockCheckTest::SetUpTestCase() {}

void VddkDeadlockCheckTest::TearDownTestCase() {}

mp_int32 StubVddkDeadlockCheckGetValueInt32Return(mp_void* pthis, const mp_string& strSection, const mp_string& strKey, mp_int32& iValue)
{
    return MP_SUCCESS;
}

bool StubStartDeadlockThread()
{
    return true;
}

bool StubGenerateIDInner(const uint64_t requestID, const std::string& strApiName, uint64_t& tempID)
{
    return true;
}

bool StubIsRunning()
{
    return false;
}

bool StubStart()
{
    return true;
}

void StubKillProcess()
{
 std::cout << "Kill Process Succ!" << std::endl;
}

#endif
