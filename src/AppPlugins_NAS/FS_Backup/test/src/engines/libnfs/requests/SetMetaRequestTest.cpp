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
#include "SetMetaRequest.h"
#include "LibnfsCopyWriter.h"

using namespace std;
using namespace FS_Backup;
using namespace Module;

namespace  {
    const int MP_SUCCESS = 0;
    const int MP_FAILED = 1;
}

class SetMetaRequestTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    NfsCommonData m_commonData {};
    NfsContextContainer m_nfsContextContainer;
    BackupTimer m_timer;
};

void SetMetaRequestTest::SetUp()
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
    m_commonData.timer = &m_timer;
    m_commonData.IsResumeSendCb = LibnfsCopyWriter::IsResumeSendCb;
    m_commonData.ResumeSendCb = LibnfsCopyWriter::ResumeSendCb;
    m_commonData.writeQueue = std::make_shared<BackupQueue<FileHandle>>(config);
    m_commonData.aggregateQueue = std::make_shared<BackupQueue<FileHandle>>(config);
}

void SetMetaRequestTest::TearDown()
{
    GlobalMockObject::verify(); // 校验mock规范并清除mock规范
}

void SetMetaRequestTest::SetUpTestCase()
{}

void SetMetaRequestTest::TearDownTestCase()
{}

static char* NfsGetError_Stub()
{
    struct nfs_context *nfsContext = nullptr;
    nfsContext = nfs_init_context();
    return nfs_get_error(nfsContext);
}

static nfs_context* GetNfsContext_Stub()
{
    struct nfs_context *nfsContext = nfs_init_context();
    return nfsContext;
}

static NfsCloseCbData* CbData_Stub()
{
    NfsCloseCbData *cbData = nullptr;
    return cbData;
}

TEST_F(SetMetaRequestTest, CreateSetMetaCbData)
{
    FileHandle fileHandle {};
    NfsCommonData commonData {};
    CreateSetMetaCbData(fileHandle, commonData);
}

TEST_F(SetMetaRequestTest, SendSetMeta)
{
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(BackupIOEngine::LIBNFS, BackupIOEngine::LIBNFS);
    fileHandle.m_file->m_fileName = "1.txt";

    NfsSetMetaCbData *cbData = nullptr;
    int ret = SendSetMeta(fileHandle, cbData);
    EXPECT_EQ(ret, MP_FAILED);

    NfsSetMetaCbData *cbData1 = new(nothrow) NfsSetMetaCbData();
    fileHandle.m_file->dstIOHandle.nfsFh = nullptr;
    cbData1->fileHandle = fileHandle;
    cbData1->writeCommonData = &m_commonData;

    MOCKER_CPP(&NfsContextWrapper::NfsFchmodChownUtimeAsync)
            .stubs()
            .will(returnValue(0))
            .then(returnValue(1));
    EXPECT_EQ(SendSetMeta(fileHandle, cbData1), MP_SUCCESS);

    MOCKER_CPP(&NfsContextWrapper::NfsGetError)
            .stubs()
            .will(invoke(NfsGetError_Stub));
    EXPECT_EQ(SendSetMeta(fileHandle, cbData1), MP_FAILED);

    NfsSetMetaCbData *cbData2 = new(nothrow) NfsSetMetaCbData();
    fileHandle.m_file->dstIOHandle.nfsFh = (struct nfsfh*) malloc(sizeof(struct nfsfh));
    cbData2->fileHandle = fileHandle;
    cbData2->writeCommonData = &m_commonData;

    MOCKER_CPP(&NfsContextWrapper::NfsChmodChownUtimeAsync)
            .stubs()
            .will(returnValue(0));
    EXPECT_EQ(SendSetMeta(fileHandle, cbData2), MP_SUCCESS);
}

