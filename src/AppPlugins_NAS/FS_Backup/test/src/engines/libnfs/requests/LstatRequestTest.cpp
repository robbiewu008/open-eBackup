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
#include "LstatRequest.h"
#include "LibnfsCopyWriter.h"

using namespace std;
using namespace FS_Backup;
using namespace Module;

namespace  {
    const int MP_SUCCESS = 0;
    const int MP_FAILED = 1;
}

class LstatRequestTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    NfsCommonData m_commonData {};
    NfsContextContainer m_nfsContextContainer;
    BackupTimer m_timer;
};

void LstatRequestTest::SetUp()
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
    m_commonData.hardlinkMap = make_shared<HardLinkMap>();
}

void LstatRequestTest::TearDown()
{
    GlobalMockObject::verify(); // 校验mock规范并清除mock规范
}

void LstatRequestTest::SetUpTestCase()
{}

void LstatRequestTest::TearDownTestCase()
{}

static char* NfsGetError_Stub()
{
    struct nfs_context *nfsContext = nullptr;
    nfsContext = nfs_init_context();
    return nfs_get_error(nfsContext);
}

TEST_F(LstatRequestTest, CreateLstatCbData)
{
    FileHandle fileHandle {};
    NfsCommonData commonData {};
    struct nfsfh* nfsfh {};
    RestoreReplacePolicy restoreReplacePolicy {};
    CreateLstatCbData(fileHandle, commonData, nfsfh, restoreReplacePolicy);
}

TEST_F(LstatRequestTest, SendLstat)
{
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(BackupIOEngine::LIBNFS, BackupIOEngine::LIBNFS);
    fileHandle.m_file->m_fileName = "1.txt";
    fileHandle.m_file->m_onlyFileName = "d1";
    fileHandle.m_file->m_mode = 0;
    fileHandle.m_file->m_dirName = "/d1";

    NfsLstatCbData *cbData = nullptr;
    int ret = SendLstat(fileHandle, cbData);
    EXPECT_EQ(ret, MP_FAILED);

    cbData = new(nothrow) NfsLstatCbData();
    cbData->fileHandle = fileHandle;
    cbData->writeCommonData = &m_commonData;
    cbData->nfsfh = nullptr;

    MOCKER_CPP(&NfsContextWrapper::NfsLstatAsync)
            .stubs()
            .will(returnValue(0))
            .then(returnValue(1));
    EXPECT_EQ(SendLstat(fileHandle, cbData), MP_SUCCESS);

    MOCKER_CPP(&NfsContextWrapper::NfsGetError)
            .stubs()
            .will(invoke(NfsGetError_Stub));
    EXPECT_EQ(SendLstat(fileHandle, cbData), MP_FAILED);

    NfsLstatCbData *cbData1 = new(nothrow) NfsLstatCbData();
    cbData1->fileHandle = fileHandle;
    cbData1->writeCommonData = &m_commonData;
    cbData1->nfsfh = (struct nfsfh*) malloc(sizeof(struct nfsfh));
    MOCKER_CPP(&NfsContextWrapper::NfsLstatAsyncWithDirHandle)
            .stubs()
            .will(returnValue(0));
    EXPECT_EQ(SendLstat(fileHandle, cbData1), MP_SUCCESS);
}

TEST_F(LstatRequestTest, SendLstatCb)
{
    int status = 0;
    struct nfs_context *nfs;
    nfs = nfs_init_context();
    void *data = (struct nfsfh*) malloc(sizeof(struct nfsfh));
    void *privateData = nullptr;

    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(BackupIOEngine::LIBNFS, BackupIOEngine::LIBNFS);
    SendLstatCb(status, nfs, data, privateData);

    auto cbData = new(nothrow) NfsLstatCbData();
    cbData->fileHandle = fileHandle;
    cbData->writeCommonData = nullptr;
    SendLstatCb(status, nfs, data, cbData);

    auto cbData1 = new(nothrow) NfsLstatCbData();
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
    MOCKER_CPP(&CheckConditionsForBackupOrRestoreJob)
            .stubs()
            .will(returnValue(true));
    MOCKER_CPP(&PushToWriteQueue)
            .stubs()
            .will(ignoreReturnValue());
    SendLstatCb(status, nfs, data, cbData1);

    auto cbData2 = new(nothrow) NfsLstatCbData();
    status = -1;
    cbData2->fileHandle = fileHandle;
    cbData2->writeCommonData = &m_commonData;
    MOCKER_CPP(&HandleLstatFailure)
            .stubs()
            .will(ignoreReturnValue());
   SendLstatCb(status, nfs, data, cbData2);

    auto cbData4 = new(nothrow) NfsLstatCbData();
    bool abort = true;
    status = 0;
    m_commonData.abort = &abort;
    cbData4->fileHandle = fileHandle;
    cbData4->writeCommonData = &m_commonData;
    SendLstatCb(status, nfs, data, cbData4);
}

