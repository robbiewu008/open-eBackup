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
#include "ReadRequest.h"
#include "LibnfsCopyReader.h"

using namespace std;
using namespace FS_Backup;
using namespace Module;

namespace  {
    const int MP_SUCCESS = 0;
    const int MP_FAILED = 1;
}

class ReadRequestTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    NfsCommonData m_commonData {};
    BackupTimer timer {};
    NfsContextContainer m_nfsContextContainer;
};

void ReadRequestTest::SetUp()
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
    m_commonData.IsResumeSendCb = LibnfsCopyReader::IsResumeSendCb;
    m_commonData.ResumeSendCb = LibnfsCopyReader::ResumeSendCb;
    m_commonData.hardlinkMap = make_shared<HardLinkMap>();
    m_commonData.abort = &abort;
    m_commonData.timer = &timer;
    m_commonData.writeDisable = true;
}

void ReadRequestTest::TearDown()
{
    GlobalMockObject::verify(); // 校验mock规范并清除mock规范
}

void ReadRequestTest::SetUpTestCase()
{}

void ReadRequestTest::TearDownTestCase()
{}

static char* NfsGetError_Stub()
{
    struct nfs_context *nfsContext = nullptr;
    nfsContext = nfs_init_context();
    return nfs_get_error(nfsContext);
}

static NfsCloseCbData* CbData_Stub()
{
    NfsCloseCbData *cbData = nullptr;
    return cbData;
}

TEST_F(ReadRequestTest, CreateReadCbData)
{
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(BackupIOEngine::LIBNFS, BackupIOEngine::LIBNFS);
    NfsCommonData commonData {};
    shared_ptr<BlockBufferMap> blockBufferMap {};
    CreateReadCbData(fileHandle, commonData, blockBufferMap);
}

TEST_F(ReadRequestTest, SendRead)
{
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(BackupIOEngine::LIBNFS, BackupIOEngine::LIBNFS);
    fileHandle.m_file->m_fileName = "1.txt";
    fileHandle.m_file->m_onlyFileName = "d1";
    fileHandle.m_file->m_mode = 0;
    fileHandle.m_file->m_dirName = "/d1";

    NfsReadCbData *cbData = nullptr;
    int ret = SendRead(fileHandle, cbData);
    EXPECT_EQ(ret, MP_FAILED);

    cbData = new(nothrow) NfsReadCbData();
    cbData->fileHandle = fileHandle;
    cbData->commonData = &m_commonData;
    MOCKER_CPP(&NfsContextWrapper::NfsReadFileAsync)
            .stubs()
            .will(returnValue(0))
            .then(returnValue(1));
    EXPECT_EQ(SendRead(fileHandle, cbData), MP_SUCCESS);

    MOCKER_CPP(&NfsContextWrapper::NfsGetError)
            .stubs()
            .will(invoke(NfsGetError_Stub));
    EXPECT_EQ(SendRead(fileHandle, cbData), MP_FAILED);
}

