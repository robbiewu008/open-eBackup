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
#include "SymLinkRequest.h"
#include "LibnfsCopyWriter.h"

using namespace std;
using namespace FS_Backup;
using namespace Module;

namespace  {
    const int MP_SUCCESS = 0;
    const int MP_FAILED = 1;
}

class SymLinkRequestTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    NfsCommonData m_commonData {};
    NfsContextContainer m_nfsContextContainer;
    BackupTimer m_timer;
};

void SymLinkRequestTest::SetUp()
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

void SymLinkRequestTest::TearDown()
{
    GlobalMockObject::verify(); // 校验mock规范并清除mock规范
}

void SymLinkRequestTest::SetUpTestCase()
{}

void SymLinkRequestTest::TearDownTestCase()
{}

static char* NfsGetError_Stub()
{
    struct nfs_context *nfsContext = nullptr;
    nfsContext = nfs_init_context();
    return nfs_get_error(nfsContext);
}

TEST_F(SymLinkRequestTest, CreateSymLinkCbData)
{
    FileHandle fileHandle {};
    NfsCommonData commonData {};
    struct nfsfh* nfsfh {};
    std::shared_ptr<BlockBufferMap> blockBufferMap = std::make_shared<BlockBufferMap>();
    CreateSymLinkCbData(fileHandle, commonData, nfsfh, blockBufferMap);
}

TEST_F(SymLinkRequestTest, SendSymLink)
{
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(BackupIOEngine::LIBNFS, BackupIOEngine::LIBNFS);
    fileHandle.m_file->m_fileName = "1.txt";

    NfsSymLinkCbData *cbData = nullptr;
    int ret = SendSymLink(fileHandle, cbData);
    EXPECT_EQ(ret, MP_FAILED);

    std::string targetStr = "targetPath";
    fileHandle.m_block.m_buffer = new uint8_t[targetStr.length()];
    ret = memcpy_s(fileHandle.m_block.m_buffer, targetStr.length(), (const char*)targetStr.c_str(),
        targetStr.length());
    EXPECT_EQ(ret, MP_SUCCESS);

    NfsSymLinkCbData *cbData1 = new(nothrow) NfsSymLinkCbData();
    cbData1->fileHandle = fileHandle;
    cbData1->writeCommonData = &m_commonData;
    cbData1->nfsfh = (struct nfsfh*) malloc(sizeof(struct nfsfh));

    MOCKER_CPP(&NfsContextWrapper::NfsSymLinkAsync)
            .stubs()
            .will(returnValue(0))
            .then(returnValue(1));
    EXPECT_EQ(SendSymLink(fileHandle, cbData1), MP_SUCCESS);

    NfsSymLinkCbData *cbData2 = new(nothrow) NfsSymLinkCbData();
    cbData2->fileHandle = fileHandle;
    cbData2->writeCommonData = &m_commonData;
    cbData2->nfsfh = (struct nfsfh*) malloc(sizeof(struct nfsfh));

    MOCKER_CPP(&NfsContextWrapper::NfsGetError)
            .stubs()
            .will(invoke(NfsGetError_Stub));
    EXPECT_EQ(SendSymLink(fileHandle, cbData2), MP_FAILED);
}

