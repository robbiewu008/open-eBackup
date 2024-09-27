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
#ifndef __ORACLEINFO_TEST_H__
#define __ORACLEINFO_TEST_H__

#define private public
using namespace std;
#include "apps/oracle/OracleInfo.h"
#include "apps/oracle/OracleLunInfo.h"
#include "common/Log.h"
#include "common/ErrorCode.h"
#include "common/Utils.h"
#include "common/MpString.h"
// #include "common/RootCaller.h"
#include "common/CSystemExec.h"
#include "common/File.h"
#include "array/array.h"
#include <sstream>
#include "gtest/gtest.h"
#include "stub.h"
#include <vector>
using namespace std;

class OracleInfoTest : public testing::Test
{
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    Stub stub;
};


// Stub<CLoggerLogType, StubCLoggerLogType, mp_void>* COracleTest::m_stub;
void OracleInfoTest::SetUp() {}

void OracleInfoTest::TearDown() {}

void OracleInfoTest::SetUpTestCase() {}

void OracleInfoTest::TearDownTestCase() {}

#endif

