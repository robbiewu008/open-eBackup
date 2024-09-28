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
#include "HardLinkRequest.h"
#include "LibnfsHardlinkWriter.h"

using namespace std;
using namespace FS_Backup;
using namespace Module;

namespace  {
    const int MP_SUCCESS = 0;
    const int MP_FAILED = 1;
}

class HardLinkRequestTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    NfsCommonData m_commonData {};
    NfsContextContainer m_nfsContextContainer;
    BackupTimer m_timer;
};

void HardLinkRequestTest::SetUp()
{
    BackupQueueConfig config {DEFAULT_BACKUP_QUEUE_SIZE, DEFAULT_BACKUP_QUEUE_MEMORY_SIZE};
    bool abort = false;
    m_commonData.nfsContextContainer = &m_nfsContextContainer;
    m_commonData.pktStats = make_shared<PacketStats>();
    m_commonData.controlInfo = std::make_shared<BackupControlInfo>();
    m_commonData.skipFailure = false;
    m_commonData.hardlinkMap = make_shared<HardLinkMap>();
    m_commonData.abort = &abort;
    m_commonData.commonObj = nullptr;
    m_commonData.IsResumeSendCb = LibnfsHardlinkWriter::IsResumeSendCb;
    m_commonData.ResumeSendCb = LibnfsHardlinkWriter::ResumeSendCb;
    m_commonData.timer = &m_timer;
    m_commonData.writeQueue = std::make_shared<BackupQueue<FileHandle>>(config);
}

void HardLinkRequestTest::TearDown()
{
    GlobalMockObject::verify(); // 校验mock规范并清除mock规范
}

void HardLinkRequestTest::SetUpTestCase()
{}

void HardLinkRequestTest::TearDownTestCase()
{}

static char* NfsGetError_Stub()
{
    struct nfs_context *nfsContext = nullptr;
    nfsContext = nfs_init_context();
    return nfs_get_error(nfsContext);
}

TEST_F(HardLinkRequestTest, CreateHardLinkCbData)
{
    FileHandle fileHandle {};
    NfsCommonData commonData {};
    struct nfsfh* nfsfh;
    RestoreReplacePolicy restoreReplacePolicy {};
    string targetPath {};
    CreateHardLinkCbData(fileHandle, commonData, nfsfh, restoreReplacePolicy, targetPath);
}

TEST_F(HardLinkRequestTest, SendHardLink)
{
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(BackupIOEngine::LIBNFS, BackupIOEngine::LIBNFS);
    fileHandle.m_file->m_fileName = "1.txt";
    RestoreReplacePolicy restoreReplacePolicy {};
    string targetPath {};

    NfsHardLinkCbData *cbData = nullptr;
    int ret = SendHardLink(fileHandle, cbData);
    EXPECT_EQ(ret, MP_FAILED);

    NfsHardLinkCbData *cbData1 = new(nothrow) NfsHardLinkCbData();
    cbData1->fileHandle = fileHandle;
    cbData1->writeCommonData = &m_commonData;
    cbData1->nfsfh = nullptr;
    cbData1->restoreReplacePolicy = restoreReplacePolicy;
    cbData1->targetPath = targetPath;

    MOCKER_CPP(&NfsContextWrapper::NfsHardLinkAsync)
            .stubs()
            .will(returnValue(0))
            .then(returnValue(1));
    EXPECT_EQ(SendHardLink(fileHandle, cbData1), MP_SUCCESS);

    MOCKER_CPP(&NfsContextWrapper::NfsGetError)
            .stubs()
            .will(invoke(NfsGetError_Stub));
    EXPECT_EQ(SendHardLink(fileHandle, cbData1), MP_FAILED);
}

