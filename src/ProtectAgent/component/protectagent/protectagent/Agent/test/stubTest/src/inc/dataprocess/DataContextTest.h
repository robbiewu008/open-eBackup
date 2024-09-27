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
#ifndef __DATA_CONTEXT_TEST_H__
#define __DATA_CONTEXT_TEST_H__

#include "dataprocess/datareadwrite/DataContext.h"
#include "gtest/gtest.h"
#include "stub.h"

class DataContextTest: public testing::Test{
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();

private:
    Stub stub;
};

void DataContextTest::SetUp() {}

void DataContextTest::TearDown() {}

void DataContextTest::SetUpTestCase() {}

void DataContextTest::TearDownTestCase() {}

#endif