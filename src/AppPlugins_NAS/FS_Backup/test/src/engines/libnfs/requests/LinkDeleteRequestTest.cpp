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
#include "LinkDeleteRequest.h"
#include "LibnfsCopyWriter.h"

using namespace std;
using namespace FS_Backup;
using namespace Module;

namespace  {
    const int MP_SUCCESS = 0;
    const int MP_FAILED = 1;
}

class LinkDeleteRequestTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    NfsCommonData m_commonData {};
    NfsContextContainer m_nfsContextContainer;
    BackupTimer m_timer;
};

void LinkDeleteRequestTest::SetUp()
{
    BackupQueueConfig config {DEFAULT_BACKUP_QUEUE_SIZE, DEFAULT_BACKUP_QUEUE_MEMORY_SIZE};
    bool abort = false;
    m_commonData.nfsContextContainer = &m_nfsContextContainer;
    m_commonData.syncNfsContextContainer = &m_nfsContextContainer;
    m_commonData.pktStats = make_shared<PacketStats>();
    m_commonData.controlInfo = std::make_shared<BackupControlInfo>();
    m_commonData.skipFailure = false;
    m_commonData.hardlinkMap = make_shared<HardLinkMap>();
    m_commonData.abort = &abort;
    m_commonData.commonObj = nullptr;
    m_commonData.IsResumeSendCb = LibnfsCopyWriter::IsResumeSendCb;
    m_commonData.ResumeSendCb = LibnfsCopyWriter::ResumeSendCb;
    m_commonData.timer = &m_timer;
    m_commonData.writeQueue = std::make_shared<BackupQueue<FileHandle>>(config);
}

void LinkDeleteRequestTest::TearDown()
{
    GlobalMockObject::verify(); // 校验mock规范并清除mock规范
}

void LinkDeleteRequestTest::SetUpTestCase()
{}

void LinkDeleteRequestTest::TearDownTestCase()
{}

static char* NfsGetError_Stub()
{
    struct nfs_context *nfsContext = nullptr;
    nfsContext = nfs_init_context();
    return nfs_get_error(nfsContext);
}

static nfs_context* GetNfsContext_Stub()
{
    struct nfs_context *nfsContext = nullptr;
    nfsContext = nfs_init_context();
    return nfsContext;
}
TEST_F(LinkDeleteRequestTest, CreateLinkDeleteCbData)
{
    FileHandle fileHandle {};
    NfsCommonData commonData {};
    struct nfsfh* nfsfh;
    CreateLinkDeleteCbData(fileHandle, commonData);
}

TEST_F(LinkDeleteRequestTest, SendLinkDelete)
{
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(BackupIOEngine::LIBNFS, BackupIOEngine::LIBNFS);
    fileHandle.m_file->m_fileName = "1.txt";
    string targetPath {};

    NfsLinkDeleteCbData *cbData = nullptr;
    int ret = SendLinkDelete(fileHandle, cbData);
    EXPECT_EQ(ret, MP_FAILED);

    NfsLinkDeleteCbData *cbData1 = new(nothrow) NfsLinkDeleteCbData();
    cbData1->fileHandle = fileHandle;
    cbData1->writeCommonData = &m_commonData;

    MOCKER_CPP(&NfsContextWrapper::NfsUnlinkAsync)
            .stubs()
            .will(returnValue(0))
            .then(returnValue(1));
    EXPECT_EQ(SendLinkDelete(fileHandle, cbData1), MP_SUCCESS);

    MOCKER_CPP(&NfsContextWrapper::NfsGetError)
            .stubs()
            .will(invoke(NfsGetError_Stub));
    EXPECT_EQ(SendLinkDelete(fileHandle, cbData1), MP_FAILED);
}

