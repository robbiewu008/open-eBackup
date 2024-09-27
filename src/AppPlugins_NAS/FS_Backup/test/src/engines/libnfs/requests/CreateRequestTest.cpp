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
#include "CreateRequest.h"
#include "LibnfsCopyWriter.h"

using namespace std;
using namespace FS_Backup;
using namespace Module;

namespace  {
    const int MP_SUCCESS = 0;
    const int MP_FAILED = 1;
}

class CreateRequestTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    NfsCommonData m_commonData {};
    NfsContextContainer m_nfsContextContainer;
    BackupTimer m_timer;
};

void CreateRequestTest::SetUp()
{
    BackupQueueConfig config {DEFAULT_BACKUP_QUEUE_SIZE, DEFAULT_BACKUP_QUEUE_MEMORY_SIZE};
    bool abort = false;
    m_commonData.abort = &abort;
    m_commonData.nfsContextContainer = &m_nfsContextContainer;
    m_commonData.pktStats = make_shared<PacketStats>();
    m_commonData.writeQueue = std::make_shared<BackupQueue<FileHandle>>(config);
    m_commonData.controlInfo = std::make_shared<BackupControlInfo>();
    m_commonData.timer = &m_timer;
    m_commonData.skipFailure = false;
    m_commonData.commonObj = nullptr;
    m_commonData.IsResumeSendCb = LibnfsCopyWriter::IsResumeSendCb;
    m_commonData.ResumeSendCb = LibnfsCopyWriter::ResumeSendCb;
}

void CreateRequestTest::TearDown()
{
    GlobalMockObject::verify(); // 校验mock规范并清除mock规范
}

void CreateRequestTest::SetUpTestCase()
{}

void CreateRequestTest::TearDownTestCase()
{}

static char* NfsGetError_Stub()
{
    struct nfs_context *nfsContext = nullptr;
    nfsContext = nfs_init_context();
    return nfs_get_error(nfsContext);
}

TEST_F(CreateRequestTest, CreateCreateCbData)
{
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(BackupIOEngine::LIBNFS, BackupIOEngine::LIBNFS);
    fileHandle.m_file->SetFlag(TRUNCATE);
    NfsCommonData commonData {};
    struct nfsfh* nfsfh {};
    uint32_t openFlag = 0;
    RestoreReplacePolicy restoreReplacePolicy {};
    CreateCreateCbData(fileHandle, commonData, nfsfh, openFlag, restoreReplacePolicy);
}

TEST_F(CreateRequestTest, SendCreate)
{
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(BackupIOEngine::LIBNFS, BackupIOEngine::LIBNFS);

    NfsCreateCbData *cbData = nullptr;
    EXPECT_EQ(SendCreate(fileHandle, cbData), MP_FAILED);

    cbData = new(nothrow) NfsCreateCbData();
    cbData->fileHandle = fileHandle;
    cbData->writeCommonData = &m_commonData;
    cbData->nfsfh = nullptr;

    MOCKER_CPP(&NfsContextWrapper::NfsCreateAsync)
            .stubs()
            .will(returnValue(0))
            .then(returnValue(1));
    EXPECT_EQ(SendCreate(fileHandle, cbData), MP_SUCCESS);

    MOCKER_CPP(&NfsContextWrapper::NfsGetError)
            .stubs()
            .will(invoke(NfsGetError_Stub));
    EXPECT_EQ(SendCreate(fileHandle, cbData), MP_FAILED);

    NfsCreateCbData *cbData2 = new(nothrow) NfsCreateCbData();
    cbData2->fileHandle = fileHandle;
    cbData2->writeCommonData = &m_commonData;
    cbData2->nfsfh = (struct nfsfh*) malloc(sizeof(struct nfsfh));
    MOCKER_CPP(&NfsContextWrapper::NfsCreateAsyncWithDirHandle)
            .stubs()
            .will(returnValue(0));
    EXPECT_EQ(SendCreate(fileHandle, cbData2), MP_SUCCESS);
}

