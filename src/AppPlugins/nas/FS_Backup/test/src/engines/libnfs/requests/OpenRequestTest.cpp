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
#include "OpenRequest.h"
#include "LibnfsCopyWriter.h"

using namespace std;
using namespace FS_Backup;
using namespace Module;

namespace  {
    const int MP_SUCCESS = 0;
    const int MP_FAILED = 1;
}

class OpenRequestTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    NfsCommonData m_commonData {};
    NfsContextContainer m_nfsContextContainer;
    BackupTimer m_timer;
};

void OpenRequestTest::SetUp()
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
}

void OpenRequestTest::TearDown()
{
    GlobalMockObject::verify(); // 校验mock规范并清除mock规范
}

void OpenRequestTest::SetUpTestCase()
{}

void OpenRequestTest::TearDownTestCase()
{}

static char* NfsGetError_Stub()
{
    struct nfs_context *nfsContext = nullptr;
    nfsContext = nfs_init_context();
    return nfs_get_error(nfsContext);
}

TEST_F(OpenRequestTest, CreateOpenCbData)
{
    FileHandle fileHandle {};
    NfsCommonData commonData {};
    CreateOpenCbData(fileHandle, commonData);
}

TEST_F(OpenRequestTest, SendOpen)
{
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(BackupIOEngine::LIBNFS, BackupIOEngine::LIBNFS);
    fileHandle.m_file->m_fileName = "1.txt";

    NfsOpenCbData *cbData = nullptr;
    EXPECT_EQ(SendOpen(fileHandle, cbData), MP_FAILED);

    NfsOpenCbData *cbData1 = new(nothrow) NfsOpenCbData();
    cbData1->fileHandle = fileHandle;
    cbData1->commonData = &m_commonData;

    MOCKER_CPP(&NfsContextWrapper::NfsOpenAsync)
            .stubs()
            .will(returnValue(0))
            .then(returnValue(1));
    EXPECT_EQ(SendOpen(fileHandle, cbData1), MP_SUCCESS);

    MOCKER_CPP(&NfsContextWrapper::NfsGetError)
            .stubs()
            .will(invoke(NfsGetError_Stub));
    EXPECT_EQ(SendOpen(fileHandle, cbData1), MP_FAILED);
}

TEST_F(OpenRequestTest, SendOpenCb)
{
    int status = 0;
    struct nfs_context *nfs = nfs_init_context();
    NfsContextWrapper nfsContextWrapper(nfs);
    void *data = (struct nfsfh*) malloc(sizeof(struct nfsfh));
    void *privateData = nullptr;

    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(BackupIOEngine::LIBNFS, BackupIOEngine::LIBNFS);
    fileHandle.m_file->m_inode = 10456;
    SendOpenCb(status, nfs, data, privateData);

    auto cbData = new(nothrow) NfsOpenCbData();
    cbData->fileHandle = fileHandle;
    cbData->commonData = nullptr;
    SendOpenCb(status, nfs, data, cbData);

    auto cbData2 = new(nothrow) NfsOpenCbData();
    cbData2->fileHandle = fileHandle;
    cbData2->commonData = &m_commonData;
    
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
    SendOpenCb(status, nfs, data, cbData2);

    auto cbData1 = new(nothrow) NfsOpenCbData();
    status = -1;
    cbData1->fileHandle = fileHandle;
    cbData1->commonData = &m_commonData;
    MOCKER_CPP(&HandleOpenFailure)
            .stubs()
            .will(ignoreReturnValue());
    SendOpenCb(status, nfs, data, cbData1);

    auto cbData3 = new(nothrow) NfsOpenCbData();
    bool abort1 = true;
    status = 0;
    m_commonData.abort = &abort1;
    cbData3->fileHandle = fileHandle;
    cbData3->commonData = &m_commonData;
    SendOpenCb(status, nfs, data, cbData3);
}

TEST_F(OpenRequestTest, HandleOpenFailure)
{
    NfsCommonData *commonData = nullptr;
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(BackupIOEngine::LIBNFS, BackupIOEngine::LIBNFS);
    fileHandle.m_file->m_fileName = "1.txt";
    fileHandle.m_retryCnt = 0;
    int status = 0;
    struct nfs_context *nfs = nfs_init_context();
    NfsContextWrapper nfsContextWrapper(nfs);
    HandleOpenFailure(commonData, status, nfs, fileHandle);

    NfsCommonData *commonData1 = &m_commonData;
    status = -EINTR;
    HandleOpenFailure(commonData1, status, nfs, fileHandle);

    fileHandle.m_retryCnt = 11;
    MOCKER_CPP(&OpenFailureHandling)
            .stubs()
            .will(ignoreReturnValue());
    HandleOpenFailure(commonData1, status, nfs, fileHandle);

    status = -1;
    MOCKER_CPP(&OpenFailureHandling)
            .stubs()
            .will(ignoreReturnValue());
    HandleOpenFailure(commonData1, status, nfs, fileHandle);
}

TEST_F(OpenRequestTest, OpenFailureHandling)
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
            .then(returnValue(0));
    OpenFailureHandling(commonData, status, fileHandle);

    NfsCommonData *commonData1 = &m_commonData;
    status = -ERANGE;
    OpenFailureHandling(commonData1, status, fileHandle);

    status = -EACCES;
    OpenFailureHandling(commonData1, status, fileHandle);
}
