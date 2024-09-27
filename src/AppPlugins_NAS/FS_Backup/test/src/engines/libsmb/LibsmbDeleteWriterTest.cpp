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
#include <stdio.h>
#include <iostream>
#include <memory>
#include <shared_mutex>
#include "config_reader/ConfigIniReader.h"
#include "common/FSBackupUtils.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "llt_stub/stub.h"
#include "llt_stub/addr_pri.h"
#include "LibsmbDeleteWriter.h"
#include "libsmb_ctx/SmbContextWrapper.h"

using namespace std;
using namespace Module;
using namespace FS_Backup;

namespace  {
    constexpr uint8_t FILEDESC_IS_DIR = 1;
    constexpr auto RECONNECT_CONTEXT_RETRY_TIMES = 5;
    const int PENDING_PACKET_REACH_THRESHOLD_TIMER_MILLISECOND = 5000;
    const int PENDING_PACKET_REACH_THRESHOLD_SLEEP_SECOND = 1;
    constexpr uint64_t BACKUP_QUEUE_WAIT_TO_MS = 50;
    constexpr auto COMPOUND_READ_MAX_SIZE = 4 * 1024 * 1024;
    constexpr auto OPENED_FILEHANDLE_REACH_THRESHOLD = 10000;
    constexpr auto DEFAULT_POLL_EXPIRED_TIME = 100;
    constexpr uint64_t MAX_BACKUP_QUEUE_SIZE = 10000;
}

class LibsmbDeleteWriterTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    std::unique_ptr<LibsmbDeleteWriter> m_libsmbDeleteWriter = nullptr;
};

void LibsmbDeleteWriterTest::SetUp()
{
    BackupParams backupParams;
    backupParams.dstAdvParams = std::make_shared<LibsmbBackupAdvanceParams>();

    WriterParams deleteWriterParams {};
    deleteWriterParams.backupParams = backupParams;
    deleteWriterParams.writeQueuePtr = nullptr;
    deleteWriterParams.controlInfo = std::make_shared<BackupControlInfo>();
    deleteWriterParams.blockBufferMap = std::make_shared<BlockBufferMap>();

    m_libsmbDeleteWriter = std::make_unique<LibsmbDeleteWriter>(deleteWriterParams);
}

void LibsmbDeleteWriterTest::TearDown()
{}

void LibsmbDeleteWriterTest::SetUpTestCase()
{}

void LibsmbDeleteWriterTest::TearDownTestCase()
{}

static string RemoveFirstSeparator_Stub(void *obj, const string &path)
{
    return "/home/1.txt";
}

static int SmbStat64_Stub_Suc(void *obj, const char *path, struct smb2_stat_64 *st)
{
    return SUCCESS;
}

static int SmbStat64_Stub_NegENOENT(void *obj, const char *path, struct smb2_stat_64 *st)
{
    return -ENOENT;
}

static int SmbStat64_Stub_10(void *obj, const char *path, struct smb2_stat_64 *st)
{
    return 10;
}

static void AddDeleteCounter_Stub_Void(void *obj, bool isSuccess, bool isDir)
{
    return;
}

static BackupRetCode CompareTypeOfDeleteEntryAndBackupCopy_Stub_Suc(void *obj, FileHandle &fileHandle, bool delStatIsDir)
{
    return BackupRetCode::SUCCESS;
}

static BackupRetCode CompareTypeOfDeleteEntryAndBackupCopy_Stub_Fail(void *obj, FileHandle &fileHandle, bool delStatIsDir)
{
    return BackupRetCode::FAILED;
}

static BackupRetCode DeleteFileDirectoryLibSmb_Stub_Suc(void *obj, FileHandle &fileHandle, bool delStatIsDir)
{
    return BackupRetCode::SUCCESS;
}

static BackupRetCode DeleteFileDirectoryLibSmb_Stub_Fail(void *obj, FileHandle &fileHandle, bool delStatIsDir)
{
    return BackupRetCode::FAILED;
}

static SmbWriterCommonData* GetSmbWriterCommonData_Stub(void *obj, FileHandle &fileHandle)
{
    auto cbData = new(nothrow) SmbWriterCommonData();
    return cbData;
}

static int SendWriterSyncRequest_Stub_Suc(void *obj, FileHandle &fileHandle, SmbWriterCommonData *cbData, LibsmbEvent event)
{
    return SUCCESS;
}

