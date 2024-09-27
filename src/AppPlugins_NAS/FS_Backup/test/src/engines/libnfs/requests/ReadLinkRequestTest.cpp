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
#include "ReadLinkRequest.h"
#include "LibnfsCopyReader.h"

using namespace std;
using namespace FS_Backup;
using namespace Module;

namespace  {
    const int MP_SUCCESS = 0;
    const int MP_FAILED = 1;
}

class ReadLinkRequestTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    NfsCommonData m_commonData {};
    NfsContextContainer m_nfsContextContainer;
    BackupTimer m_timer;
};

void ReadLinkRequestTest::SetUp()
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
    m_commonData.IsResumeSendCb = LibnfsCopyReader::IsResumeSendCb;
    m_commonData.ResumeSendCb = LibnfsCopyReader::ResumeSendCb;
    m_commonData.writeQueue = std::make_shared<BackupQueue<FileHandle>>(config);
    m_commonData.aggregateQueue = std::make_shared<BackupQueue<FileHandle>>(config);
}

void ReadLinkRequestTest::TearDown()
{
    GlobalMockObject::verify(); // 校验mock规范并清除mock规范
}

void ReadLinkRequestTest::SetUpTestCase()
{}

void ReadLinkRequestTest::TearDownTestCase()
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

TEST_F(ReadLinkRequestTest, CreateReadlinkCbData)
{
    FileHandle fileHandle {};
    NfsCommonData commonData {};
    std::shared_ptr<BlockBufferMap> blockBufferMap = std::make_shared<BlockBufferMap>();
    CreateReadlinkCbData(fileHandle, commonData, blockBufferMap);
}

TEST_F(ReadLinkRequestTest, SendReadlink)
{
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(BackupIOEngine::LIBNFS, BackupIOEngine::LIBNFS);
    fileHandle.m_file->m_fileName = "1.txt";

    NfsReadlinkCbData *cbData = nullptr;
    int ret = SendReadlink(fileHandle, cbData);
    EXPECT_EQ(ret, MP_FAILED);

    NfsReadlinkCbData *cbData1 = new(nothrow) NfsReadlinkCbData();
    cbData1->fileHandle = fileHandle;
    cbData1->commonData = &m_commonData;

    MOCKER_CPP(&NfsContextWrapper::NfsReadLinkAsync)
            .stubs()
            .will(returnValue(0))
            .then(returnValue(1));
    EXPECT_EQ(SendReadlink(fileHandle, cbData1), MP_SUCCESS);

    MOCKER_CPP(&NfsContextWrapper::NfsGetError)
            .stubs()
            .will(invoke(NfsGetError_Stub));
    EXPECT_EQ(SendReadlink(fileHandle, cbData1), MP_FAILED);
}

TEST_F(ReadLinkRequestTest, SendReadlinkCb)
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
    SendReadlinkCb(status, nfs, data, privateData);

    auto cbData = new(nothrow) NfsReadlinkCbData();
    cbData->fileHandle = fileHandle;
    cbData->commonData = nullptr;
    SendReadlinkCb(status, nfs, data, cbData);

    auto cbData2 = new(nothrow) NfsReadlinkCbData();
    status = 0;
    cbData2->fileHandle = fileHandle;
    cbData2->commonData = &m_commonData;
    MOCKER_CPP(&LibnfsCopyReader::IsResumeSendCb)
            .stubs()
            .will(returnValue(true))
            .then(returnValue(true))
            .then(returnValue(true))
            .then(returnValue(true))
            .then(returnValue(true));
    MOCKER_CPP(&LibnfsCopyReader::ResumeSendCb)
            .stubs()
            .will(ignoreReturnValue())
            .then(ignoreReturnValue())
            .then(ignoreReturnValue())
            .then(ignoreReturnValue())
            .then(ignoreReturnValue());
    MOCKER_CPP(&HandleReadlinkSuccess)
            .stubs()
            .will(ignoreReturnValue());
    SendReadlinkCb(status, nfs, data, cbData2);

    auto cbData5 = new(nothrow) NfsReadlinkCbData();
    status = -1;
    cbData5->fileHandle = fileHandle;
    cbData5->commonData = &m_commonData;
    MOCKER_CPP(&HandleReadlinkFailure)
            .stubs()
            .will(ignoreReturnValue());
    nfs_get_error(nfs);
    SendReadlinkCb(status, nfs, data, cbData5);

    auto cbData3 = new(nothrow) NfsReadlinkCbData();
    status = -EINTR;
    cbData3->fileHandle = fileHandle;
    cbData3->commonData = &m_commonData;

    SendReadlinkCb(status, nfs, data, cbData3);

    auto cbData4 = new(nothrow) NfsReadlinkCbData();
    status = -EINTR;
    fileHandle.m_retryCnt = 11;
    cbData4->fileHandle = fileHandle;
    cbData4->commonData = &m_commonData;
    MOCKER_CPP(&HandleReadlinkFailure)
            .stubs()
            .will(ignoreReturnValue());
    SendReadlinkCb(status, nfs, data, cbData4);

    auto cbData6 = new(nothrow) NfsReadlinkCbData();
    bool abort1 = true;
    status = 0;
    m_commonData.abort = &abort1;
    cbData6->fileHandle = fileHandle;
    cbData6->commonData = &m_commonData;
    SendReadlinkCb(status, nfs, data, cbData6);
}

TEST_F(ReadLinkRequestTest, HandleReadlinkSuccess)
{
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(BackupIOEngine::LIBNFS, BackupIOEngine::LIBNFS);
    make_unique<uint8_t []>(8192);
    m_commonData.writeDisable = false;
    std::shared_ptr<BlockBufferMap> blockBufferMap = std::make_shared<BlockBufferMap>();
    auto cbData = new(nothrow) NfsReadlinkCbData();
    cbData->fileHandle = fileHandle;
    cbData->commonData = &m_commonData;
    cbData->blockBufferMap = blockBufferMap;
    std::string targetPath = "file1";
    void *data = (char*) targetPath.c_str();
    HandleReadlinkSuccess(cbData, fileHandle, data);

    m_commonData.writeDisable = true;

    auto cbData1 = new(nothrow) NfsReadlinkCbData();
    cbData1->fileHandle = fileHandle;
    cbData1->commonData = &m_commonData;
    cbData1->blockBufferMap = blockBufferMap;
    HandleReadlinkSuccess(cbData1, fileHandle, data);
}

TEST_F(ReadLinkRequestTest, HandleReadlinkFailure)
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
    HandleReadlinkFailure(commonData, status, fileHandle);

    NfsCommonData *commonData1 = &m_commonData;
    status = -EACCES;
    HandleReadlinkFailure(commonData1, status, fileHandle);

    status = -1;
    HandleReadlinkFailure(commonData1, status, fileHandle);
}