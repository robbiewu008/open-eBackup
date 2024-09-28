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
#include "WriteRequest.h"
#include "LibnfsCopyWriter.h"

using namespace std;
using namespace FS_Backup;
using namespace Module;

namespace  {
    const int MP_SUCCESS = 0;
    const int MP_FAILED = 1;
}

class WriteRequestTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    NfsCommonData m_commonData {};
    BackupTimer timer {};
    NfsContextContainer m_nfsContextContainer;
};

void WriteRequestTest::SetUp()
{
    BackupQueueConfig config {DEFAULT_BACKUP_QUEUE_SIZE, DEFAULT_BACKUP_QUEUE_MEMORY_SIZE};
    bool abort = false;
    m_commonData.nfsContextContainer = &m_nfsContextContainer;
    m_commonData.syncNfsContextContainer = &m_nfsContextContainer;
    m_commonData.pktStats = make_shared<PacketStats>();
    m_commonData.writeQueue = std::make_shared<BackupQueue<FileHandle>>(config);
    m_commonData.aggregateQueue = std::make_shared<BackupQueue<FileHandle>>(config);
    m_commonData.controlInfo = std::make_shared<BackupControlInfo>();
    m_commonData.commonObj = nullptr;
    m_commonData.IsResumeSendCb = LibnfsCopyWriter::IsResumeSendCb;
    m_commonData.ResumeSendCb = LibnfsCopyWriter::ResumeSendCb;
    m_commonData.hardlinkMap = make_shared<HardLinkMap>();
    m_commonData.abort = &abort;
    m_commonData.timer = &timer;
    m_commonData.writeDisable = true;
}

void WriteRequestTest::TearDown()
{
    GlobalMockObject::verify(); // 校验mock规范并清除mock规范
}

void WriteRequestTest::SetUpTestCase()
{}

void WriteRequestTest::TearDownTestCase()
{}

static char* NfsGetError_Stub()
{
    struct nfs_context *nfsContext = nullptr;
    nfsContext = nfs_init_context();
    return nfs_get_error(nfsContext);
}

static NfsWriteCbData* CbData_Stub()
{
    NfsWriteCbData *cbData = nullptr;
    return cbData;
}

static NfsSetMetaCbData* CbData_Stub1()
{
    NfsSetMetaCbData *cbData = nullptr;
    return cbData;
}

static NfsCloseCbData* CbData_Stub2()
{
    NfsCloseCbData *cbData = nullptr;
    return cbData;
}

TEST_F(WriteRequestTest, CreateWriteCbData)
{
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(BackupIOEngine::LIBNFS, BackupIOEngine::LIBNFS);
    NfsCommonData commonData {};
    shared_ptr<BlockBufferMap> blockBufferMap {};
    CreateWriteCbData(fileHandle, commonData, blockBufferMap);
}

TEST_F(WriteRequestTest, SendWrite)
{
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(BackupIOEngine::LIBNFS, BackupIOEngine::LIBNFS);
    fileHandle.m_file->m_fileName = "1.txt";
    fileHandle.m_file->m_onlyFileName = "d1";
    fileHandle.m_file->m_mode = 0;
    fileHandle.m_file->m_dirName = "/d1";

    NfsWriteCbData *cbData = nullptr;
    int ret = SendWrite(fileHandle, cbData);
    EXPECT_EQ(ret, MP_FAILED);

    cbData = new(nothrow) NfsWriteCbData();
    fileHandle.m_file->m_blockStats.m_writeReqCnt = 1;
    fileHandle.m_file->SetDstState(FileDescState::INIT);
    cbData->fileHandle = fileHandle;
    cbData->writeCommonData = &m_commonData;
    MOCKER_CPP(&NfsContextWrapper::NfsFsyncAsync)
            .stubs()
            .will(returnValue(0))
            .then(returnValue(1));
    MOCKER_CPP(&NfsContextWrapper::NfsWriteAsync)
            .stubs()
            .will(returnValue(0))
            .then(returnValue(1));
    EXPECT_EQ(SendWrite(fileHandle, cbData), MP_SUCCESS);
    cbData = new(nothrow) NfsWriteCbData();
    cbData->fileHandle = fileHandle;
    cbData->writeCommonData = &m_commonData;
    EXPECT_EQ(SendWrite(fileHandle, cbData), MP_FAILED);
    cbData = new(nothrow) NfsWriteCbData();
    cbData->fileHandle = fileHandle;
    cbData->writeCommonData = &m_commonData;
    MOCKER_CPP(&CreateWriteCbData)
            .stubs()
            .will(invoke(CbData_Stub));
    EXPECT_EQ(SendWrite(fileHandle, cbData), MP_FAILED);
    cbData = new(nothrow) NfsWriteCbData();
    fileHandle.m_file->m_blockStats.m_writeRespCnt = 1;
    cbData->fileHandle = fileHandle;
    cbData->writeCommonData = &m_commonData;
    EXPECT_EQ(SendWrite(fileHandle, cbData), MP_FAILED);
}

