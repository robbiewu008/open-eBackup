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
#include "MknodRequest.h"
#include "LibnfsCopyWriter.h"

using namespace std;
using namespace FS_Backup;
using namespace Module;

namespace  {
    const int MP_SUCCESS = 0;
    const int MP_FAILED = 1;
}

class MknodRequestTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    NfsCommonData m_commonData {};
    NfsContextContainer m_nfsContextContainer;
    BackupTimer m_timer;
};

void MknodRequestTest::SetUp()
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
    m_commonData.IsResumeSendCb = LibnfsCopyWriter::IsResumeSendCb;
    m_commonData.ResumeSendCb = LibnfsCopyWriter::ResumeSendCb;
    m_commonData.timer = &m_timer;
    m_commonData.readQueue = std::make_shared<BackupQueue<FileHandle>>(config);
    m_commonData.writeQueue = std::make_shared<BackupQueue<FileHandle>>(config);
}

void MknodRequestTest::TearDown()
{
    GlobalMockObject::verify(); // 校验mock规范并清除mock规范
}

void MknodRequestTest::SetUpTestCase()
{}

void MknodRequestTest::TearDownTestCase()
{}

static char* NfsGetError_Stub()
{
    struct nfs_context *nfsContext = nullptr;
    nfsContext = nfs_init_context();
    return nfs_get_error(nfsContext);
}

TEST_F(MknodRequestTest, CreateMknodCbData)
{
    FileHandle fileHandle {};
    NfsCommonData commonData {};
    struct nfsfh* nfsfh {};
    CreateMknodCbData(fileHandle, commonData, nfsfh);
}

TEST_F(MknodRequestTest, SendMknod)
{
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(BackupIOEngine::LIBNFS, BackupIOEngine::LIBNFS);
    fileHandle.m_file->m_fileName = "1.txt";

    NfsMknodCbData *cbData = nullptr;
    EXPECT_EQ(SendMknod(fileHandle, cbData), MP_FAILED);

    NfsMknodCbData *cbData1 = new(nothrow) NfsMknodCbData();
    cbData1->fileHandle = fileHandle;
    cbData1->writeCommonData = &m_commonData;
    cbData1->nfsfh = (struct nfsfh*) malloc(sizeof(struct nfsfh));

    MOCKER_CPP(&NfsContextWrapper::NfsMknodAsync)
            .stubs()
            .will(returnValue(0))
            .then(returnValue(1));
    EXPECT_EQ(SendMknod(fileHandle, cbData1), MP_SUCCESS);

    MOCKER_CPP(&NfsContextWrapper::NfsGetError)
            .stubs()
            .will(invoke(NfsGetError_Stub));
    EXPECT_EQ(SendMknod(fileHandle, cbData1), MP_FAILED);
}

TEST_F(MknodRequestTest, SendMknodCb)
{
    int status = 0;
    struct nfs_context *nfs;
    nfs = nfs_init_context();
    NfsContextWrapper nfsContextWrapper(nfs);
    void *data = (struct nfsfh*) malloc(sizeof(struct nfsfh));
    void *privateData = nullptr;

    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(BackupIOEngine::LIBNFS, BackupIOEngine::LIBNFS);
    SendMknodCb(status, nfs, data, privateData);

    auto cbData = new(nothrow) NfsMknodCbData();
    cbData->fileHandle = fileHandle;
    cbData->writeCommonData = nullptr;
    SendMknodCb(status, nfs, data, cbData);

    auto cbData1 = new(nothrow) NfsMknodCbData();
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
    MOCKER_CPP(&HandleMknodSuccess)
            .stubs()
            .will(ignoreReturnValue());
    SendMknodCb(status, nfs, data, cbData1);

    auto cbData2 = new(nothrow) NfsMknodCbData();
    status = -1;
    cbData2->fileHandle = fileHandle;
    cbData2->writeCommonData = &m_commonData;
    cbData2->nfsfh = (struct nfsfh*) malloc(sizeof(struct nfsfh));

    MOCKER_CPP(&HandleMknodFailure)
            .stubs()
            .will(ignoreReturnValue());
    SendMknodCb(status, nfs, data, cbData2);

    auto cbData3 = new(nothrow) NfsMknodCbData();
    bool abort1 = true;
    status = 0;
    m_commonData.abort = &abort1;
    cbData3->fileHandle = fileHandle;
    cbData3->writeCommonData = &m_commonData;
    cbData3->nfsfh = (struct nfsfh*) malloc(sizeof(struct nfsfh));

    MOCKER_CPP(&Libnfscommonmethods::FreeNfsFh)
            .stubs()
            .will(ignoreReturnValue());
    SendMknodCb(status, nfs, data, cbData3);
}