TEST_F(LinkDeleteRequestTest, SendLinkDeleteCb)
{
    int status = 0;
    struct nfs_context *nfs;
    nfs = nfs_init_context();
    void *data = nullptr;
    void *privateData = nullptr;
    RestoreReplacePolicy restoreReplacePolicy = RestoreReplacePolicy::IGNORE_EXIST;
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(BackupIOEngine::LIBNFS, BackupIOEngine::LIBNFS);
    fileHandle.m_file->m_fileName = "1.txt";

    SendLinkDeleteCb(status, nfs, data, privateData);

    auto cbData = new(nothrow) NfsLinkDeleteCbData();
    cbData->fileHandle = fileHandle;
    cbData->writeCommonData = nullptr;

    SendLinkDeleteCb(status, nfs, data, cbData);

    auto cbData1 = new(nothrow) NfsLinkDeleteCbData();
    cbData1->fileHandle = fileHandle;
    cbData1->writeCommonData = &m_commonData;

    MOCKER_CPP(&LibnfsCopyWriter::IsResumeSendCb)
            .stubs()
            .will(returnValue(true))
            .then(returnValue(true))
            .then(returnValue(true));
    MOCKER_CPP(&LibnfsCopyWriter::ResumeSendCb)
            .stubs()
            .will(ignoreReturnValue())
            .then(ignoreReturnValue())
            .then(ignoreReturnValue());
    SendLinkDeleteCb(status, nfs, data, cbData1);

    auto cbData2 = new(nothrow) NfsLinkDeleteCbData();
    status = -1;
    cbData2->fileHandle = fileHandle;
    cbData2->writeCommonData = &m_commonData;

    MOCKER_CPP(&HandleLinkDeleteFailure)
            .stubs()
            .will(ignoreReturnValue());
    SendLinkDeleteCb(status, nfs, data, cbData2);

    auto cbData3 = new(nothrow) NfsLinkDeleteCbData();
    bool abort1 = true;
    status = 0;
    m_commonData.abort = &abort1;
    cbData3->fileHandle = fileHandle;
    cbData3->writeCommonData = &m_commonData;

    SendLinkDeleteCb(status, nfs, data, cbData3);
}

TEST_F(LinkDeleteRequestTest, HandleLinkDeleteFailure)
{
    NfsCommonData *commonData = nullptr;
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(BackupIOEngine::LIBNFS, BackupIOEngine::LIBNFS);
    fileHandle.m_file->m_fileName = "1.txt";
    fileHandle.m_retryCnt = 0;
    int status = 0;
    struct nfs_context *nfs = nullptr;
    nfs = nfs_init_context();
    NfsContextWrapper nfsContextWrapper(nfs);

    HandleLinkDeleteFailure(commonData, status, fileHandle, nfs);

    NfsCommonData *commonData1 = &m_commonData;
    status = -BACKUP_ERR_ENOENT;
    HandleLinkDeleteFailure(commonData1, status, fileHandle, nfs);

    status = -BACKUP_ERR_NOTEMPTY;
    HandleLinkDeleteFailure(commonData1, status, fileHandle, nfs);

    status = -EINTR;
    HandleLinkDeleteFailure(commonData1, status, fileHandle, nfs);

    status = -EINTR;
    fileHandle.m_retryCnt = 11;
    MOCKER_CPP(&LinkDeleteFailureHandling)
            .stubs()
            .will(ignoreReturnValue());
    HandleLinkDeleteFailure(commonData1, status, fileHandle, nfs);

    status = 1;
    nfs_get_error(nfs);
    MOCKER_CPP(&LinkDeleteFailureHandling)
            .stubs()
            .will(ignoreReturnValue());
    HandleLinkDeleteFailure(commonData1, status, fileHandle, nfs);
}

TEST_F(LinkDeleteRequestTest, LinkDeleteFailureHandling)
{
    auto commonData = nullptr;
    int status;
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(BackupIOEngine::LIBNFS, BackupIOEngine::LIBNFS);
    fileHandle.m_file->m_fileName = "1.txt";

    MOCKER_CPP(&FileDesc::SetSrcState)
            .stubs()
            .will(returnValue(FileDescState::INIT));
    MOCKER_CPP(&FileDesc::SetDstState)
            .stubs()
            .will(returnValue(FileDescState::INIT));
    MOCKER_CPP(&Libnfscommonmethods::RemoveHardLinkMapEntryIfFileCreationFailed)
            .stubs()
            .will(returnValue(0))
            .then(returnValue(0))
            .then(returnValue(0))
            .then(returnValue(0));
    LinkDeleteFailureHandling(commonData, status, fileHandle);

    NfsCommonData *commonData1 = &m_commonData;
    status = -ERANGE;
    LinkDeleteFailureHandling(commonData1, status, fileHandle);

    status = -EACCES;
    LinkDeleteFailureHandling(commonData1, status, fileHandle);

    status = -1;
    LinkDeleteFailureHandling(commonData1, status, fileHandle);
}

