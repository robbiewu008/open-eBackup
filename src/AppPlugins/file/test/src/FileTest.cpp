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
#include "Module/src/common/Path.h"
#include "config_reader/ConfigIniReader.h"
#include "File.h"
#ifdef FUZZ_ENABLED
#include "secodeFuzz.h"
#endif

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
    const string PLUGIN_LOG_FILE = "FileTestLLT.log";
    constexpr auto MODULE = "FileTest";
}

class CFileTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
};

void CFileTest::SetUp()
{}

void CFileTest::TearDown()
{}

void CFileTest::SetUpTestCase()
{}

void CFileTest::TearDownTestCase()
{}

void ConfigPlugin(int argc, char** argv)
{
    Module::CPath::GetInstance().Init(argv[0]);
    string LogPath = Module::CPath::GetInstance().GetLogPath();
    cout << "Test log path: " << LogPath << endl;
    Module::CLogger::GetInstance().Init(PLUGIN_LOG_FILE.c_str(), LogPath);
    Module::CFile::CreateDir(LogPath.c_str());
    Module::CLogger::GetInstance().SetLogConf(0, 100, 100);
    return;
}

int main(int argc, char** argv)
{
    ConfigPlugin(argc, argv);
    #ifdef FUZZ_ENABLED
    DT_Set_Report_Path((char *)"./log/"); //设置FUZZ报告输出路径
    DT_Enable_Leak_Check(0,0);
    #endif
    ::testing::GTEST_FLAG(output) = "xml:./report/FileUT_report.xml";
    ::testing::InitGoogleMock(&argc, argv);
    HCP_Log(INFO, MODULE) << "======================= Start File Service LLT ==========================" << HCPENDLOG;
    return RUN_ALL_TESTS();
}
