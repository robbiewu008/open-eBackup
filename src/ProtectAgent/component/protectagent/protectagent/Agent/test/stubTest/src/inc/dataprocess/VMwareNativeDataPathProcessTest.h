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
#ifndef __AGENT_VMWARENATIVE_DATAPATHPROCESS_TEST_H__
#define __AGENT_VMWARENATIVE_DATAPATHPROCESS_TEST_H__

#define private public
#include "dataprocess/datapath/VMwareNativeDataPathProcess.h"
#include "common/ConfigXmlParse.h"
#include "gtest/gtest.h"
#include "stub.h"

class VMwareNativeDataPathProcessTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();

public:
    Stub stub;
};

void VMwareNativeDataPathProcessTest::SetUp()
{}

void VMwareNativeDataPathProcessTest::TearDown()
{}

void VMwareNativeDataPathProcessTest::SetUpTestCase()
{}

void VMwareNativeDataPathProcessTest::TearDownTestCase()
{}

#endif