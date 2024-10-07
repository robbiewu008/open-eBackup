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
#include "CloseRequest.h"
#include "LibnfsCopyWriter.h"

using namespace std;
using namespace FS_Backup;
using namespace Module;

namespace  {
    const int MP_SUCCESS = 0;
    const int MP_FAILED = 1;
}

class CloseRequestTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    NfsCommonData m_commonData {};
    NfsContextContainer m_nfsContextContainer;
};

void CloseRequestTest::SetUp()
{
    BackupQueueConfig config {DEFAULT_BACKUP_QUEUE_SIZE, DEFAULT_BACKUP_QUEUE_MEMORY_SIZE};
    m_commonData.nfsContextContainer = &m_nfsContextContainer;
    m_commonData.syncNfsContextContainer = &m_nfsContextContainer;
    m_commonData.pktStats = make_shared<PacketStats>();
    m_commonData.writeQueue = std::make_shared<BackupQueue<FileHandle>>(config);
    m_commonData.controlInfo = std::make_shared<BackupControlInfo>();
    m_commonData.commonObj = nullptr;
    m_commonData.IsResumeSendCb = LibnfsCopyWriter::IsResumeSendCb;
    m_commonData.ResumeSendCb = LibnfsCopyWriter::ResumeSendCb;
    m_commonData.hardlinkMap = make_shared<HardLinkMap>();
}

void CloseRequestTest::TearDown()
{
    GlobalMockObject::verify(); // 校验mock规范并清除mock规范
}

void CloseRequestTest::SetUpTestCase()
{}

void CloseRequestTest::TearDownTestCase()
{}

static char* NfsGetError_Stub()
{
    struct nfs_context *nfsContext = nullptr;
    nfsContext = nfs_init_context();
    return nfs_get_error(nfsContext);
}

TEST_F(CloseRequestTest, CreateCloseCbData)
{
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(BackupIOEngine::LIBNFS, BackupIOEngine::LIBNFS);
    NfsCommonData commonData {};
    BACKUP_DIRECTION direction = BACKUP_DIRECTION::SRC;
    CreateCloseCbData(fileHandle, commonData, direction);

    direction = BACKUP_DIRECTION::DST;
    CreateCloseCbData(fileHandle, commonData, direction);
}

TEST_F(CloseRequestTest, SendClose)
{
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(BackupIOEngine::LIBNFS, BackupIOEngine::LIBNFS);
    fileHandle.m_file->m_fileName = "1.txt";
    fileHandle.m_file->m_onlyFileName = "d1";
    fileHandle.m_file->m_mode = 0;
    fileHandle.m_file->m_dirName = "/d1";

    NfsCloseCbData *cbData = nullptr;
    int ret = SendClose(fileHandle, cbData);
    EXPECT_EQ(ret, MP_FAILED);

    cbData = new(nothrow) NfsCloseCbData();
    cbData->fileHandle = fileHandle;
    cbData->commonData = &m_commonData;
    cbData->nfsFh = nullptr;

    ret = SendClose(fileHandle, cbData);
    EXPECT_EQ(ret, MP_FAILED);

    cbData->nfsFh = (struct nfsfh*) malloc(sizeof(struct nfsfh));
    MOCKER_CPP(&NfsContextWrapper::NfsCloseAsync)
            .stubs()
            .will(returnValue(0))
            .then(returnValue(1));
    EXPECT_EQ(SendClose(fileHandle, cbData), MP_SUCCESS);

    MOCKER_CPP(&NfsContextWrapper::NfsGetError)
            .stubs()
            .will(invoke(NfsGetError_Stub));
    MOCKER_CPP(&Libnfscommonmethods::FreeNfsFh)
            .stubs()
            .will(ignoreReturnValue());
    MOCKER_CPP(&ProcessCloseCompletion)
            .stubs()
            .will(ignoreReturnValue());
    EXPECT_EQ(SendClose(fileHandle, cbData), MP_FAILED);
}