TEST_F(ReadRequestTest, SendReadCb)
{
    int status = 0;
    struct nfs_context *nfs = nfs_init_context();
    NfsContextWrapper nfsContextWrapper(nfs);
    void *data = (struct nfsfh*) malloc(sizeof(struct nfsfh));
    void *privateData = nullptr;

    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(BackupIOEngine::LIBNFS, BackupIOEngine::LIBNFS);

    SendReadCb(status, nfs, data, privateData);

    auto cbData = new(nothrow) NfsReadCbData();
    cbData->fileHandle = fileHandle;
    cbData->commonData = &m_commonData;
    cbData->blockBufferMap = make_shared<BlockBufferMap>();

    MOCKER_CPP(&LibnfsCopyReader::IsResumeSendCb)
            .stubs()
            .will(returnValue(true))
            .then(returnValue(true))
            .then(returnValue(true))
            .then(returnValue(true));
    MOCKER_CPP(&LibnfsCopyReader::ResumeSendCb)
            .stubs()
            .will(ignoreReturnValue())
            .then(ignoreReturnValue())
            .then(ignoreReturnValue())
            .then(ignoreReturnValue());
    MOCKER_CPP(&HandleReadSuccess)
            .stubs()
            .will(ignoreReturnValue());
    SendReadCb(status, nfs, data, cbData);

    auto cbData1 = new(nothrow) NfsReadCbData();
    status = -1;
    fileHandle.m_file->SetSrcState(FileDescState::INIT);
    fileHandle.m_file->SetDstState(FileDescState::INIT);
    cbData1->fileHandle = fileHandle;
    cbData1->commonData = &m_commonData;
    cbData1->blockBufferMap = make_shared<BlockBufferMap>();

    MOCKER_CPP(&HandleReadFailure)
            .stubs()
            .will(ignoreReturnValue());
    SendReadCb(status, nfs, data, cbData1);

    auto cbData2 = new(nothrow) NfsReadCbData();
    status = 0;
    fileHandle.m_file->SetSrcState(FileDescState::READ_FAILED);
    fileHandle.m_file->SetDstState(FileDescState::WRITE_FAILED);
    fileHandle.m_file->m_blockStats.m_readRespCnt = 1;
    fileHandle.m_file->m_blockStats.m_readReqCnt = 1;
    cbData2->fileHandle = fileHandle;
    cbData2->commonData = &m_commonData;
    cbData2->blockBufferMap = make_shared<BlockBufferMap>();

    MOCKER_CPP(&SendCloseFile)
            .stubs()
            .will(returnValue(MP_SUCCESS));
    SendReadCb(status, nfs, data, cbData2);

    auto cbData3 = new(nothrow) NfsReadCbData();
    status = 0;
    bool abort = true;
    m_commonData.abort = &abort;
    cbData3->fileHandle = fileHandle;
    cbData3->commonData = &m_commonData;
    cbData3->blockBufferMap = make_shared<BlockBufferMap>();

    MOCKER_CPP(&Libnfscommonmethods::FreeNfsFh)
            .stubs()
            .will(ignoreReturnValue());
    SendReadCb(status, nfs, data, cbData3);
}

TEST_F(ReadRequestTest, HandleReadSuccess)
{
    int status = 0;
    void *data = (struct nfsfh*) malloc(sizeof(struct nfsfh));
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(BackupIOEngine::LIBNFS, BackupIOEngine::LIBNFS);
    NfsReadCbData *cbData = nullptr;

    HandleReadSuccess(cbData, fileHandle, data, status);

    auto cbData1 = new(nothrow) NfsReadCbData();
    cbData1->fileHandle = fileHandle;
    cbData1->commonData = nullptr;
    cbData1->blockBufferMap = make_shared<BlockBufferMap>();

    HandleReadSuccess(cbData1, fileHandle, data, status);

    auto cbData2 = new(nothrow) NfsReadCbData();
    cbData2->fileHandle = fileHandle;
    cbData2->commonData = &m_commonData;
    cbData2->blockBufferMap = make_shared<BlockBufferMap>();

    MOCKER_CPP(&memcpy_s)
            .stubs()
            .will(returnValue(1))
            .then(returnValue(0))
            .then(returnValue(0));
    MOCKER_CPP(&ReadFailureHandling)
            .stubs()
            .will(ignoreReturnValue());
    HandleReadSuccess(cbData2, fileHandle, data, status);

    auto cbData3 = new(nothrow) NfsReadCbData();
    fileHandle.m_file->m_blockStats.m_readRespCnt = 1;
    fileHandle.m_file->m_blockStats.m_totalCnt = 1;
    cbData3->fileHandle = fileHandle;
    cbData3->commonData = &m_commonData;
    cbData3->blockBufferMap = make_shared<BlockBufferMap>();
    MOCKER_CPP(&Libnfscommonmethods::FreeNfsFh)
            .stubs()
            .will(ignoreReturnValue());
    MOCKER_CPP(&SendCloseFile)
            .stubs()
            .will(returnValue(MP_SUCCESS));
    HandleReadSuccess(cbData3, fileHandle, data, status);

    auto cbData4 = new(nothrow) NfsReadCbData();
    m_commonData.writeDisable = false;
    cbData4->fileHandle = fileHandle;
    cbData4->commonData = &m_commonData;
    cbData4->blockBufferMap = make_shared<BlockBufferMap>();
    HandleReadSuccess(cbData4, fileHandle, data, status);
}