TEST_F(SetMetaRequestTest, SendSetMetaCb)
{
    int status = 0;
    struct nfs_context *nfs = nfs_init_context();
    void *data = nullptr;
    void *privateData = nullptr;

    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(BackupIOEngine::LIBNFS, BackupIOEngine::LIBNFS);
    fileHandle.m_file->m_fileName = "1.txt";
    fileHandle.m_retryCnt = 0;
    SendSetMetaCb(status, nfs, data, privateData);

    auto cbData = new(nothrow) NfsSetMetaCbData();
    cbData->fileHandle = fileHandle;
    cbData->writeCommonData = nullptr;
    SendSetMetaCb(status, nfs, data, cbData);

    auto cbData1 = new(nothrow) NfsSetMetaCbData();
    status = 0;
    fileHandle.m_file->ClearFlag(IS_DIR);
    fileHandle.m_file->dstIOHandle.nfsFh = (struct nfsfh*) malloc(sizeof(struct nfsfh));
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
    MOCKER_CPP(&SendDstCloseFile)
            .stubs()
            .will(returnValue(0));
    SendSetMetaCb(status, nfs, data, cbData1);

    auto cbData2 = new(nothrow) NfsSetMetaCbData();
    status = -1;
    cbData2->fileHandle = fileHandle;
    cbData2->writeCommonData = &m_commonData;
    MOCKER_CPP(&HandleSetMetaFailure)
            .stubs()
            .will(ignoreReturnValue());
    SendSetMetaCb(status, nfs, data, cbData2);

    auto cbData3 = new(nothrow) NfsSetMetaCbData();
    bool abort1 = true;
    m_commonData.abort = &abort1;
    status = 0;
    fileHandle.m_file->dstIOHandle.nfsFh = (struct nfsfh*) malloc(sizeof(struct nfsfh));
    cbData3->fileHandle = fileHandle;
    cbData3->writeCommonData = &m_commonData;
    MOCKER_CPP(&Libnfscommonmethods::FreeNfsFh)
            .stubs()
            .will(ignoreReturnValue());
    SendSetMetaCb(status, nfs, data, cbData3);
}

TEST_F(SetMetaRequestTest, MetaModifiedCb)
{
    int status = 0;
    struct nfs_context *nfs;
    nfs = nfs_init_context();
    void *data = (struct nfsfh*) malloc(sizeof(struct nfsfh));
    void *privateData = nullptr;

    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(BackupIOEngine::LIBNFS, BackupIOEngine::LIBNFS);
    fileHandle.m_file->m_fileName = "1.txt";
    fileHandle.m_retryCnt = 0;
    MetaModifiedCb(status, nfs, data, privateData);

    auto cbData = new(nothrow) NfsSetMetaCbData();
    cbData->fileHandle = fileHandle;
    cbData->writeCommonData = nullptr;
    MetaModifiedCb(status, nfs, data, cbData);

    auto cbData2 = new(nothrow) NfsSetMetaCbData();
    status = 0;
    fileHandle.m_file->m_scannermode = CTRL_ENTRY_MODE_META_MODIFIED;
    fileHandle.m_file->m_mode = 16832;
    cbData2->fileHandle = fileHandle;
    cbData2->writeCommonData = &m_commonData;
    MOCKER_CPP(&LibnfsCopyWriter::IsResumeSendCb)
            .stubs()
            .will(returnValue(true))
            .then(returnValue(true))
            .then(returnValue(true))
            .then(returnValue(true))
            .then(returnValue(true));
    MOCKER_CPP(&LibnfsCopyWriter::ResumeSendCb)
            .stubs()
            .will(ignoreReturnValue())
            .then(ignoreReturnValue())
            .then(ignoreReturnValue())
            .then(ignoreReturnValue())
            .then(ignoreReturnValue());
    MetaModifiedCb(status, nfs, data, cbData2);

    auto cbData3 = new(nothrow) NfsSetMetaCbData();
    fileHandle.m_file->m_mode = 0;
    cbData3->fileHandle = fileHandle;
    cbData3->writeCommonData = &m_commonData;
    MetaModifiedCb(status, nfs, data, cbData3);

    auto cbData4 = new(nothrow) NfsSetMetaCbData();
    status = -BACKUP_ERR_ENOENT;
    cbData4->fileHandle = fileHandle;
    cbData4->writeCommonData = &m_commonData;
    MetaModifiedCb(status, nfs, data, cbData4);

    auto cbData5 = new(nothrow) NfsSetMetaCbData();
    status = -1;
    cbData5->fileHandle = fileHandle;
    cbData5->writeCommonData = &m_commonData;
    MetaModifiedCb(status, nfs, data, cbData5);

    auto cbData6 = new(nothrow) NfsSetMetaCbData();
    bool abort1 = true;
    m_commonData.abort = &abort1;
    cbData6->fileHandle = fileHandle;
    cbData6->writeCommonData = &m_commonData;
    MetaModifiedCb(status, nfs, data, cbData6);
}

