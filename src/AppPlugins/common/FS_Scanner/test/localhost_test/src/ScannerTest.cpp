/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 * Author: l00347293
 * Create: 2022-07-21.
 */

#include <list>
#include <cstdio>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "log/Log.h"
#include "common/Path.h"
#include "common/File.h"
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
    const string PLUGIN_LOG_FILE = "ScannerTestLLT.log";
    constexpr auto MODULE = "ScannerTest";
}

class ScannerTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
};

void ScannerTest::SetUp()
{}

void ScannerTest::TearDown()
{}

void ScannerTest::SetUpTestCase()
{}

void ScannerTest::TearDownTestCase()
{}


TEST_F(ScannerTest, Test1)
{
    EXPECT_EQ(true, true);
}

void ConfigPlugin(int argc, char** argv)
{
    Module::CPath::GetInstance().Init(argv[0]);
    string LogPath = Module::CPath::GetInstance().GetLogPath();
    cout << "Test log path: " << LogPath << endl;
    Module::CFile::CreateDir(LogPath.c_str());
    Module::CLogger::GetInstance().Init(PLUGIN_LOG_FILE.c_str(), LogPath);
    Module::CLogger::GetInstance().SetLogConf(0, 100, 100);
    return;
}

int main(int argc, char** argv)
{
    ConfigPlugin(argc, argv);
    ::testing::GTEST_FLAG(output) = "xml:./report/ScannerUT_report.xml";
    ::testing::InitGoogleMock(&argc, argv);
    HCP_Log(INFO, MODULE) << "======================= Start Scanner LLT ==========================" << HCPENDLOG;
    return RUN_ALL_TESTS();
}
