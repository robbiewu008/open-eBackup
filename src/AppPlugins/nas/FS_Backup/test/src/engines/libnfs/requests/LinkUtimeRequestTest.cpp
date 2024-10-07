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
#include "LinkUtimeRequest.h"
#include "LibnfsCopyWriter.h"


using namespace std;
using namespace FS_Backup;
using namespace Module;

namespace  {
    const int MP_SUCCESS = 0;
    const int MP_FAILED = 1;
}

class LinkUtimeRequestTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    NfsCommonData m_commonData {};
    NfsContextContainer m_nfsContextContainer;
    BackupTimer m_timer;
};
void LinkUtimeRequestTest::SetUp()
{
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
}

void LinkUtimeRequestTest::TearDown()
{
    GlobalMockObject::verify(); // 校验mock规范并清除mock规范
}

void LinkUtimeRequestTest::SetUpTestCase()
{}

void LinkUtimeRequestTest::TearDownTestCase()
{}

static char* NfsGetError_Stub()
{
    struct nfs_context *nfsContext = nullptr;
    nfsContext = nfs_init_context();
    return nfs_get_error(nfsContext);
}

TEST_F(LinkUtimeRequestTest, CreateLinkUtimeCbData)
{
    FileHandle fileHandle {};
    NfsCommonData commonData {};
    CreateLinkUtimeCbData(fileHandle, commonData);
}

TEST_F(LinkUtimeRequestTest, SendLinkUtime)
{
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(BackupIOEngine::LIBNFS, BackupIOEngine::LIBNFS);
    fileHandle.m_file->m_fileName = "1.txt";
    RestoreReplacePolicy restoreReplacePolicy {};
    string targetPath {};

    NfsLinkUtimeCbData *cbData = nullptr;
    int ret = SendLinkUtime(fileHandle, cbData);
    EXPECT_EQ(ret, MP_FAILED);

    NfsLinkUtimeCbData *cbData1 = new(nothrow) NfsLinkUtimeCbData();
    cbData1->fileHandle = fileHandle;
    cbData1->writeCommonData = &m_commonData;

    MOCKER_CPP(&NfsContextWrapper::NfsLutimeAsync)
            .stubs()
            .will(returnValue(0))
            .then(returnValue(1));
    EXPECT_EQ(SendLinkUtime(fileHandle, cbData1), MP_SUCCESS);

    MOCKER_CPP(&NfsContextWrapper::NfsGetError)
            .stubs()
            .will(invoke(NfsGetError_Stub));
    EXPECT_EQ(SendLinkUtime(fileHandle, cbData1), MP_FAILED);
}

TEST_F(LinkUtimeRequestTest, SendLinkUtimeCb)
{
    int status = 0;
    struct nfs_context *nfs;
    nfs = nfs_init_context();
    void *data = (struct nfsfh*) malloc(sizeof(struct nfsfh));
    void *privateData = nullptr;

    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(BackupIOEngine::LIBNFS, BackupIOEngine::LIBNFS);
    fileHandle.m_file->m_inode = 10456;
    fileHandle.m_file->m_scannermode = CTRL_ENTRY_MODE_BOTH_MODIFIED;
    fileHandle.m_file->m_nlink = 2;

    SendLinkUtimeCb(status, nfs, data, privateData);

    auto cbData = new(nothrow) NfsLinkUtimeCbData();
    cbData->fileHandle = fileHandle;
    cbData->writeCommonData = nullptr;

    SendLinkUtimeCb(status, nfs, data, cbData);

    auto cbData1 = new(nothrow) NfsLinkUtimeCbData();
    status = 0;
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
    MOCKER_CPP(&HardLinkMap::IsTargetCopied)
            .stubs()
            .will(returnValue(false));
    MOCKER_CPP(&HardLinkMap::SetTargetCopied)
            .stubs()
            .will(returnValue(true));
    SendLinkUtimeCb(status, nfs, data, cbData1);

    auto cbData2 = new(nothrow) NfsLinkUtimeCbData();
    status = -1;
    cbData2->fileHandle = fileHandle;
    cbData2->writeCommonData = &m_commonData;

    MOCKER_CPP(&HandleLinkUtimeFailure)
            .stubs()
            .will(ignoreReturnValue());
    SendLinkUtimeCb(status, nfs, data, cbData2);

    auto cbData3 = new(nothrow) NfsLinkUtimeCbData();
    bool abort1 = true;
    status = 0;
    m_commonData.abort = &abort1;
    cbData3->fileHandle = fileHandle;
    cbData3->writeCommonData = &m_commonData;

    SendLinkUtimeCb(status, nfs, data, cbData3);
}

TEST_F(LinkUtimeRequestTest, HandleLinkUtimeFailure)
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

    HandleLinkUtimeFailure(commonData, status, fileHandle, nfs);

    NfsCommonData *commonData1 = &m_commonData;
    status = -BACKUP_ERR_NOTDIR;
    HandleLinkUtimeFailure(commonData1, status, fileHandle, nfs);

    status = -BACKUP_ERR_EEXIST;
    HandleLinkUtimeFailure(commonData1, status, fileHandle, nfs);

    status = -EINTR;
    HandleLinkUtimeFailure(commonData1, status, fileHandle, nfs);

    status = -EINTR;
    fileHandle.m_retryCnt = 11;
    MOCKER_CPP(&LinkUtimeFailureHandling)
            .stubs()
            .will(ignoreReturnValue());
    HandleLinkUtimeFailure(commonData1, status, fileHandle, nfs);

    status = -1;
    nfs_get_error(nfs);
    MOCKER_CPP(&LinkUtimeFailureHandling)
            .stubs()
            .will(ignoreReturnValue());
    HandleLinkUtimeFailure(commonData1, status, fileHandle, nfs);
}

TEST_F(LinkUtimeRequestTest, LinkUtimeFailureHandling)
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
    LinkUtimeFailureHandling(commonData, status, fileHandle);

    NfsCommonData *commonData1 = &m_commonData;
    status = -ERANGE;
    LinkUtimeFailureHandling(commonData1, status, fileHandle);

    status = -EACCES;
    LinkUtimeFailureHandling(commonData1, status, fileHandle);

    status = -1;
    LinkUtimeFailureHandling(commonData1, status, fileHandle);
}
