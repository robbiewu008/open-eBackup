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
#include "CopyCtrlParser.h"
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
    const string CTRL_NAME = "/opt/copyctrl.txt";
}

class CopyCtrlParserTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();

    string m_ctrlFile = CTRL_NAME;
};

void CopyCtrlParserTest::SetUp()
{}

void CopyCtrlParserTest::TearDown()
{}

void CopyCtrlParserTest::SetUpTestCase()
{}

void CopyCtrlParserTest::TearDownTestCase()
{}

CopyCtrlParser::Params ConstructCopyCtrlParams(string ctrlFile)
{
    CopyCtrlParser::Params params;

    params.maxEntriesPerFile = 4;
    params.minEntriesPerFile = 2;
    params.maxDataSize = (10 * 1024 * 1024);  // 10 MB
    params.minDataSize = (5 * 1024 * 1024);   // 5 MB
    params.m_ctrlFileTimeElapsed = 30;
    params.m_ctlFileName = ctrlFile;
    params.backupType = "FULL";
    params.metaDataScope = "folder-and-files";
    params.taskId = "123";
    params.nasServer = "1.0.0.0";
    params.nasSharePath = "/share1";
    params.proto = "nfs";
    params.protoVersion = "3";
    return params;
}

TEST_F(CopyCtrlParserTest, WriteCopyCtrl)
{
    CopyCtrlParser::Params params = ConstructCopyCtrlParams(m_ctrlFile);
    CopyCtrlParser copyCtrlParser(params);
    CTRL_FILE_RETCODE ret = copyCtrlParser.Open(CTRL_FILE_OPEN_MODE::WRITE);
    EXPECT_EQ(ret, CTRL_FILE_RETCODE::SUCCESS);

    CopyCtrlFileEntry fileEntry1;
    fileEntry1.m_mode = CTRL_ENTRY_MODE_META_MODIFIED;
    fileEntry1.m_fileName = "f,i,l,e,,,123";
    fileEntry1.m_metaFileName = "meta_file_1";
    fileEntry1.m_metaFileIndex = 1;
    fileEntry1.metaFileOffset = 123;
    fileEntry1.metaFileReadLen = 55;
    ret = copyCtrlParser.WriteFileEntry(fileEntry1);
    EXPECT_EQ(ret, CTRL_FILE_RETCODE::SUCCESS);

    CopyCtrlFileEntry fileEntry2;
    fileEntry2.m_mode = CTRL_ENTRY_MODE_DATA_DELETED;
    fileEntry2.m_fileName = ",,file2,,";
    ret = copyCtrlParser.WriteFileEntry(fileEntry2);
    EXPECT_EQ(ret, CTRL_FILE_RETCODE::SUCCESS);

    EXPECT_FALSE(copyCtrlParser.IsFileBufferEmpty());

    CopyCtrlDirEntry dirEntry1;
    dirEntry1.m_mode = CTRL_ENTRY_MODE_DATA_MODIFIED;
    dirEntry1.m_dirName = "/dir1";
    dirEntry1.m_metaFileIndex = 1;
    dirEntry1.metaFileOffset = 123;
    dirEntry1.metaFileReadLen = 55;
    ret = copyCtrlParser.WriteDirEntry(dirEntry1);
    EXPECT_EQ(ret, CTRL_FILE_RETCODE::SUCCESS);

    CopyCtrlDirEntry dirEntry2;
    dirEntry2.m_mode = CTRL_ENTRY_MODE_DATA_DELETED;
    dirEntry2.m_dirName = "/,di,r,,2,";
    dirEntry2.m_fileCount = 0;
    ret = copyCtrlParser.WriteDirEntry(dirEntry2);
    EXPECT_EQ(ret, CTRL_FILE_RETCODE::LIMIT_REACHED);

    EXPECT_TRUE(copyCtrlParser.IsFileBufferEmpty());

    uint32_t numOfEntries = copyCtrlParser.GetEntries();
    EXPECT_EQ(numOfEntries, params.maxEntriesPerFile);

    EXPECT_FALSE(copyCtrlParser.CheckCtrlFileTimeElapse());
    
    ret = copyCtrlParser.Close(CTRL_FILE_OPEN_MODE::WRITE);
    EXPECT_EQ(ret, CTRL_FILE_RETCODE::SUCCESS);

    ret = copyCtrlParser.WriteDirEntry(dirEntry2);
    EXPECT_EQ(ret, CTRL_FILE_RETCODE::FAILED);
}