TEST_F(ReadRequestTest, HandleReadFailure)
{
    NfsReadCbData *cbData = nullptr;
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(BackupIOEngine::LIBNFS, BackupIOEngine::LIBNFS);
    int status = 0;
    struct nfs_context *nfs = nfs_init_context();

    HandleReadFailure(cbData, fileHandle, status, nfs);

    auto cbData1 = new(nothrow) NfsReadCbData();
    cbData1->fileHandle = fileHandle;
    cbData1->commonData = nullptr;
    cbData1->blockBufferMap = make_shared<BlockBufferMap>();

    HandleReadFailure(cbData1, fileHandle, status, nfs);

    auto cbData2 = new(nothrow) NfsReadCbData();
    status = -EINTR;
    fileHandle.m_retryCnt = 0;
    cbData2->fileHandle = fileHandle;
    cbData2->commonData = &m_commonData;
    cbData2->blockBufferMap = make_shared<BlockBufferMap>();

    HandleReadFailure(cbData2, fileHandle, status, nfs);

    status = -EINTR;
    fileHandle.m_retryCnt = 11;
    cbData2->fileHandle = fileHandle;

    MOCKER_CPP(&ReadFailureHandling)
            .stubs()
            .will(ignoreReturnValue())
            .then(ignoreReturnValue());
    nfs_get_error(nfs);
    HandleReadFailure(cbData2, fileHandle, status, nfs);

    status = -1;
    nfs_get_error(nfs);
    HandleReadFailure(cbData2, fileHandle, status, nfs);
}

TEST_F(ReadRequestTest, ReadFailureHandling)
{
    NfsReadCbData *cbData = nullptr;
    int status = 0;
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(BackupIOEngine::LIBNFS, BackupIOEngine::LIBNFS);

    ReadFailureHandling(cbData, status, fileHandle);

    auto cbData1 = new(nothrow) NfsReadCbData();
    cbData1->fileHandle = fileHandle;
    cbData1->commonData = nullptr;
    cbData1->blockBufferMap = make_shared<BlockBufferMap>();

    ReadFailureHandling(cbData1, status, fileHandle);

    status = -EACCES;
    fileHandle.m_file->SetSrcState(FileDescState::INIT);
    fileHandle.m_file->SetDstState(FileDescState::INIT);
    m_commonData.skipFailure = false;

    cbData1->fileHandle = fileHandle;
    cbData1->commonData = &m_commonData;

    MOCKER_CPP(&SendCloseFile)
            .stubs()
            .will(returnValue(MP_SUCCESS))
            .then(returnValue(MP_SUCCESS));
    ReadFailureHandling(cbData1, status, fileHandle);

    status = -1;
    ReadFailureHandling(cbData1, status, fileHandle);
}

TEST_F(ReadRequestTest, SendCloseFile)
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
    EXPECT_EQ(SendCloseFile(fileHandle, commonData), MP_SUCCESS);

    fileHandle.m_file->srcIOHandle.nfsFh = nullptr;
    EXPECT_EQ(SendCloseFile(fileHandle, commonData), MP_FAILED);

    fileHandle.m_file->srcIOHandle.nfsFh = (struct nfsfh*) malloc(sizeof(struct nfsfh));
    MOCKER_CPP(&SendNfsRequest)
            .stubs()
            .will(returnValue(MP_FAILED));
    EXPECT_EQ(SendCloseFile(fileHandle, commonData), MP_FAILED);

    fileHandle.m_file->srcIOHandle.nfsFh = (struct nfsfh*) malloc(sizeof(struct nfsfh));
    MOCKER_CPP(&CreateCloseCbData)
            .stubs()
            .will(invoke(CbData_Stub));
    MOCKER_CPP(&Libnfscommonmethods::FreeNfsFh)
            .stubs()
            .will(ignoreReturnValue());
    EXPECT_EQ(SendCloseFile(fileHandle, commonData), FAILED);
}