static int SendWriterSyncRequest_Stub_Fail(void *obj, FileHandle &fileHandle, SmbWriterCommonData *cbData, LibsmbEvent event)
{
    return FAILED;
}

static int SmbUnlink_Stub_Suc(void *obj, const char *path)
{
    return SUCCESS;
}

static int SmbUnlink_Stub_NegENOENT(void *obj, const char *path)
{
    return -ENOENT;
}

static int SmbUnlink_Stub_10(void *obj, const char *path)
{
    return 10;
}

static string SmbGetError_Stub(void *obj)
{
    return "111";
}

static bool IsComplete_Stub_True(void *obj)
{
    return true;
}

static bool IsComplete_Stub_False(void *obj)
{
    return false;
}

static bool IsAbort_Stub_True(void *obj)
{
    return true;
}

static bool IsAbort_Stub_False(void *obj)
{
    return false;
}

static void FillSmbWriterCommonData_Stub_Void(void *obj)
{
    return;
}

/*
 * 用例名称：验证LibsmbDeleteWriter的IsComplete接口
 * 前置条件：无
 * check点：在ControlFileReader没有file的情况下，IsComplete返回true
 */
TEST_F(LibsmbDeleteWriterTest, IsComplete) {
    BackupParams backupParams;
    backupParams.dstAdvParams = std::make_shared<LibsmbBackupAdvanceParams>();

    WriterParams deleteWriterParams {};
    deleteWriterParams.backupParams = backupParams;
    deleteWriterParams.writeQueuePtr = nullptr;
    deleteWriterParams.controlInfo = std::make_shared<BackupControlInfo>();
    deleteWriterParams.blockBufferMap = std::make_shared<BlockBufferMap>();

    LibsmbDeleteWriter libsmbDeleteWriter(deleteWriterParams);

    deleteWriterParams.controlInfo->m_aggregatePhaseComplete = true;
    EXPECT_EQ(libsmbDeleteWriter.IsComplete(), true);
}

/*
* 用例名称：CheckSmbConnectContexts
* 前置条件：
* check点：验证SmbConnectContexts返回值
*/
TEST_F(LibsmbDeleteWriterTest, CheckSmbConnectContexts)
{
    EXPECT_EQ(m_libsmbDeleteWriter->SmbConnectContexts(), FAILED);
}

/*
* 用例名称：CheckAbort
* 前置条件：
* check点：验证Abort返回值
*/
TEST_F(LibsmbDeleteWriterTest, CheckAbort)
{
    EXPECT_EQ(m_libsmbDeleteWriter->Abort(), BackupRetCode::SUCCESS);
}

/*
* 用例名称：CheckGetStatus
* 前置条件：
* check点：验证GetStatus返回值
*/
TEST_F(LibsmbDeleteWriterTest, CheckGetStatus)
{
    m_libsmbDeleteWriter->m_controlInfo->m_writePhaseComplete = false;
    EXPECT_EQ(m_libsmbDeleteWriter->GetStatus(), BackupPhaseStatus::INPROGRESS);

    m_libsmbDeleteWriter->m_controlInfo->m_writePhaseComplete = true;
    m_libsmbDeleteWriter->m_abort = true;
    EXPECT_EQ(m_libsmbDeleteWriter->GetStatus(), BackupPhaseStatus::ABORTED);

    m_libsmbDeleteWriter->m_abort = false;
    m_libsmbDeleteWriter->m_controlInfo->m_failed = true;
    m_libsmbDeleteWriter->m_controlInfo->m_controlReaderFailed = true;
    EXPECT_EQ(m_libsmbDeleteWriter->GetStatus(), BackupPhaseStatus::FAILED);

    m_libsmbDeleteWriter->m_controlInfo->m_failed = false;
    m_libsmbDeleteWriter->m_controlInfo->m_controlReaderFailed = false;
    EXPECT_EQ(m_libsmbDeleteWriter->GetStatus(), BackupPhaseStatus::COMPLETED);
}

/*
* 用例名称：CheckOpenFile
* 前置条件：
* check点：验证OpenFile返回值
*/
TEST_F(LibsmbDeleteWriterTest, CheckOpenFile)
{
    FileHandle fileHandle;
    EXPECT_EQ(m_libsmbDeleteWriter->OpenFile(fileHandle), SUCCESS);
}