TEST_F(CopyCtrlParserTest, GetHeader)
{
    CopyCtrlParser::Header header;
    CopyCtrlParser copyCtrlParser(m_ctrlFile);
    CTRL_FILE_RETCODE ret = copyCtrlParser.GetHeader(header);
    EXPECT_EQ(ret, CTRL_FILE_RETCODE::FAILED);

    ret = copyCtrlParser.Open(CTRL_FILE_OPEN_MODE::READ);
    EXPECT_EQ(ret, CTRL_FILE_RETCODE::SUCCESS);

    ret = copyCtrlParser.Open(CTRL_FILE_OPEN_MODE::READ);
    EXPECT_EQ(ret, CTRL_FILE_RETCODE::SUCCESS);

    ret = copyCtrlParser.GetHeader(header);
    EXPECT_EQ(ret, CTRL_FILE_RETCODE::SUCCESS);

    EXPECT_EQ(header.title, CTRL_HEADER_TITLE);
    EXPECT_EQ(header.version, CTRL_HEADER_VERSION);
}

TEST_F(CopyCtrlParserTest, ReadCopyCtrl)
{
    CopyCtrlFileEntry fileEntry1 {};
    CopyCtrlDirEntry dirEntry1 {};

    CopyCtrlParser copyCtrlParser(m_ctrlFile);
    CTRL_FILE_RETCODE ret = copyCtrlParser.ReadEntry(fileEntry1, dirEntry1);
    EXPECT_EQ(ret, CTRL_FILE_RETCODE::FAILED);

    ret = copyCtrlParser.Open(CTRL_FILE_OPEN_MODE::READ);
    EXPECT_EQ(ret, CTRL_FILE_RETCODE::SUCCESS);

    ret = copyCtrlParser.ReadEntry(fileEntry1, dirEntry1);
    EXPECT_EQ(ret, CTRL_FILE_RETCODE::SUCCESS);
    cout << "dirName: " << dirEntry1.m_dirName << "\n" ;
    cout << "mode: " << dirEntry1.m_mode << "\n";
    cout << "metaFileName: " << dirEntry1.m_metaFileName << "\n";
    cout << "metaFileOffset: " << dirEntry1.metaFileOffset << "\n";
    cout << "metaFileReadLen: " << dirEntry1.metaFileReadLen << "\n";
    cout << "fileCount: " << dirEntry1.m_fileCount << "\n";
    cout << "aclFlag: " << dirEntry1.m_aclFlag << "\n";
    cout << "\n";

    CopyCtrlFileEntry fileEntry2 {};
    CopyCtrlDirEntry dirEntry2 {};
    ret = copyCtrlParser.ReadEntry(fileEntry2, dirEntry2);
    EXPECT_EQ(ret, CTRL_FILE_RETCODE::SUCCESS);
    cout << "fileName: " << fileEntry2.m_fileName << "\n" ;


    CopyCtrlFileEntry fileEntry3 {};
    CopyCtrlDirEntry dirEntry3 {};
    ret = copyCtrlParser.ReadEntry(fileEntry3, dirEntry3);
    EXPECT_EQ(ret, CTRL_FILE_RETCODE::SUCCESS);
    cout << "fileName: " << fileEntry3.m_fileName << "\n" ;

    CopyCtrlFileEntry fileEntry4 {};
    CopyCtrlDirEntry dirEntry4 {};
    ret = copyCtrlParser.ReadEntry(fileEntry4, dirEntry4);
    EXPECT_EQ(ret, CTRL_FILE_RETCODE::SUCCESS);
    cout << "dirName: " << dirEntry4.m_dirName << "\n" ;

    ret = copyCtrlParser.ReadEntry(fileEntry4, dirEntry4);
    EXPECT_EQ(ret, CTRL_FILE_RETCODE::READ_EOF);

    ret = copyCtrlParser.Close(CTRL_FILE_OPEN_MODE::READ);
    EXPECT_EQ(ret, CTRL_FILE_RETCODE::SUCCESS);

    try {
        boost::filesystem::remove(m_ctrlFile);
    } catch (const boost::filesystem::filesystem_error &e) {
        cout << "remove() exeption: " << e.code().message() << endl;
    }
}