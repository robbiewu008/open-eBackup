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
#ifndef _AGENT_VMWARENATIVE_TASK_
#define _AGENT_VMWARENATIVE_TASK_

#include "apps/vmwarenative/VMwareNativeTask.h"
#include "gtest/gtest.h"
#include "stub.h"

class VMwareNativeTaskTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();

private:
    Stub stub;
};

void VMwareNativeTaskTest::SetUp()
{}

void VMwareNativeTaskTest::TearDown()
{}

void VMwareNativeTaskTest::SetUpTestCase()
{}

void VMwareNativeTaskTest::TearDownTestCase()
{}

#endif