TEST_F(SymLinkRequestTest, SendSymLinkCb)
{
    int status = 0;
    struct nfs_context *nfs = nfs_init_context();
    NfsContextWrapper nfsContextWrapper(nfs);
    void *data = (struct nfsfh*) malloc(sizeof(struct nfsfh));
    void *privateData = nullptr;
    std::shared_ptr<BlockBufferMap> blockBufferMap = std::make_shared<BlockBufferMap>();

    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(BackupIOEngine::LIBNFS, BackupIOEngine::LIBNFS);
    fileHandle.m_file->m_inode = 10456;

    SendSymLinkCb(status, nfs, data, privateData);

    auto cbData = new(nothrow) NfsSymLinkCbData();
    cbData->fileHandle = fileHandle;
    cbData->writeCommonData = nullptr;
    cbData->nfsfh = nullptr;
    cbData->blockBufferMap = blockBufferMap;
    SendSymLinkCb(status, nfs, data, cbData);

    auto cbData1 = new(nothrow) NfsSymLinkCbData();
    cbData1->fileHandle = fileHandle;
    cbData1->writeCommonData = &m_commonData;
    cbData1->nfsfh = (struct nfsfh*) malloc(sizeof(struct nfsfh));
    cbData1->blockBufferMap = blockBufferMap;
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
    SendSymLinkCb(status, nfs, data, cbData1);

    auto cbData2 = new(nothrow) NfsSymLinkCbData();
    status = -1;
    cbData2->fileHandle = fileHandle;
    cbData2->writeCommonData = &m_commonData;
    cbData2->nfsfh = (struct nfsfh*) malloc(sizeof(struct nfsfh));
    cbData2->blockBufferMap = blockBufferMap;
    MOCKER_CPP(&HandleSymLinkFailure)
            .stubs()
            .will(ignoreReturnValue());
    SendSymLinkCb(status, nfs, data, cbData2);

    auto cbData3 = new(nothrow) NfsSymLinkCbData();
    bool abort1 = true;
    status = 0;
    m_commonData.abort = &abort1;
    cbData3->fileHandle = fileHandle;
    cbData3->writeCommonData = &m_commonData;
    cbData3->nfsfh = (struct nfsfh*) malloc(sizeof(struct nfsfh));
    cbData3->blockBufferMap = blockBufferMap;
    SendSymLinkCb(status, nfs, data, cbData3);
}

TEST_F(SymLinkRequestTest, HandleSymLinkFailure)
{
    NfsCommonData *commonData = nullptr;
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(BackupIOEngine::LIBNFS, BackupIOEngine::LIBNFS);
    fileHandle.m_file->m_fileName = "1.txt";
    fileHandle.m_retryCnt = 0;
    int status = 0;
    struct nfs_context *nfs = nfs_init_context();
    NfsContextWrapper nfsContextWrapper(nfs);
    HandleSymLinkFailure(commonData, fileHandle, status, nfs);

    NfsCommonData *commonData1 = &m_commonData;
    status = -BACKUP_ERR_ENOENT;
    HandleSymLinkFailure(commonData1, fileHandle, status, nfs);

    status = -BACKUP_ERR_NOTDIR;
    MOCKER_CPP(&SymlinkFailureHandling)
            .stubs()
            .will(ignoreReturnValue());
    HandleSymLinkFailure(commonData1, fileHandle, status, nfs);

    status = -BACKUP_ERR_EEXIST;
    HandleSymLinkFailure(commonData1, fileHandle, status, nfs);

    status = -EINTR;
    HandleSymLinkFailure(commonData1, fileHandle, status, nfs);

    status = -EINTR;
    fileHandle.m_retryCnt = 11;
    MOCKER_CPP(&SymlinkFailureHandling)
            .stubs()
            .will(ignoreReturnValue());
    HandleSymLinkFailure(commonData1, fileHandle, status, nfs);

    status = -BACKUP_ERR_EISDIR;
    HandleSymLinkFailure(commonData1, fileHandle, status, nfs);

    status = -1;
    nfs_get_error(nfs);
    MOCKER_CPP(&SymlinkFailureHandling)
            .stubs()
            .will(ignoreReturnValue());
    HandleSymLinkFailure(commonData1, fileHandle, status, nfs);
}

TEST_F(SymLinkRequestTest, SymlinkFailureHandling)
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
    SymlinkFailureHandling(commonData, status, fileHandle);

    NfsCommonData *commonData1 = &m_commonData;
    status = -ERANGE;
    SymlinkFailureHandling(commonData1, status, fileHandle);

    status = -EACCES;
    SymlinkFailureHandling(commonData1, status, fileHandle);

    status = -1;
    SymlinkFailureHandling(commonData1, status, fileHandle);
}