TEST_F(HardLinkRequestTest, SendHardLinkCb)
{
    int status = 0;
    struct nfs_context *nfs = nfs_init_context();
    void *data = nullptr;
    void *privateData = nullptr;
    RestoreReplacePolicy restoreReplacePolicy {};
    string targetPath {};

    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(BackupIOEngine::LIBNFS, BackupIOEngine::LIBNFS);
    fileHandle.m_file->m_inode = 10456;

    SendHardLinkCb(status, nfs, data, privateData);

    auto cbData = new(nothrow) NfsHardLinkCbData();
    cbData->fileHandle = fileHandle;
    cbData->writeCommonData = nullptr;
    cbData->nfsfh = nullptr;
    cbData->restoreReplacePolicy = restoreReplacePolicy;
    cbData->targetPath = targetPath;

    SendHardLinkCb(status, nfs, data, cbData);

    auto cbData1 = new(nothrow) NfsHardLinkCbData();
    cbData1->fileHandle = fileHandle;
    cbData1->writeCommonData = &m_commonData;
    cbData1->nfsfh = (struct nfsfh*) malloc(sizeof(struct nfsfh));
    cbData1->restoreReplacePolicy = restoreReplacePolicy;
    cbData1->targetPath = targetPath;

    MOCKER_CPP(&LibnfsHardlinkWriter::IsResumeSendCb)
            .stubs()
            .will(returnValue(true));
    MOCKER_CPP(&LibnfsHardlinkWriter::ResumeSendCb)
            .stubs()
            .will(ignoreReturnValue());
    SendHardLinkCb(status, nfs, data, cbData1);

    auto cbData2 = new(nothrow) NfsHardLinkCbData();
    status = -1;
    cbData2->fileHandle = fileHandle;
    cbData2->writeCommonData = &m_commonData;
    cbData2->nfsfh = (struct nfsfh*) malloc(sizeof(struct nfsfh));
    cbData2->restoreReplacePolicy = restoreReplacePolicy;
    cbData2->targetPath = targetPath;

    MOCKER_CPP(&LibnfsHardlinkWriter::IsResumeSendCb)
            .stubs()
            .will(returnValue(true));
    MOCKER_CPP(&LibnfsHardlinkWriter::ResumeSendCb)
            .stubs()
            .will(ignoreReturnValue());
    MOCKER_CPP(HandleHardLinkFailure)
            .stubs()
            .will(ignoreReturnValue());
    SendHardLinkCb(status, nfs, data, cbData2);

    auto cbData3 = new(nothrow) NfsHardLinkCbData();
    bool abort1 = true;
    status = 0;
    m_commonData.abort = &abort1;
    cbData3->fileHandle = fileHandle;
    cbData3->writeCommonData = &m_commonData;
    cbData3->nfsfh = (struct nfsfh*) malloc(sizeof(struct nfsfh));
    cbData3->restoreReplacePolicy = restoreReplacePolicy;
    cbData3->targetPath = targetPath;

    MOCKER_CPP(&LibnfsHardlinkWriter::IsResumeSendCb)
            .stubs()
            .will(returnValue(true));
    MOCKER_CPP(&LibnfsHardlinkWriter::ResumeSendCb)
            .stubs()
            .will(ignoreReturnValue());
    SendHardLinkCb(status, nfs, data, cbData3);
}

TEST_F(HardLinkRequestTest, HandleHardLinkFailure)
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
    RestoreReplacePolicy restoreReplacePolicy = RestoreReplacePolicy::NONE;

    HandleHardLinkFailure(commonData, fileHandle, restoreReplacePolicy, status, nfs);

    NfsCommonData *commonData1 = &m_commonData;
    status = -ESTALE;
    nfs_get_error(nfs);
    HandleHardLinkFailure(commonData1, fileHandle, restoreReplacePolicy, status, nfs);

    status = -BACKUP_ERR_NOTDIR;
    nfs_get_error(nfs);
    HandleHardLinkFailure(commonData1, fileHandle, restoreReplacePolicy, status, nfs);

    status = -BACKUP_ERR_EEXIST;
    nfs_get_error(nfs);
    HandleHardLinkFailure(commonData1, fileHandle, restoreReplacePolicy, status, nfs);

    status = -EINTR;
    nfs_get_error(nfs);
    HandleHardLinkFailure(commonData1, fileHandle, restoreReplacePolicy, status, nfs);

    status = -EINTR;
    nfs_get_error(nfs);
    fileHandle.m_retryCnt = 11;
    HandleHardLinkFailure(commonData1, fileHandle, restoreReplacePolicy, status, nfs);

    status = -BACKUP_ERR_EISDIR;
    nfs_get_error(nfs);
    HandleHardLinkFailure(commonData1, fileHandle, restoreReplacePolicy, status, nfs);

    status = -1;
    nfs_get_error(nfs);
    HandleHardLinkFailure(commonData1, fileHandle, restoreReplacePolicy, status, nfs);
}


TEST_F(HardLinkRequestTest, HandleExistStatus)
{
    auto commonData = nullptr;
    int status;
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(BackupIOEngine::LIBNFS, BackupIOEngine::LIBNFS);
    fileHandle.m_file->m_inode = 10456;
    RestoreReplacePolicy restoreReplacePolicy = RestoreReplacePolicy::NONE;

    HandleExistStatus(commonData, fileHandle, restoreReplacePolicy);

    NfsCommonData *commonData1 = &m_commonData;
    HandleExistStatus(commonData, fileHandle, restoreReplacePolicy);

    restoreReplacePolicy = RestoreReplacePolicy::IGNORE_EXIST;
    HandleExistStatus(commonData, fileHandle, restoreReplacePolicy);
}

TEST_F(HardLinkRequestTest, HardlinkFailureHandling)
{
    auto commonData = nullptr;
    int status;
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(BackupIOEngine::LIBNFS, BackupIOEngine::LIBNFS);
    MOCKER_CPP(&FileDesc::SetSrcState)
            .stubs()
            .will(returnValue(FileDescState::INIT));
    MOCKER_CPP(&FileDesc::SetDstState)
            .stubs()
            .will(returnValue(FileDescState::INIT));

    HardlinkFailureHandling(commonData, status, fileHandle);

    NfsCommonData *commonData1 = &m_commonData;
    status = -ERANGE;
    HardlinkFailureHandling(commonData1, status, fileHandle);

    status = -EACCES;
    HardlinkFailureHandling(commonData1, status, fileHandle);

    status = -1;
    HardlinkFailureHandling(commonData1, status, fileHandle);
}
