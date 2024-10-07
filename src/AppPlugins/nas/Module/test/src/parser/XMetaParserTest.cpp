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
#include "XMetaParser.h"
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
    constexpr auto MODULE = "XMETA_TEST";
    const string META_NAME = "/opt/opmetabin";
}

static uint64_t m_offset {0};
class XMetaParserTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    string m_metaFile = META_NAME;
};

void XMetaParserTest::SetUp()
{}

void XMetaParserTest::TearDown()
{}

void XMetaParserTest::SetUpTestCase()
{}

void XMetaParserTest::TearDownTestCase()
{}

XMetaParser::Params ConstructXMetaParms(string metafile)
{
    XMetaParser::Params params {};

    params.m_fileName = metafile;
    params.maxEntriesPerFile = 10000;
    params.maxDataSize = 4096;
    params.taskId = "123";
    params.nasServer = "10.28.12.xxx";
    params.nasSharePath = "/share1";
    params.proto = "nfs";
    params.protoVersion = "3";
    params.backupType = "FullBackup";
    params.metaDataScope = "FilesAndFolders";
    return params;
}

vector<XMetaField> ConstructXMetaEntry(string nameStr)
{
    XMetaField name {};
    name.m_xMetaType = XMETA_TYPE::XMETA_TYPE_NAME;
    name.m_value = nameStr;
    return {name};
}

TEST_F(XMetaParserTest, WriteXMetaData)
{
    auto params = ConstructXMetaParms(m_metaFile);
    XMetaParser xMetaParser(params);
    CTRL_FILE_RETCODE ret = xMetaParser.Open(CTRL_FILE_OPEN_MODE::WRITE);
    EXPECT_EQ(ret, CTRL_FILE_RETCODE::SUCCESS);

    ret = xMetaParser.Open(CTRL_FILE_OPEN_MODE::WRITE);
    EXPECT_EQ(ret, CTRL_FILE_RETCODE::SUCCESS);

    auto entry1 = ConstructXMetaEntry("testfile1");
    m_offset = xMetaParser.GetCurrentOffset();
    cout << "OffSet 1: " << m_offset << endl;
    uint64_t writtenLen = xMetaParser.WriteXMeta(entry1);
    cout << "entry1 Written Length: " << writtenLen << endl;
    bool writeFlag = writtenLen > 0;
    EXPECT_EQ(writeFlag, true);

    string retFile = xMetaParser.GetFileName();
    EXPECT_EQ(retFile, m_metaFile);

    ret = xMetaParser.Close(CTRL_FILE_OPEN_MODE::WRITE);
    EXPECT_EQ(ret, CTRL_FILE_RETCODE::SUCCESS);

    ret = xMetaParser.Close(CTRL_FILE_OPEN_MODE::WRITE);
    EXPECT_EQ(ret, CTRL_FILE_RETCODE::SUCCESS);
}

TEST_F(XMetaParserTest, ReadXMetaData)
{
    XMetaParser xMetaParser(m_metaFile);
    CTRL_FILE_RETCODE ret = xMetaParser.Open(CTRL_FILE_OPEN_MODE::READ);
    EXPECT_EQ(ret, CTRL_FILE_RETCODE::SUCCESS);

    vector<XMetaField> entry1 {};
    cout << "OffSet 1: " << m_offset << endl;
    ret = xMetaParser.ReadXMeta(entry1, 253);
    EXPECT_EQ(ret, CTRL_FILE_RETCODE::SUCCESS);
    cout << "Entry size: " << entry1.size() << endl;
    EXPECT_EQ(entry1.size(), 1);
    cout << "Entry type: " << (int)entry1[0].m_xMetaType << endl;
    EXPECT_EQ(entry1[0].m_xMetaType, XMETA_TYPE::XMETA_TYPE_NAME);
    cout << "Entry value: " << entry1[0].m_value << endl;
    EXPECT_EQ(entry1[0].m_value, "testfile1");
    ret = xMetaParser.Close(CTRL_FILE_OPEN_MODE::WRITE);
    EXPECT_EQ(ret, CTRL_FILE_RETCODE::SUCCESS);
}
