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
#ifndef _JOB_STATE_ACTION_TEST_H_
#define _JOB_STATE_ACTION_TEST_H_

#define protected public
#include <vector>
#include <algorithm>
#include "stub.h"
#include "gtest/gtest.h"
#include "cunitpub/publicInc.h"

class JobStateActionTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    Stub stub;
};

void JobStateActionTest::SetUp() {}

void JobStateActionTest::TearDown() {}

void JobStateActionTest::SetUpTestCase() {}

void JobStateActionTest::TearDownTestCase() {}

#endif