/*
* 用例名称：CheckWriteMeta
* 前置条件：
* check点：验证WriteMeta返回值
*/
TEST_F(LibsmbDeleteWriterTest, CheckWriteMeta)
{
    FileHandle fileHandle;
    EXPECT_EQ(m_libsmbDeleteWriter->WriteMeta(fileHandle), SUCCESS);
}

/*
* 用例名称：CheckWriteData
* 前置条件：
* check点：验证WriteData返回值
*/
TEST_F(LibsmbDeleteWriterTest, CheckWriteData)
{
    Stub stub;
    BackupParams params;
    params.srcEngine = BackupIOEngine::ARCHIVE_CLIENT;
    params.dstEngine = BackupIOEngine::POSIX;
    FileHandle fileHandle;
    fileHandle.m_file = std::make_shared<FileDesc>(params.srcEngine, params.dstEngine);
    
    stub.set(RemoveFirstSeparator, RemoveFirstSeparator_Stub);
    stub.set(ADDR(LibsmbDeleteWriter, AddDeleteCounter), AddDeleteCounter_Stub_Void);

    stub.set(ADDR(SmbContextWrapper, SmbStat64), SmbStat64_Stub_NegENOENT);
    stub.set(ADDR(LibsmbDeleteWriter, AddDeleteCounter), AddDeleteCounter_Stub_Void);
    EXPECT_EQ(m_libsmbDeleteWriter->WriteData(fileHandle), SUCCESS);
    stub.reset(ADDR(SmbContextWrapper, SmbStat64));
    stub.reset(ADDR(LibsmbDeleteWriter, AddDeleteCounter));

    stub.set(ADDR(SmbContextWrapper, SmbStat64), SmbStat64_Stub_10);
    EXPECT_EQ(m_libsmbDeleteWriter->WriteData(fileHandle), FAILED);
    stub.reset(ADDR(SmbContextWrapper, SmbStat64));
    
    stub.set(ADDR(SmbContextWrapper, SmbStat64), SmbStat64_Stub_Suc);
    stub.set(ADDR(LibsmbDeleteWriter, CompareTypeOfDeleteEntryAndBackupCopy), CompareTypeOfDeleteEntryAndBackupCopy_Stub_Fail);
    EXPECT_EQ(m_libsmbDeleteWriter->WriteData(fileHandle), FAILED);
    stub.reset(ADDR(SmbContextWrapper, SmbStat64));
    stub.reset(ADDR(LibsmbDeleteWriter, CompareTypeOfDeleteEntryAndBackupCopy));

    stub.set(ADDR(SmbContextWrapper, SmbStat64), SmbStat64_Stub_Suc);
    stub.set(ADDR(LibsmbDeleteWriter, CompareTypeOfDeleteEntryAndBackupCopy), CompareTypeOfDeleteEntryAndBackupCopy_Stub_Suc);
    stub.set(ADDR(LibsmbDeleteWriter, DeleteFileDirectoryLibSmb), DeleteFileDirectoryLibSmb_Stub_Fail);
    EXPECT_EQ(m_libsmbDeleteWriter->WriteData(fileHandle), FAILED);
    stub.reset(ADDR(SmbContextWrapper, SmbStat64));
    stub.reset(ADDR(LibsmbDeleteWriter, CompareTypeOfDeleteEntryAndBackupCopy));
    stub.reset(ADDR(LibsmbDeleteWriter, DeleteFileDirectoryLibSmb));

    stub.set(ADDR(SmbContextWrapper, SmbStat64), SmbStat64_Stub_Suc);
    stub.set(ADDR(LibsmbDeleteWriter, CompareTypeOfDeleteEntryAndBackupCopy), CompareTypeOfDeleteEntryAndBackupCopy_Stub_Suc);
    stub.set(ADDR(LibsmbDeleteWriter, DeleteFileDirectoryLibSmb), DeleteFileDirectoryLibSmb_Stub_Suc);
    EXPECT_EQ(m_libsmbDeleteWriter->WriteData(fileHandle), SUCCESS);
    stub.reset(ADDR(SmbContextWrapper, SmbStat64));
    stub.reset(ADDR(LibsmbDeleteWriter, CompareTypeOfDeleteEntryAndBackupCopy));
    stub.reset(ADDR(LibsmbDeleteWriter, DeleteFileDirectoryLibSmb));

    stub.reset(RemoveFirstSeparator);
    stub.reset(ADDR(LibsmbDeleteWriter, AddDeleteCounter));
}

