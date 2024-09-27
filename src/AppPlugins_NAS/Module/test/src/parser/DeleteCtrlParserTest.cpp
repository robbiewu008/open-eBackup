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
#include "DeleteCtrlParser.h"
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
    constexpr auto MODULE = "DELETE_CTRL_TEST";
    const string CTRL_NAME = "/opt/deletectrl.txt";
    uint64_t m_offset {0};
}

class DeleteCtrlParserTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    string m_ctrlFile = CTRL_NAME;
};

void DeleteCtrlParserTest::SetUp()
{}

void DeleteCtrlParserTest::TearDown()
{}

void DeleteCtrlParserTest::SetUpTestCase()
{}

void DeleteCtrlParserTest::TearDownTestCase()
{}

DeleteCtrlParser::Params ConstructDelCtrlParams(string ctrlfile)
{
    DeleteCtrlParser::Params params;

    params.m_ctlFileName = ctrlfile;
    params.maxEntriesPerFile = 3;
    params.taskId = "123";
    params.nasServer = "1.0.0.0";
    params.nasSharePath = "/share1";
    params.proto = "nfs";
    params.protoVersion = "3";
    return params;
}

TEST_F(DeleteCtrlParserTest, WriteDelCtrl)
{
    auto params = ConstructDelCtrlParams(m_ctrlFile);
    DeleteCtrlParser deleteCtrlParser(params);
    CTRL_FILE_RETCODE ret = deleteCtrlParser.Open(CTRL_FILE_OPEN_MODE::WRITE);
    EXPECT_EQ(ret, CTRL_FILE_RETCODE::SUCCESS);

    DeleteCtrlEntry deleteEntry1 {};
    deleteEntry1.m_absPath = "/dir1";
    deleteEntry1.m_isDel = true;
    deleteEntry1.m_isDir = true;
    ret = deleteCtrlParser.WriteDirEntry(deleteEntry1);
    EXPECT_EQ(ret, CTRL_FILE_RETCODE::SUCCESS);

    string fileName1 = "/dir1/file1";
    ret = deleteCtrlParser.WriteFileEntry(fileName1);
    EXPECT_EQ(ret, CTRL_FILE_RETCODE::SUCCESS);

    DeleteCtrlEntry deleteEntry2 {};
    deleteEntry2.m_absPath = "/dir1/dir2";
    deleteEntry2.m_isDel = false;
    deleteEntry2.m_isDir = true;
    ret = deleteCtrlParser.WriteDirEntry(deleteEntry2);
    EXPECT_EQ(ret, CTRL_FILE_RETCODE::LIMIT_REACHED);

    uint32_t numOfEntries = deleteCtrlParser.GetEntries();
    EXPECT_EQ(numOfEntries, params.maxEntriesPerFile);
    
    ret = deleteCtrlParser.Close(CTRL_FILE_OPEN_MODE::WRITE);
    EXPECT_EQ(ret, CTRL_FILE_RETCODE::SUCCESS);

    ret = deleteCtrlParser.WriteDirEntry(deleteEntry2);
    EXPECT_EQ(ret, CTRL_FILE_RETCODE::FAILED);

    ret = deleteCtrlParser.WriteFileEntry(fileName1);
    EXPECT_EQ(ret, CTRL_FILE_RETCODE::FAILED);
}

TEST_F(DeleteCtrlParserTest, GetHeader)
{
    DeleteCtrlParser::Header header;
    DeleteCtrlParser deleteCtrlParser(m_ctrlFile);
    CTRL_FILE_RETCODE ret = deleteCtrlParser.GetHeader(header);
    EXPECT_EQ(ret, CTRL_FILE_RETCODE::FAILED);

    ret = deleteCtrlParser.Open(CTRL_FILE_OPEN_MODE::READ);
    EXPECT_EQ(ret, CTRL_FILE_RETCODE::SUCCESS);

    ret = deleteCtrlParser.Open(CTRL_FILE_OPEN_MODE::READ);
    EXPECT_EQ(ret, CTRL_FILE_RETCODE::SUCCESS);

    ret = deleteCtrlParser.GetHeader(header);
    EXPECT_EQ(ret, CTRL_FILE_RETCODE::SUCCESS);

    EXPECT_EQ(header.title, DELETECTRL_HEADER_TITLE);
    EXPECT_EQ(header.version, DELETECTRL_HEADER_VERSION);

    string delCtrlFileName = deleteCtrlParser.GetCtrlFileName();
    EXPECT_EQ(delCtrlFileName, m_ctrlFile);
}

TEST_F(DeleteCtrlParserTest, ReadDelCtrl)
{
    DeleteCtrlEntry deleteEntry1 {};
    string  fileName1 {};

    DeleteCtrlParser deleteCtrlParser(m_ctrlFile);
    CTRL_FILE_RETCODE ret = deleteCtrlParser.ReadEntry(deleteEntry1, fileName1);
    EXPECT_EQ(ret, CTRL_FILE_RETCODE::FAILED);

    ret = deleteCtrlParser.Open(CTRL_FILE_OPEN_MODE::READ);
    EXPECT_EQ(ret, CTRL_FILE_RETCODE::SUCCESS);

    ret = deleteCtrlParser.ReadEntry(deleteEntry1, fileName1);
    EXPECT_EQ(ret, CTRL_FILE_RETCODE::SUCCESS);
    cout << "Directory Path: " << deleteEntry1.m_absPath << "\n" ;
    cout << "IsDel: " << deleteEntry1.m_isDel << "\n";
    cout << "IsDir: " << deleteEntry1.m_isDir << "\n";
    cout << "\n";

    DeleteCtrlEntry deleteEntry2 {};
    string  fileName2 {};
    ret = deleteCtrlParser.ReadEntry(deleteEntry2, fileName2);
    EXPECT_EQ(ret, CTRL_FILE_RETCODE::SUCCESS);
    cout << "fileName2: " << fileName2 << endl;

    DeleteCtrlEntry deleteEntry3 {};
    string  fileName3 {};
    ret = deleteCtrlParser.ReadEntry(deleteEntry3, fileName3);

    ret = deleteCtrlParser.ReadEntry(deleteEntry3, fileName3);
    EXPECT_EQ(ret, CTRL_FILE_RETCODE::READ_EOF);

    ret = deleteCtrlParser.Close(CTRL_FILE_OPEN_MODE::READ);
    EXPECT_EQ(ret, CTRL_FILE_RETCODE::SUCCESS);

    try {
        boost::filesystem::remove(m_ctrlFile);
    } catch (const boost::filesystem::filesystem_error &e) {
        cout << "remove() exeption: " << e.code().message() << endl;
    }
}