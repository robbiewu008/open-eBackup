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
#include <list>
#include <iostream>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "log/Log.h"
#include "common/Path.h"
#include "config_reader/ConfigIniReader.h"

using ::testing::_;
using ::testing::AtLeast;
using testing::DoAll;
using testing::InitGoogleMock;
using ::testing::Return;
using ::testing::Sequence;
using ::testing::SetArgReferee;
using ::testing::Throw;
using namespace std;

namespace {
    const string PLUGIN_LOG_FILE = "ModuleTestLLT.log";
    constexpr auto MODULE = "ModuleTest";
}

class CModuleTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
};

void CModuleTest::SetUp()
{}

void CModuleTest::TearDown()
{}

void CModuleTest::SetUpTestCase()
{}

void CModuleTest::TearDownTestCase()
{}


TEST_F(CModuleTest, Test1)
{
    EXPECT_EQ(true, true);
}

void ConfigPlugin(int argc, char** argv)
{
    Module::CPath::GetInstance().Init(argv[0]);
    cout << "Test log path: " << Module::CPath::GetInstance().GetLogPath() << endl;
    Module::CLogger::GetInstance().Init(
        PLUGIN_LOG_FILE.c_str(), Module::CPath::GetInstance().GetLogPath());
    Module::CLogger::GetInstance().SetLogLevel(Module::OS_LOG_DEBUG);
    return;
}

int main(int argc, char** argv)
{
    ConfigPlugin(argc, argv);
    ::testing::GTEST_FLAG(output) = "xml:./report/ModuleUT_report.xml";
    ::testing::InitGoogleMock(&argc, argv);
    HCP_Log(INFO, MODULE) << "======================= Start AppPlugin_NAS LLT ==========================" << HCPENDLOG;
    return RUN_ALL_TESTS();
}