TEST_F(WriteRequestTest, SendWriteCb)
{
    int status = 0;
    struct nfs_context *nfs = nfs_init_context();
    NfsContextWrapper nfsContextWrapper(nfs);
    void *data = (struct nfsfh*) malloc(sizeof(struct nfsfh));
    void *privateData = nullptr;

    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(BackupIOEngine::LIBNFS, BackupIOEngine::LIBNFS);

    SendWriteCb(status, nfs, data, privateData);

    auto cbData = new(nothrow) NfsWriteCbData();
    cbData->fileHandle = fileHandle;
    cbData->writeCommonData = nullptr;
    cbData->blockBufferMap = make_shared<BlockBufferMap>();

    SendWriteCb(status, nfs, data, cbData);

    fileHandle.m_file->SetSrcState(FileDescState::INIT);
    fileHandle.m_file->SetDstState(FileDescState::INIT);
    fileHandle.m_file->m_blockStats.m_writeRespCnt = 1;
    fileHandle.m_file->m_blockStats.m_totalCnt = 1;
    MOCKER_CPP(&LibnfsCopyWriter::IsResumeSendCb)
            .stubs()
            .will(returnValue(true))
            .then(returnValue(true))
            .then(returnValue(true))
            .then(returnValue(true));
    MOCKER_CPP(&LibnfsCopyWriter::ResumeSendCb)
            .stubs()
            .will(ignoreReturnValue())
            .then(ignoreReturnValue())
            .then(ignoreReturnValue())
            .then(ignoreReturnValue());
    //MOCKER_CPP(&SendSetMeta)
    //        .stubs()
    //        .will(returnValue(MP_SUCCESS));
    //SendWriteCb(status, nfs, data, cbData);

    auto cbData1 = new(nothrow) NfsWriteCbData();
    status = -1;
    fileHandle.m_file->SetSrcState(FileDescState::INIT);
    fileHandle.m_file->SetDstState(FileDescState::INIT);
    cbData1->fileHandle = fileHandle;
    cbData1->writeCommonData = &m_commonData;
    cbData1->blockBufferMap = make_shared<BlockBufferMap>();

    MOCKER_CPP(&HandleWriteFailure)
            .stubs()
            .will(ignoreReturnValue());
    SendWriteCb(status, nfs, data, cbData1);

    auto cbData2 = new(nothrow) NfsWriteCbData();
    status = 0;
    fileHandle.m_file->SetSrcState(FileDescState::READ_FAILED);
    fileHandle.m_file->SetDstState(FileDescState::WRITE_FAILED);
    fileHandle.m_file->m_blockStats.m_writeRespCnt = 1;
    fileHandle.m_file->m_blockStats.m_writeReqCnt = 1;
    cbData2->fileHandle = fileHandle;
    cbData2->writeCommonData = &m_commonData;
    cbData2->blockBufferMap = make_shared<BlockBufferMap>();

    SendWriteCb(status, nfs, data, cbData2);

    auto cbData3 = new(nothrow) NfsWriteCbData();
    status = 0;
    bool abort = true;
    m_commonData.abort = &abort;
    cbData3->fileHandle = fileHandle;
    cbData3->writeCommonData = &m_commonData;
    cbData3->blockBufferMap = make_shared<BlockBufferMap>();

    SendWriteCb(status, nfs, data, cbData3);
}

TEST_F(WriteRequestTest, HandleWriteFailure)
{
    int status = 0;
    void *data = (struct nfsfh*) malloc(sizeof(struct nfsfh));
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(BackupIOEngine::LIBNFS, BackupIOEngine::LIBNFS);
    NfsWriteCbData *cbData = nullptr;
    struct nfs_context *nfs = nfs_init_context();
    NfsContextWrapper nfsContextWrapper(nfs);

    HandleWriteFailure(cbData, fileHandle, status, nfs);

    auto cbData1 = new(nothrow) NfsWriteCbData();
    cbData1->fileHandle = fileHandle;
    cbData1->writeCommonData = nullptr;
    cbData1->blockBufferMap = make_shared<BlockBufferMap>();

    HandleWriteFailure(cbData1, fileHandle, status, nfs);

    auto cbData2 = new(nothrow) NfsWriteCbData();
    status = -EINTR;
    fileHandle.m_retryCnt = 0;
    cbData2->fileHandle = fileHandle;
    cbData2->writeCommonData = &m_commonData;
    cbData2->blockBufferMap = make_shared<BlockBufferMap>();

    nfs_get_error(nfs);
    HandleWriteFailure(cbData2, fileHandle, status, nfs);

    fileHandle.m_retryCnt = 11;
    cbData2->fileHandle = fileHandle;
    MOCKER_CPP(&WriteFailureHandling)
            .stubs()
            .will(ignoreReturnValue());
    nfs_get_error(nfs);
    HandleWriteFailure(cbData2, fileHandle, status, nfs);
}