TEST_F(CloseRequestTest, SendCloseCb)
{
    int status = 0;
    struct nfs_context *nfs = nfs_init_context();
    NfsContextWrapper nfsContextWrapper(nfs);
    void *data = (struct nfsfh*) malloc(sizeof(struct nfsfh));
    void *privateData = nullptr;

    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(BackupIOEngine::LIBNFS, BackupIOEngine::LIBNFS);

    SendCloseCb(status, nfs, data, privateData);

    auto cbData = new(nothrow) NfsCloseCbData();
    cbData->fileHandle = fileHandle;
    cbData->commonData = nullptr;
    cbData->nfsFh = nullptr;

    MOCKER_CPP(&Libnfscommonmethods::FreeNfsFh)
            .stubs()
            .will(ignoreReturnValue());
    SendCloseCb(status, nfs, data, cbData);

    auto cbData1 = new(nothrow) NfsCloseCbData();
    status = -1;
    cbData1->fileHandle = fileHandle;
    cbData1->commonData = &m_commonData;
    cbData1->nfsFh = (struct nfsfh*) malloc(sizeof(struct nfsfh));

    nfs_get_error(nfs);
    MOCKER_CPP(&LibnfsCopyWriter::IsResumeSendCb)
            .stubs()
            .will(returnValue(true));
    MOCKER_CPP(&LibnfsCopyWriter::ResumeSendCb)
            .stubs()
            .will(ignoreReturnValue());
    MOCKER_CPP(&ProcessCloseCompletion)
            .stubs()
            .will(ignoreReturnValue());
    SendCloseCb(status, nfs, data, cbData1);
}

TEST_F(CloseRequestTest, ProcessCloseCompletion)
{
    NfsCommonData *commonData = nullptr;
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(BackupIOEngine::LIBNFS, BackupIOEngine::LIBNFS);
    fileHandle.m_file->m_nlink = 2;

    ProcessCloseCompletion(fileHandle, commonData);

    NfsCommonData *commonData1 = &m_commonData;
    MOCKER_CPP(&FileDesc::SetSrcState)
            .stubs()
            .will(returnValue(FileDescState::READ_FAILED));
    ProcessCloseCompletion(fileHandle, commonData1);

    MOCKER_CPP(&FileDesc::SetSrcState)
            .stubs()
            .will(returnValue(FileDescState::INIT));
    MOCKER_CPP(&HardLinkMap::IsTargetCopied)
            .stubs()
            .will(returnValue(true));
    MOCKER_CPP(&HardLinkMap::SetTargetCopied)
            .stubs()
            .will(returnValue(true));
    ProcessCloseCompletion(fileHandle, commonData1);
}

TEST_F(CloseRequestTest, SendCloseSync)
{
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(BackupIOEngine::LIBNFS, BackupIOEngine::LIBNFS);

    NfsCloseCbData *cbData = nullptr;
    int ret = SendCloseSync(fileHandle, cbData);
    EXPECT_EQ(ret, MP_FAILED);

    cbData = new(nothrow) NfsCloseCbData();
    cbData->fileHandle = fileHandle;
    cbData->commonData = &m_commonData;
    cbData->nfsFh = nullptr;

    ret = SendCloseSync(fileHandle, cbData);
    EXPECT_EQ(ret, MP_FAILED);

    auto cbData1 = new(nothrow) NfsCloseCbData();
    cbData1->fileHandle = fileHandle;
    cbData1->commonData = &m_commonData;
    cbData1->nfsFh = (struct nfsfh*) malloc(sizeof(struct nfsfh));

    MOCKER_CPP(&NfsContextWrapper::NfsClose)
            .stubs()
            .will(returnValue(-1));
    MOCKER_CPP(&LibnfsCopyWriter::IsResumeSendCb)
            .stubs()
            .will(returnValue(true));
    MOCKER_CPP(&LibnfsCopyWriter::ResumeSendCb)
            .stubs()
            .will(ignoreReturnValue());
    MOCKER_CPP(&Libnfscommonmethods::FreeNfsFh)
            .stubs()
            .will(ignoreReturnValue());
    MOCKER_CPP(&FileDesc::SetSrcState)
            .stubs()
            .will(returnValue(FileDescState::INIT));
    EXPECT_EQ(SendCloseSync(fileHandle, cbData1), MP_SUCCESS);
}