TEST_F(LinkDeleteRequestTest, SendLinkDeleteSync)
{
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(BackupIOEngine::LIBNFS, BackupIOEngine::LIBNFS);
    fileHandle.m_file->m_fileName = "1.txt";
    fileHandle.m_file->m_nlink = 2;
    NfsLinkDeleteCbData *cbData = nullptr;
    RestoreReplacePolicy restoreReplacePolicy = RestoreReplacePolicy::IGNORE_EXIST;

    int ret = SendLinkDeleteSync(fileHandle, cbData);
    EXPECT_EQ(ret, MP_FAILED);

    NfsLinkDeleteCbData *cbData1 = new(nothrow) NfsLinkDeleteCbData();
    cbData1->fileHandle = fileHandle;
    cbData1->writeCommonData = nullptr;

    ret = SendLinkDeleteSync(fileHandle, cbData1);
    EXPECT_EQ(ret, MP_FAILED);

    NfsLinkDeleteCbData *cbData3 = new(nothrow) NfsLinkDeleteCbData();
    cbData3->fileHandle = fileHandle;
    cbData3->writeCommonData = &m_commonData;

    MOCKER_CPP(&NfsContextWrapper::NfsUnlinkLock)
            .stubs()
            .will(returnValue(0))
            .then(returnValue(0))
            .then(returnValue(1));
    MOCKER_CPP(&HandleLinkDeleteSyncStatus)
            .stubs()
            .will(returnValue(0))
            .then(returnValue(0))
            .then(returnValue(1));
    MOCKER_CPP(&HardLinkMap::GetTargetPath)
            .stubs()
            .will(returnValue(0));
    EXPECT_EQ(SendLinkDeleteSync(fileHandle, cbData3), MP_SUCCESS);

    NfsLinkDeleteCbData *cbData4 = new(nothrow) NfsLinkDeleteCbData();
    fileHandle.m_file->m_nlink = 1;
    fileHandle.m_file->m_mode = 41471;
    cbData4->fileHandle = fileHandle;
    cbData4->writeCommonData = &m_commonData;
    EXPECT_EQ(SendLinkDeleteSync(fileHandle, cbData4), MP_SUCCESS);

    NfsLinkDeleteCbData *cbData2 = new(nothrow) NfsLinkDeleteCbData();
    cbData2->fileHandle = fileHandle;
    cbData2->writeCommonData = &m_commonData;
    EXPECT_EQ(SendLinkDeleteSync(fileHandle, cbData2), MP_FAILED);
}

TEST_F(LinkDeleteRequestTest, HandleLinkDeleteSyncStatus)
{
    int status = 0;
    std::shared_ptr<NfsContextWrapper> nfs = m_commonData.syncNfsContextContainer->GetCurrContext();
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(BackupIOEngine::LIBNFS, BackupIOEngine::LIBNFS);
    fileHandle.m_file->m_fileName = "1.txt";
    fileHandle.m_retryCnt = 0;

    HandleLinkDeleteSyncStatus(status, fileHandle, nfs, nullptr);

    HandleLinkDeleteSyncStatus(status, fileHandle, nfs, &m_commonData);

    status = -BACKUP_ERR_ENOENT;
    int ret = HandleLinkDeleteSyncStatus(status, fileHandle, nfs, &m_commonData);
    EXPECT_EQ(ret, MP_SUCCESS);

    status = -BACKUP_ERR_NOTEMPTY;
    MOCKER_CPP(&Libnfscommonmethods::LibNfsDeleteDirectorySync)
            .stubs()
            .will(returnValue(0))
            .then(returnValue(1));
    EXPECT_EQ(HandleLinkDeleteSyncStatus(status, fileHandle, nfs, &m_commonData), MP_SUCCESS);

    MOCKER_CPP(&LinkDeleteFailureHandling)
            .stubs()
            .will(ignoreReturnValue());
    EXPECT_EQ(HandleLinkDeleteSyncStatus(status, fileHandle, nfs, &m_commonData), MP_FAILED);

    status = -EINTR;
    HandleLinkDeleteSyncStatus(status, fileHandle, nfs, &m_commonData);

    status = -EINTR;
    fileHandle.m_retryCnt = 11;
    MOCKER_CPP(&LinkDeleteFailureHandling)
            .stubs()
            .will(ignoreReturnValue());
    HandleLinkDeleteSyncStatus(status, fileHandle, nfs, &m_commonData);

    status = 1;
    MOCKER_CPP(&NfsContextWrapper::GetNfsContext)
            .stubs()
            .will(invoke(GetNfsContext_Stub));
    MOCKER_CPP(&LinkDeleteFailureHandling)
            .stubs()
            .will(ignoreReturnValue());
    EXPECT_EQ(HandleLinkDeleteSyncStatus(status, fileHandle, nfs, &m_commonData), MP_FAILED);
}