/*
* 用例名称：CheckAddDeleteCounter
* 前置条件：
* check点：验证AddDeleteCounter返回值
*/
TEST_F(LibsmbDeleteWriterTest, CheckAddDeleteCounter)
{
    bool isSuccess = true;
    bool isDir = true;
    EXPECT_NO_THROW(m_libsmbDeleteWriter->AddDeleteCounter(isSuccess, isDir));

    isDir = false;
    EXPECT_NO_THROW(m_libsmbDeleteWriter->AddDeleteCounter(isSuccess, isDir));

    isSuccess = false;
    isDir = true;
    EXPECT_NO_THROW(m_libsmbDeleteWriter->AddDeleteCounter(isSuccess, isDir));
    
    isDir = false;
    EXPECT_NO_THROW(m_libsmbDeleteWriter->AddDeleteCounter(isSuccess, isDir));
}

/*
* 用例名称：CheckCompareTypeOfDeleteEntryAndBackupCopy
* 前置条件：
* check点：验证CompareTypeOfDeleteEntryAndBackupCopy返回值
*/
TEST_F(LibsmbDeleteWriterTest, CheckCompareTypeOfDeleteEntryAndBackupCopy)
{
    BackupParams params;
    params.srcEngine = BackupIOEngine::ARCHIVE_CLIENT;
    params.dstEngine = BackupIOEngine::POSIX;
    FileHandle fileHandle;
    fileHandle.m_file = std::make_shared<FileDesc>(params.srcEngine, params.dstEngine);

    fileHandle.m_file->SetFlag(IS_DIR);
    bool delStatIsDir = false;
    EXPECT_EQ(m_libsmbDeleteWriter->CompareTypeOfDeleteEntryAndBackupCopy(fileHandle, delStatIsDir), BackupRetCode::FAILED);

    delStatIsDir = true;
    EXPECT_EQ(m_libsmbDeleteWriter->CompareTypeOfDeleteEntryAndBackupCopy(fileHandle, delStatIsDir), BackupRetCode::SUCCESS);

    fileHandle.m_file->ClearFlag(IS_DIR);
    delStatIsDir = false;
    EXPECT_EQ(m_libsmbDeleteWriter->CompareTypeOfDeleteEntryAndBackupCopy(fileHandle, delStatIsDir), BackupRetCode::SUCCESS);

    delStatIsDir = true;
    EXPECT_EQ(m_libsmbDeleteWriter->CompareTypeOfDeleteEntryAndBackupCopy(fileHandle, delStatIsDir), BackupRetCode::FAILED);
}

/*
* 用例名称：CheckDeleteFileDirectoryLibSmb
* 前置条件：
* check点：验证DeleteFileDirectoryLibSmb返回值
*/
TEST_F(LibsmbDeleteWriterTest, CheckDeleteFileDirectoryLibSmb)
{
    Stub stub;
    BackupParams params;
    params.srcEngine = BackupIOEngine::ARCHIVE_CLIENT;
    params.dstEngine = BackupIOEngine::POSIX;
    FileHandle fileHandle;
    fileHandle.m_file = std::make_shared<FileDesc>(params.srcEngine, params.dstEngine);

    bool isDir = true;
    stub.set(ADDR(LibsmbDeleteWriter, GetSmbWriterCommonData), GetSmbWriterCommonData_Stub);
    stub.set(SendWriterSyncRequest, SendWriterSyncRequest_Stub_Fail);
    EXPECT_EQ(m_libsmbDeleteWriter->DeleteFileDirectoryLibSmb(fileHandle, isDir), BackupRetCode::FAILED);
    stub.reset(SendWriterSyncRequest);
    stub.set(SendWriterSyncRequest, SendWriterSyncRequest_Stub_Suc);
    EXPECT_EQ(m_libsmbDeleteWriter->DeleteFileDirectoryLibSmb(fileHandle, isDir), BackupRetCode::SUCCESS);
    stub.reset(SendWriterSyncRequest);
    stub.reset(ADDR(LibsmbDeleteWriter, GetSmbWriterCommonData));

    isDir = false;
    stub.set(RemoveFirstSeparator, RemoveFirstSeparator_Stub);
    stub.set(ADDR(SmbContextWrapper, SmbUnlink), SmbUnlink_Stub_Suc);
    EXPECT_EQ(m_libsmbDeleteWriter->DeleteFileDirectoryLibSmb(fileHandle, isDir), BackupRetCode::SUCCESS);
    stub.reset(ADDR(SmbContextWrapper, SmbUnlink));
    
    stub.set(ADDR(SmbContextWrapper, SmbUnlink), SmbUnlink_Stub_NegENOENT);
    EXPECT_EQ(m_libsmbDeleteWriter->DeleteFileDirectoryLibSmb(fileHandle, isDir), BackupRetCode::SUCCESS);
    stub.reset(ADDR(SmbContextWrapper, SmbUnlink));
    
    stub.set(ADDR(SmbContextWrapper, SmbUnlink), SmbUnlink_Stub_10);
    stub.set(ADDR(SmbContextWrapper, SmbGetError), SmbGetError_Stub);
    EXPECT_EQ(m_libsmbDeleteWriter->DeleteFileDirectoryLibSmb(fileHandle, isDir), BackupRetCode::FAILED);
    stub.reset(ADDR(SmbContextWrapper, SmbUnlink));
    stub.reset(ADDR(SmbContextWrapper, SmbGetError));
    stub.reset(RemoveFirstSeparator);
}