TEST_F(SetMetaRequestTest, HandleSetMetaFailure)
{
    NfsCommonData *commonData = nullptr;
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(BackupIOEngine::LIBNFS, BackupIOEngine::LIBNFS);
    fileHandle.m_file->m_fileName = "1.txt";
    fileHandle.m_retryCnt = 0;
    int status = 0;
    struct nfs_context *nfs = nfs_init_context();
    NfsContextWrapper nfsContextWrapper(nfs);
    HandleSetMetaFailure(commonData, fileHandle, status, nfs);

    NfsCommonData *commonData1 = &m_commonData;
    status = -BACKUP_ERR_ENOENT;
    MOCKER_CPP(&SetMetaFailureHandling)
            .stubs()
            .will(ignoreReturnValue())
            .then(ignoreReturnValue())
            .then(ignoreReturnValue());
    HandleSetMetaFailure(commonData1, fileHandle, status, nfs);

    status = -EINTR;
    HandleSetMetaFailure(commonData1, fileHandle, status, nfs);

    fileHandle.m_retryCnt = 11;
    HandleSetMetaFailure(commonData1, fileHandle, status, nfs);

    status = -1;
    nfs_get_error(nfs);
    HandleSetMetaFailure(commonData1, fileHandle, status, nfs);
}

TEST_F(SetMetaRequestTest, SetMetaFailureHandling)
{
    auto commonData = nullptr;
    int status;
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(BackupIOEngine::LIBNFS, BackupIOEngine::LIBNFS);
    fileHandle.m_file->m_fileName = "1.txt";
    fileHandle.m_file->m_scannermode = "mm";
    fileHandle.m_file->ClearFlag(IS_DIR);
    MOCKER_CPP(&FileDesc::SetSrcState)
            .stubs()
            .will(returnValue(FileDescState::INIT));
    MOCKER_CPP(&FileDesc::SetDstState)
            .stubs()
            .will(returnValue(FileDescState::INIT));

    SetMetaFailureHandling(commonData, status, fileHandle);

    NfsCommonData *commonData1 = &m_commonData;
    status = -ERANGE;
    SetMetaFailureHandling(commonData1, status, fileHandle);

    status = -EACCES;
    SetMetaFailureHandling(commonData1, status, fileHandle);

    status = -1;
    fileHandle.m_file->SetFlag(IS_DIR);
    SetMetaFailureHandling(commonData1, status, fileHandle);
}

TEST_F(SetMetaRequestTest, SendDstCloseFile)
{
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(BackupIOEngine::LIBNFS, BackupIOEngine::LIBNFS);
    NfsCommonData *commonData = &m_commonData;
    MOCKER_CPP(&FileDesc::IsFlagSet)
            .stubs()
            .will(returnValue(true))
            .then(returnValue(false))
            .then(returnValue(false));
    EXPECT_EQ(SendDstCloseFile(fileHandle, commonData), MP_SUCCESS);

    fileHandle.m_file->dstIOHandle.nfsFh = nullptr;
    EXPECT_EQ(SendDstCloseFile(fileHandle, commonData), MP_FAILED);

    fileHandle.m_file->dstIOHandle.nfsFh = (struct nfsfh*) malloc(sizeof(struct nfsfh));
    MOCKER_CPP(&SendNfsRequest)
            .stubs()
            .will(returnValue(1));
    MOCKER_CPP(&FileDesc::SetSrcState)
            .stubs()
            .will(returnValue(FileDescState::INIT));
    MOCKER_CPP(&FileDesc::SetDstState)
            .stubs()
            .will(returnValue(FileDescState::INIT));
    EXPECT_EQ(SendDstCloseFile(fileHandle, commonData), MP_FAILED);

    MOCKER_CPP(&CreateCloseCbData)
            .stubs()
            .will(invoke(CbData_Stub));
    MOCKER_CPP(&Libnfscommonmethods::FreeNfsFh)
            .stubs()
            .will(ignoreReturnValue());
    EXPECT_EQ(SendDstCloseFile(fileHandle, commonData), MP_FAILED);
}