TEST_F(WriteRequestTest, WriteFailureHandling)
{
    NfsCommonData *commonData = nullptr;
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(BackupIOEngine::LIBNFS, BackupIOEngine::LIBNFS);
    int status = 0;

    WriteFailureHandling(commonData, status, fileHandle);

    m_commonData.skipFailure = false;
    NfsCommonData *commonData1 = &m_commonData;
    fileHandle.m_file->SetSrcState(FileDescState::INIT);
    fileHandle.m_file->SetDstState(FileDescState::INIT);
    status = -ERANGE;
    WriteFailureHandling(commonData1, status, fileHandle);

    status = -EACCES;
    WriteFailureHandling(commonData1, status, fileHandle);

    status = -1;
    WriteFailureHandling(commonData1, status, fileHandle);
}

TEST_F(WriteRequestTest, SendFileSyncCb)
{
    int status = 0;
    struct nfs_context *nfs = nfs_init_context();
    NfsContextWrapper nfsContextWrapper(nfs);
    void *data = (struct nfsfh*) malloc(sizeof(struct nfsfh));
    void *privateData = nullptr;
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(BackupIOEngine::LIBNFS, BackupIOEngine::LIBNFS);

    SendFileSyncCb(status, nfs, data, privateData);

    auto cbData1 = new(nothrow) NfsWriteCbData();
    cbData1->fileHandle = fileHandle;
    cbData1->writeCommonData = nullptr;
    cbData1->blockBufferMap = make_shared<BlockBufferMap>();

    SendFileSyncCb(status, nfs, data, cbData1);

    auto cbData2 = new(nothrow) NfsWriteCbData();
    cbData2->fileHandle = fileHandle;
    cbData2->writeCommonData = &m_commonData;
    cbData2->blockBufferMap = make_shared<BlockBufferMap>();

    status = -1;
    MOCKER_CPP(&LibnfsCopyWriter::IsResumeSendCb)
            .stubs()
            .will(returnValue(true));
    MOCKER_CPP(&LibnfsCopyWriter::ResumeSendCb)
            .stubs()
            .will(ignoreReturnValue());
    nfs_get_error(nfs);
    SendFileSyncCb(status, nfs, data, cbData2);
}

TEST_F(WriteRequestTest, SendSetMeta)
{
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(BackupIOEngine::LIBNFS, BackupIOEngine::LIBNFS);
    m_commonData.writeMeta = false;
    NfsCommonData *commonData = &m_commonData;

    MOCKER_CPP(&SendCloseForAggregateFile)
            .stubs()
            .will(returnValue(MP_SUCCESS));
    EXPECT_EQ(SendSetMeta(fileHandle, commonData), MP_SUCCESS);

    m_commonData.writeMeta = true;
    commonData = &m_commonData;
    MOCKER_CPP(&SendNfsRequest)
            .stubs()
            .will(returnValue(MP_SUCCESS))
            .then(returnValue(MP_FAILED));
    EXPECT_EQ(SendSetMeta(fileHandle, commonData), MP_SUCCESS);

    fileHandle.m_file->SetSrcState(FileDescState::INIT);
    fileHandle.m_file->SetDstState(FileDescState::INIT);
    m_commonData.skipFailure = false;
    commonData = &m_commonData;
    EXPECT_EQ(SendSetMeta(fileHandle, commonData), MP_FAILED);

     MOCKER_CPP(&CreateSetMetaCbData)
            .stubs()
            .will(invoke(CbData_Stub1));
    EXPECT_EQ(SendSetMeta(fileHandle, commonData), MP_FAILED);
}

TEST_F(WriteRequestTest, SendCloseForAggregateFile)
{
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(BackupIOEngine::LIBNFS, BackupIOEngine::LIBNFS);
    NfsCommonData *commonData = &m_commonData;

    MOCKER_CPP(&FileDesc::IsFlagSet)
            .stubs()
            .will(returnValue(true))
            .then(returnValue(false))
            .then(returnValue(false))
            .then(returnValue(false));
    EXPECT_EQ(SendCloseForAggregateFile(fileHandle, commonData), MP_SUCCESS);

    fileHandle.m_file->dstIOHandle.nfsFh = nullptr;
    EXPECT_EQ(SendCloseForAggregateFile(fileHandle, commonData), MP_FAILED);

    fileHandle.m_file->SetSrcState(FileDescState::INIT);
    fileHandle.m_file->SetDstState(FileDescState::INIT);
    fileHandle.m_file->dstIOHandle.nfsFh = (struct nfsfh*) malloc(sizeof(struct nfsfh));
    m_commonData.skipFailure = false;
    commonData = &m_commonData;
    MOCKER_CPP(&SendNfsRequest)
            .stubs()
            .will(returnValue(MP_FAILED));
    EXPECT_EQ(SendCloseForAggregateFile(fileHandle, commonData), MP_FAILED);

    MOCKER_CPP(&CreateCloseCbData)
            .stubs()
            .will(invoke(CbData_Stub2));
    MOCKER_CPP(&Libnfscommonmethods::FreeNfsFh)
            .stubs()
            .will(ignoreReturnValue());
    EXPECT_EQ(SendCloseForAggregateFile(fileHandle, commonData), MP_FAILED);
}
