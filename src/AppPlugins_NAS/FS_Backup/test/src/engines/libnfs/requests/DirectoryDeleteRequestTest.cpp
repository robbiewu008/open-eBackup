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
#include "gtest/gtest.h"
#include "mockcpp/mockcpp.hpp"
#include "DirectoryDeleteRequest.h"

using namespace std;
using namespace FS_Backup;
using namespace Module;

namespace  {
    const int MP_SUCCESS = 0;
    const int MP_FAILED = 1;
}

class DirectoryDeleteRequestTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    NfsCommonData m_commonData {};
    NfsContextContainer m_nfsContextContainer;
    HardLinkMap hardLinkMap {};
};

void DirectoryDeleteRequestTest::SetUp()
{
    BackupQueueConfig config {DEFAULT_BACKUP_QUEUE_SIZE, DEFAULT_BACKUP_QUEUE_MEMORY_SIZE};
    m_commonData.nfsContextContainer = &m_nfsContextContainer;
    m_commonData.writeQueue = std::make_shared<BackupQueue<FileHandle>>(config);
    m_commonData.controlInfo = std::make_shared<BackupControlInfo>();
    m_commonData.skipFailure = false;
    m_commonData.hardlinkMap = make_shared<HardLinkMap>();
}

void DirectoryDeleteRequestTest::TearDown()
{
    GlobalMockObject::verify(); // 校验mock规范并清除mock规范
}

void DirectoryDeleteRequestTest::SetUpTestCase()
{}

void DirectoryDeleteRequestTest::TearDownTestCase()
{}

TEST_F(DirectoryDeleteRequestTest, CreateDirDeleteCbData)
{
    FileHandle fileHandle {};
    NfsCommonData commonData {};
    CreateDirDeleteCbData(fileHandle, commonData);
}

TEST_F(DirectoryDeleteRequestTest, SendDirDelete)
{
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(BackupIOEngine::LIBNFS, BackupIOEngine::LIBNFS);

    NfsDirDeleteCbData *cbData = nullptr;
    int ret = SendDirDelete(fileHandle, cbData);
    EXPECT_EQ(ret, MP_FAILED);

    NfsDirDeleteCbData *cbData1 = new(nothrow) NfsDirDeleteCbData();
    cbData1->fileHandle = fileHandle;
    cbData1->writeCommonData = nullptr;
    ret = SendDirDelete(fileHandle, cbData1);
    EXPECT_EQ(ret, MP_FAILED);

    NfsDirDeleteCbData *cbData3 = new(nothrow) NfsDirDeleteCbData();
    cbData3->fileHandle = fileHandle;
    cbData3->writeCommonData = &m_commonData;

    MOCKER_CPP(&Libnfscommonmethods::LibNfsDeleteDirectorySync)
            .stubs()
            .will(returnValue(0))
            .then(returnValue(1));
    EXPECT_EQ(SendDirDelete(fileHandle, cbData3), MP_SUCCESS);

    NfsDirDeleteCbData *cbData4 = new(nothrow) NfsDirDeleteCbData();
    cbData4->fileHandle = fileHandle;
    cbData4->writeCommonData = &m_commonData;

    MOCKER_CPP(&FileDesc::SetSrcState)
            .stubs()
            .will(returnValue(FileDescState::INIT));
    MOCKER_CPP(&FileDesc::SetDstState)
            .stubs()
            .will(returnValue(FileDescState::INIT));
    MOCKER_CPP(&Libnfscommonmethods::RemoveHardLinkMapEntryIfFileCreationFailed)
            .stubs()
            .will(returnValue(0));
    EXPECT_EQ(SendDirDelete(fileHandle, cbData4), MP_FAILED);
}