TEST_F(MknodRequestTest, HandleMknodSuccess)
{
    NfsCommonData *commonData = nullptr;
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(BackupIOEngine::LIBNFS, BackupIOEngine::LIBNFS);
    fileHandle.m_file->m_fileName = "1.txt";
    fileHandle.m_file->m_nlink = 2;
    int status = 0;
    struct nfs_context *nfs = nfs_init_context();
    void *data = (struct nfsfh*) malloc(sizeof(struct nfsfh));
    HandleMknodSuccess(commonData, fileHandle, data, nfs);

    NfsCommonData *commonData1 = &m_commonData;
    MOCKER_CPP(&HardLinkMap::IsTargetCopied)
            .stubs()
            .will(returnValue(false));
    MOCKER_CPP(&HardLinkMap::SetTargetCopied)
            .stubs()
            .will(returnValue(true));
    HandleMknodSuccess(commonData1, fileHandle, data, nfs);
}

TEST_F(MknodRequestTest, HandleMknodFailure)
{
    NfsCommonData *commonData = nullptr;
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(BackupIOEngine::LIBNFS, BackupIOEngine::LIBNFS);
    fileHandle.m_file->m_fileName = "1.txt";
    fileHandle.m_retryCnt = 0;
    int status = 0;
    struct nfs_context *nfs = nfs_init_context();
    NfsContextWrapper nfsContextWrapper(nfs);

    HandleMknodFailure(commonData, fileHandle, status, nfs);

    NfsCommonData *commonData1 = &m_commonData;
    status = -BACKUP_ERR_NOTDIR;
    MOCKER_CPP(&MknodFailureHandling)
            .stubs()
            .will(ignoreReturnValue())
            .then(ignoreReturnValue())
            .then(ignoreReturnValue());
    HandleMknodFailure(commonData1, fileHandle, status, nfs);

    status = -BACKUP_ERR_EEXIST;
    HandleMknodFailure(commonData1, fileHandle, status, nfs);

    status = -EINTR;
    HandleMknodFailure(commonData1, fileHandle, status, nfs);

    status = -EINTR;
    fileHandle.m_retryCnt = 11;
    HandleMknodFailure(commonData1, fileHandle, status, nfs);

    status = -1;
    nfs_get_error(nfs);
    HandleMknodFailure(commonData1, fileHandle, status, nfs);
}

TEST_F(MknodRequestTest, MknodFailureHandling)
{
    auto commonData = nullptr;
    int status;
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(BackupIOEngine::LIBNFS, BackupIOEngine::LIBNFS);
    fileHandle.m_file->m_fileName = "1.txt";
    fileHandle.m_file->SetSrcState(FileDescState::INIT);

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
    MknodFailureHandling(commonData, status, fileHandle);

    NfsCommonData *commonData1 = &m_commonData;
    status = -ERANGE;
    MknodFailureHandling(commonData1, status, fileHandle);

    status = -EACCES;
    MknodFailureHandling(commonData1, status, fileHandle);

    status = -1;
    MknodFailureHandling(commonData1, status, fileHandle);
}