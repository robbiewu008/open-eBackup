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
#include <cstdio>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "llt_stub/stub.h"
#include "llt_stub/addr_pri.h"
#include "CheckPointParser.h"
#include "ParserStructs.h"

using ::testing::_;
using ::testing::AtLeast;
using testing::DoAll;
using testing::InitGoogleMock;
using ::testing::Return;
using ::testing::Sequence;
using ::testing::SetArgReferee;
using ::testing::Throw;
using namespace std;
using namespace Module;

namespace {
    const string CTRL_NAME = "/opt/chkpntctrl.txt";
}

class CheckPointParserTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();

    string m_ctrlFile = CTRL_NAME;
};

void CheckPointParserTest::SetUp()
{}

void CheckPointParserTest::TearDown()
{}

void CheckPointParserTest::SetUpTestCase()
{}

void CheckPointParserTest::TearDownTestCase()
{}

CheckPointParser::Params ConstructCheckPointCtrlParams(string ctrlFile)
{
    CheckPointParser::Params params;

    params.maxEntriesPerFile = 4;
    params.maxFileSize = (10 * 1024 * 1024);  // 10 MB
    params.chkPntFileName = ctrlFile;
    params.backupType = "FULL";
    params.metaDataScope = "folder-and-files";
    params.taskId = "123";
    params.nasServer = "1.0.0.0";
    params.nasSharePath = "/share1";
    params.proto = "nfs";
    params.protoVersion = "3";
    return params;
}

TEST_F(CheckPointParserTest, WriteChkPntEntries)
{
    CheckPointParser::Params params = ConstructCheckPointCtrlParams(m_ctrlFile);
    CheckPointParser checkPointParser(params);
    CTRL_FILE_RETCODE ret = checkPointParser.Open(CTRL_FILE_OPEN_MODE::WRITE);
    EXPECT_EQ(ret, CTRL_FILE_RETCODE::SUCCESS);

    ret = checkPointParser.Open(CTRL_FILE_OPEN_MODE::WRITE);
    EXPECT_EQ(ret, CTRL_FILE_RETCODE::SUCCESS);

    string chkPntEntry1 = "/OceanstorFS/dir1,0";
    ret = checkPointParser.WriteChkPntEntry(chkPntEntry1);
    EXPECT_EQ(ret, CTRL_FILE_RETCODE::SUCCESS);

    string chkPntEntry2 = "/OceanstorFS/dir2,0";
    ret = checkPointParser.WriteChkPntEntry(chkPntEntry2);
    EXPECT_EQ(ret, CTRL_FILE_RETCODE::SUCCESS);

    string chkPntEntry3 = "/OceanstorFS/dir1/dir2,14";
    ret = checkPointParser.WriteChkPntEntry(chkPntEntry3);
    EXPECT_EQ(ret, CTRL_FILE_RETCODE::SUCCESS);

    string chkPntEntry4 = "/OceanstorFS/dir2/dir1,14";
    ret = checkPointParser.WriteChkPntEntry(chkPntEntry4);
    EXPECT_EQ(ret, CTRL_FILE_RETCODE::LIMIT_REACHED);
    
    ret = checkPointParser.Close(CTRL_FILE_OPEN_MODE::WRITE);
    EXPECT_EQ(ret, CTRL_FILE_RETCODE::SUCCESS);

    ret = checkPointParser.WriteChkPntEntry(chkPntEntry4);
    EXPECT_EQ(ret, CTRL_FILE_RETCODE::FAILED);
}

TEST_F(CheckPointParserTest, GetHeader)
{
    CheckPointParser::Header header;
    CheckPointParser checkPointParser(m_ctrlFile);
    CTRL_FILE_RETCODE ret = checkPointParser.GetHeader(header);
    EXPECT_EQ(ret, CTRL_FILE_RETCODE::FAILED);

    ret = checkPointParser.Open(CTRL_FILE_OPEN_MODE::READ);
    EXPECT_EQ(ret, CTRL_FILE_RETCODE::SUCCESS);

    ret = checkPointParser.Open(CTRL_FILE_OPEN_MODE::READ);
    EXPECT_EQ(ret, CTRL_FILE_RETCODE::SUCCESS);

    ret = checkPointParser.GetHeader(header);
    EXPECT_EQ(ret, CTRL_FILE_RETCODE::SUCCESS);

    EXPECT_EQ(header.title, CHECKPOINT_HEADER_TITLE);
    EXPECT_EQ(header.version, CHECKPOINT_HEADER_VERSION);
}

TEST_F(CheckPointParserTest, ReadAllEntries)
{
    CheckPointParser checkPointParser(m_ctrlFile);
    vector<string> chkPntEntriesList = checkPointParser.ReadAllChkPntEntries();
    EXPECT_TRUE(chkPntEntriesList.empty());
    
    CTRL_FILE_RETCODE ret = checkPointParser.Open(CTRL_FILE_OPEN_MODE::READ);
    EXPECT_EQ(ret, CTRL_FILE_RETCODE::SUCCESS);

    chkPntEntriesList = checkPointParser.ReadAllChkPntEntries();
    EXPECT_FALSE(chkPntEntriesList.empty());

    ret = checkPointParser.Close(CTRL_FILE_OPEN_MODE::READ);
    EXPECT_EQ(ret, CTRL_FILE_RETCODE::SUCCESS);
}

TEST_F(CheckPointParserTest, ReadMultipleChkPntEntries)
{
    int maxReadEntries = 3;
    CheckPointParser checkPointParser(m_ctrlFile);
    vector<string> chkPntEntriesList = checkPointParser.ReadMultipleChkPntEntries(maxReadEntries);
    EXPECT_TRUE(chkPntEntriesList.empty());
    
    CTRL_FILE_RETCODE ret = checkPointParser.Open(CTRL_FILE_OPEN_MODE::READ);
    EXPECT_EQ(ret, CTRL_FILE_RETCODE::SUCCESS);

    chkPntEntriesList = checkPointParser.ReadMultipleChkPntEntries(maxReadEntries);
    EXPECT_EQ(chkPntEntriesList.size(), maxReadEntries);

    ret = checkPointParser.Close(CTRL_FILE_OPEN_MODE::READ);
    EXPECT_EQ(ret, CTRL_FILE_RETCODE::SUCCESS);

    try {
        boost::filesystem::remove(m_ctrlFile);
    } catch (const boost::filesystem::filesystem_error &e) {
        cout << "remove() exeption: " << e.code().message() << endl;
    }
}