TEST_F(LstatRequestTest, HandleLstatFailure)
{
    NfsCommonData *commonData = nullptr;
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(BackupIOEngine::LIBNFS, BackupIOEngine::LIBNFS);
    int status = 0;
    struct nfs_context *nfs = nfs_init_context();
    HandleLstatFailure(commonData, fileHandle, status, nfs);

    commonData = &m_commonData;
    fileHandle.m_file->m_nlink = 2;
    status = -BACKUP_ERR_NOTDIR;
    MOCKER_CPP(&HardLinkMap::GetTargetPath)
            .stubs()
            .will(returnValue(0))
            .then(returnValue(0));
    MOCKER_CPP(&PushToWriteQueue)
            .stubs()
            .will(ignoreReturnValue())
            .then(ignoreReturnValue());
    HandleLstatFailure(commonData, fileHandle, status, nfs);

    fileHandle.m_file->m_fileName = "1.txt";
    HandleLstatFailure(commonData, fileHandle, status, nfs);

    status = -EINTR;
    fileHandle.m_retryCnt = 0;
    HandleLstatFailure(commonData, fileHandle, status, nfs);

    fileHandle.m_retryCnt = 11;
    MOCKER_CPP(&LstatFailureHandling)
            .stubs()
            .will(ignoreReturnValue())
            .then(ignoreReturnValue());
    HandleLstatFailure(commonData, fileHandle, status, nfs);

    status = -1;
    HandleLstatFailure(commonData, fileHandle, status, nfs);
}

TEST_F(LstatRequestTest, CheckConditionsForBackupOrRestoreJob)
{
    NfsCommonData *commonData = nullptr;
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(BackupIOEngine::LIBNFS, BackupIOEngine::LIBNFS);
    fileHandle.m_file->m_nlink = 1;
    RestoreReplacePolicy restoreReplacePolicy {};
    struct nfs_stat_64 *st;

    bool ret = CheckConditionsForBackupOrRestoreJob(commonData, fileHandle, restoreReplacePolicy, st);
    EXPECT_EQ(ret, false);

    NfsCommonData *commonData1 = &m_commonData;
    MOCKER_CPP(&CheckConditionsForRestore)
            .stubs()
            .will(returnValue(true))
            .then(returnValue(false))
            .then(returnValue(false));
    ret = CheckConditionsForBackupOrRestoreJob(commonData1, fileHandle, restoreReplacePolicy, st);
    EXPECT_EQ(ret, true);

    fileHandle.m_file->m_nlink = 2;
    ret = CheckConditionsForBackupOrRestoreJob(commonData1, fileHandle, restoreReplacePolicy, st);
    EXPECT_EQ(ret, false);

    fileHandle.m_file->m_fileName = "1.txt";
    ret = CheckConditionsForBackupOrRestoreJob(commonData1, fileHandle, restoreReplacePolicy, st);
    EXPECT_EQ(ret, false);
}