TEST_F(CreateRequestTest, SendCreateCb)
{
    int status = 0;
    struct nfs_context *nfs;
    nfs = nfs_init_context();
    void *data = (struct nfsfh*) malloc(sizeof(struct nfsfh));
    void *privateData = nullptr;

    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(BackupIOEngine::LIBNFS, BackupIOEngine::LIBNFS);
    SendCreateCb(status, nfs, data, privateData);

    auto cbData = new(nothrow) NfsCreateCbData();
    cbData->fileHandle = fileHandle;
    cbData->writeCommonData = nullptr;
    cbData->nfsfh = nullptr;

    SendCreateCb(status, nfs, data, cbData);

    auto cbData1 = new(nothrow) NfsCreateCbData();
    cbData1->fileHandle = fileHandle;
    cbData1->writeCommonData = &m_commonData;
    cbData1->nfsfh = (struct nfsfh*) malloc(sizeof(struct nfsfh));

    MOCKER_CPP(&LibnfsCopyWriter::IsResumeSendCb)
            .stubs()
            .will(returnValue(true));
    MOCKER_CPP(&LibnfsCopyWriter::ResumeSendCb)
            .stubs()
            .will(ignoreReturnValue());
    SendCreateCb(status, nfs, data, cbData1);

    auto cbData2 = new(nothrow) NfsCreateCbData();
    status = -1;
    cbData2->fileHandle = fileHandle;
    cbData2->writeCommonData = &m_commonData;
    cbData2->nfsfh = (struct nfsfh*) malloc(sizeof(struct nfsfh));

    MOCKER_CPP(&LibnfsCopyWriter::IsResumeSendCb)
            .stubs()
            .will(returnValue(true));
    MOCKER_CPP(&LibnfsCopyWriter::ResumeSendCb)
            .stubs()
            .will(ignoreReturnValue());
    MOCKER_CPP(&HandleCreateFailure)
            .stubs()
            .will(ignoreReturnValue());
    SendCreateCb(status, nfs, data, cbData2);

    auto cbData3 = new(nothrow) NfsCreateCbData();
    bool abort = true;
    status = 0;
    m_commonData.abort = &abort;
    cbData3->fileHandle = fileHandle;
    cbData3->writeCommonData = &m_commonData;
    cbData3->nfsfh = (struct nfsfh*) malloc(sizeof(struct nfsfh));

    MOCKER_CPP(&LibnfsCopyWriter::IsResumeSendCb)
            .stubs()
            .will(returnValue(true));
    MOCKER_CPP(&LibnfsCopyWriter::ResumeSendCb)
            .stubs()
            .will(ignoreReturnValue());
    MOCKER_CPP(&Libnfscommonmethods::FreeNfsFh)
            .stubs()
            .will(ignoreReturnValue());
    SendCreateCb(status, nfs, data, cbData3);
}

TEST_F(CreateRequestTest, HandleCreateFailure)
{
    NfsCommonData *commonData = nullptr;
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(BackupIOEngine::LIBNFS, BackupIOEngine::LIBNFS);
    fileHandle.m_retryCnt = 0;
    int status = 0;
    struct nfs_context *nfs = nfs_init_context();
    NfsContextWrapper nfsContextWrapper(nfs);

    HandleCreateFailure(commonData, fileHandle, status, nfs);

    NfsCommonData *commonData1 = &m_commonData;
    status = -EINTR;
    nfs_get_error(nfs);
    HandleCreateFailure(commonData1, fileHandle, status, nfs);

    status = -EINTR;
    fileHandle.m_retryCnt = 11;
    nfs_get_error(nfs);
    HandleCreateFailure(commonData1, fileHandle, status, nfs);

    status = -BACKUP_ERR_ENOENT;
    nfs_get_error(nfs);
    MOCKER_CPP(&NfsContextWrapper::NfsGetError)
            .stubs()
            .will(invoke(NfsGetError_Stub))
            .then(invoke(NfsGetError_Stub))
            .then(invoke(NfsGetError_Stub));
    HandleCreateFailure(commonData1, fileHandle, status, nfs);

    status = -BACKUP_ERR_EEXIST;
    nfs_get_error(nfs);
    HandleCreateFailure(commonData1, fileHandle, status, nfs);

    status = 0;
    HandleCreateFailure(commonData1, fileHandle, status, nfs);
}

TEST_F(CreateRequestTest, CreateFailureHandling)
{
    auto commonData = nullptr;
    int status;
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(BackupIOEngine::LIBNFS, BackupIOEngine::LIBNFS);
    fileHandle.m_file->m_fileName = "1.txt";

    MOCKER_CPP(&Libnfscommonmethods::RemoveHardLinkMapEntryIfFileCreationFailed)
            .stubs()
            .will(returnValue(0))
            .then(returnValue(0))
            .then(returnValue(0))
            .then(returnValue(0));
    CreateFailureHandling(commonData, status, fileHandle);

    NfsCommonData *commonData1 = &m_commonData;
    status = -ERANGE;
    CreateFailureHandling(commonData1, status, fileHandle);

    status = -EACCES;
    CreateFailureHandling(commonData1, status, fileHandle);

    status = -1;
    CreateFailureHandling(commonData1, status, fileHandle);
}

static NfsLstatCbData* NfsLstatCbData_Stub()
{
    auto cbData = nullptr;
    return cbData;
}

static NfsLstatCbData* NfsLstatCbData_Stub1()
{
    auto cbData = new(nothrow) NfsLstatCbData();
    return cbData;
}

TEST_F(CreateRequestTest, SendLstatFromCreate)
{
    FileHandle fileHandle {};
    NfsCommonData *commonData = nullptr;
    struct nfsfh *nfsfh = nullptr;
    RestoreReplacePolicy restoreReplacePolicy {};

    EXPECT_EQ(SendLstatFromCreate(fileHandle, commonData, nfsfh, restoreReplacePolicy), MP_FAILED);

    NfsCommonData *commonData1 = &m_commonData;
    MOCKER_CPP(&CreateLstatCbData)
            .stubs()
            .will(invoke(NfsLstatCbData_Stub));
    MOCKER_CPP(&CreateFailureHandling)
            .stubs()
            .will(returnValue(0))
            .then(returnValue(0));
    EXPECT_EQ(SendLstatFromCreate(fileHandle, commonData1, nfsfh, restoreReplacePolicy), MP_FAILED);
}