/*
* 用例名称：CheckIsAbort
* 前置条件：
* check点：验证IsAbort返回值
*/
TEST_F(LibsmbDeleteWriterTest, CheckIsAbort)
{
    m_libsmbDeleteWriter->m_abort = true;
    m_libsmbDeleteWriter->m_controlInfo->m_failed = true;
    m_libsmbDeleteWriter->m_controlInfo->m_controlReaderFailed = true;
    EXPECT_EQ(m_libsmbDeleteWriter->IsAbort(), true);

    m_libsmbDeleteWriter->m_abort = false;
    m_libsmbDeleteWriter->m_controlInfo->m_failed = false;
    m_libsmbDeleteWriter->m_controlInfo->m_controlReaderFailed = false;
    EXPECT_EQ(m_libsmbDeleteWriter->IsAbort(), false);
}


/*
* 用例名称：CheckThreadFunc
* 前置条件：
* check点：验证ThreadFunc返回值
*/
TEST_F(LibsmbDeleteWriterTest, CheckThreadFunc)
{
    Stub stub;
    stub.set(ADDR(LibsmbDeleteWriter, IsComplete), IsComplete_Stub_True);
    EXPECT_NO_THROW(m_libsmbDeleteWriter->ThreadFunc());
    stub.reset(ADDR(LibsmbDeleteWriter, IsComplete));

    stub.set(ADDR(LibsmbDeleteWriter, IsComplete), IsComplete_Stub_False);
    stub.set(ADDR(LibsmbDeleteWriter, IsAbort), IsAbort_Stub_True);
    EXPECT_NO_THROW(m_libsmbDeleteWriter->ThreadFunc());
    stub.reset(ADDR(LibsmbDeleteWriter, IsComplete));
    stub.reset(ADDR(LibsmbDeleteWriter, IsAbort));
}

/*
* 用例名称：CheckFillSmbWriterCommonData
* 前置条件：
* check点：检查FillSmbWriterCommonData返回值
*/
TEST_F(LibsmbDeleteWriterTest, CheckFillSmbWriterCommonData)
{
    m_libsmbDeleteWriter->m_deleteSmbContext = std::shared_ptr<Module::SmbContextWrapper>();
    m_libsmbDeleteWriter->m_params.writeMeta = true;
    m_libsmbDeleteWriter->m_pktStats = std::shared_ptr<PacketStats>();
    auto writerCommonData = new(nothrow) SmbWriterCommonData();
    EXPECT_NO_THROW(m_libsmbDeleteWriter->FillSmbWriterCommonData(writerCommonData));
    delete writerCommonData;
}

/*
* 用例名称：CheckGetSmbWriterCommonData
* 前置条件：
* check点：检查GetSmbWriterCommonData返回值
*/
TEST_F(LibsmbDeleteWriterTest, CheckGetSmbWriterCommonData)
{
    Stub stub;
    FileHandle fileHandle;

    stub.set(ADDR(LibsmbDeleteWriter, FillSmbWriterCommonData), FillSmbWriterCommonData_Stub_Void);
    EXPECT_NO_THROW(m_libsmbDeleteWriter->GetSmbWriterCommonData(fileHandle));
    stub.reset(ADDR(LibsmbDeleteWriter, FillSmbWriterCommonData));
}