TEST_F(LstatRequestTest, CheckConditionsForRestore)
{
    NfsCommonData *commonData = nullptr;
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(BackupIOEngine::LIBNFS, BackupIOEngine::LIBNFS);
    fileHandle.m_file->m_nlink = 2;
    RestoreReplacePolicy restoreReplacePolicy {};
    struct nfs_stat_64 *st;
    string targetPath {};
    CheckConditionsForRestore(commonData, fileHandle, restoreReplacePolicy, st, targetPath);

    NfsCommonData *commonData1 = &m_commonData;

    restoreReplacePolicy = RestoreReplacePolicy::NONE;
    bool ret = CheckConditionsForRestore(commonData1, fileHandle, restoreReplacePolicy, st, targetPath);
    EXPECT_EQ(ret, true);

    restoreReplacePolicy = RestoreReplacePolicy::OVERWRITE;
    MOCKER_CPP(&HandleOverWrite)
            .stubs()
            .will(ignoreReturnValue());
    ret = CheckConditionsForRestore(commonData1, fileHandle, restoreReplacePolicy, st, targetPath);
    EXPECT_EQ(ret, false);

    restoreReplacePolicy = RestoreReplacePolicy::IGNORE_EXIST;
    fileHandle.m_file->m_fileName = "1.txt";
    MOCKER_CPP(&HardLinkMap::IncreaseRef)
            .stubs()
            .will(returnValue(0));
    ret = CheckConditionsForRestore(commonData1, fileHandle, restoreReplacePolicy, st, targetPath);
    EXPECT_EQ(ret, false);
}
TEST_F(LstatRequestTest, HandleOverWrite)
{
    auto commonData = nullptr;
    int status;
    FileHandle fileHandle {};
    RestoreReplacePolicy restoreReplacePolicy = RestoreReplacePolicy::OVERWRITE;
    fileHandle.m_file = std::make_shared<FileDesc>(BackupIOEngine::LIBNFS, BackupIOEngine::LIBNFS);
    fileHandle.m_file->m_fileName = "1.txt";
    fileHandle.m_file->m_size = 5;
    fileHandle.m_file->m_mode = 0;
    struct nfs_stat_64 st;
    HandleOverWrite(commonData, fileHandle, &st, restoreReplacePolicy);

    NfsCommonData *commonData1 = &m_commonData;
    MOCKER_CPP(&FileDesc::SetSrcState)
            .stubs()
            .will(returnValue(FileDescState::LINK))
            .then(returnValue(FileDescState::INIT));
    MOCKER_CPP(&PushToWriteQueue)
            .stubs()
            .will(ignoreReturnValue())
            .then(ignoreReturnValue());
    HandleOverWrite(commonData1, fileHandle, &st, restoreReplacePolicy);
    HandleOverWrite(commonData1, fileHandle, &st, restoreReplacePolicy);
}

TEST_F(LstatRequestTest, LstatFailureHandling)
{
    auto commonData = nullptr;
    int status;
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(BackupIOEngine::LIBNFS, BackupIOEngine::LIBNFS);

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
    LstatFailureHandling(commonData, status, fileHandle);

    NfsCommonData *commonData1 = &m_commonData;
    status = -ERANGE;
    LstatFailureHandling(commonData1, status, fileHandle);

    status = -EACCES;
    LstatFailureHandling(commonData1, status, fileHandle);

    status = -1;
    LstatFailureHandling(commonData1, status, fileHandle);
}

TEST_F(LstatRequestTest, PushToWriteQueue)
{
    auto commonData = nullptr;
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(BackupIOEngine::LIBNFS, BackupIOEngine::LIBNFS);

    PushToWriteQueue(commonData, fileHandle);

    NfsCommonData *commonData1 = &m_commonData;
    PushToWriteQueue(commonData1, fileHandle);
}

static NfsCreateCbData* NfsCreateCbData_Stub()
{
    auto cbData = nullptr;
    return cbData;
}

static NfsCreateCbData* NfsCreateCbData_Stub1()
{
    auto cbData = new(nothrow) NfsCreateCbData();
    return cbData;
}

TEST_F(LstatRequestTest, SendCreateWithTruncateFlag)
{
    FileHandle fileHandle {};
    NfsCommonData *commonData = nullptr;
    struct nfsfh *nfsfh = nullptr;
    RestoreReplacePolicy restoreReplacePolicy {};

    EXPECT_EQ(SendCreateWithTruncateFlag(fileHandle, commonData, nfsfh, restoreReplacePolicy), MP_FAILED);

    NfsCommonData *commonData1 = &m_commonData;

    MOCKER_CPP(&CreateCreateCbData)
            .stubs()
            .will(invoke(NfsCreateCbData_Stub1))
            .then(invoke(NfsCreateCbData_Stub1));
    MOCKER_CPP(&SendNfsRequest)
            .stubs()
            .will(returnValue(1))
            .then(returnValue(0));
    MOCKER_CPP(&CreateFailureHandling)
            .stubs()
            .will(returnValue(0));
    EXPECT_EQ(SendCreateWithTruncateFlag(fileHandle, commonData1, nfsfh, restoreReplacePolicy), MP_FAILED);

    EXPECT_EQ(SendCreateWithTruncateFlag(fileHandle, commonData1, nfsfh, restoreReplacePolicy), MP_SUCCESS);
}

TEST_F(LstatRequestTest, IsSpecialDeviceFile)
{
    mode_t mode = 16832;
    IsSpecialDeviceFile(mode);
}
