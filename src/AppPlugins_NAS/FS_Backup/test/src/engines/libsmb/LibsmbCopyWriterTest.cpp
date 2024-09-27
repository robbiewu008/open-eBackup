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
#include "LibsmbCopyWriter.h"
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

class LibsmbCopyWriterTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();

    std::unique_ptr<LibsmbCopyWriter> m_libsmbCopyWriter = nullptr;

};

void LibsmbCopyWriterTest::SetUp()
{
    BackupQueueConfig config {DEFAULT_BACKUP_QUEUE_SIZE, DEFAULT_BACKUP_QUEUE_MEMORY_SIZE};
    BackupParams params;
    params.srcEngine = BackupIOEngine::ARCHIVE_CLIENT;
    params.dstEngine = BackupIOEngine::POSIX;
    params.srcAdvParams = std::make_shared<LibsmbBackupAdvanceParams>();
    params.dstAdvParams = std::make_shared<LibsmbBackupAdvanceParams>();

    WriterParams copyWriterParams {};
    copyWriterParams.backupParams = params;
    copyWriterParams.writeQueuePtr = std::make_shared<BackupQueue<FileHandle>>(config);
    copyWriterParams.readQueuePtr = std::make_shared<BackupQueue<FileHandle>>(config);
    copyWriterParams.controlInfo = std::make_shared<BackupControlInfo>();
    copyWriterParams.blockBufferMap = std::make_shared<BlockBufferMap>();

    m_libsmbCopyWriter = std::make_unique<LibsmbCopyWriter>(copyWriterParams);
}

void LibsmbCopyWriterTest::TearDown()
{}

void LibsmbCopyWriterTest::SetUpTestCase()
{}

void LibsmbCopyWriterTest::TearDownTestCase()
{}

int SmbConnectContexts_stub2()
{
    return Module::FAILED;
}

void SmbDisconnectContexts_stub2()
{
    return;
}

string SmbGetClientGuid_stub2()
{
    return "";
}

static int SmbCloseAsync_stub(struct smb2fh *smbfh, smb2_command_cb cb, void *privateData)
{
    return 0;
}

static uint64_t GetValue_Stub_DEFAULT_MAX_NOACCESS(void *obj, PKT_TYPE reqType, PKT_COUNTER counterType)
{
    return DEFAULT_MAX_NOACCESS + 1;
}

static uint64_t GetValue_Stub_Zero(void *obj, PKT_TYPE reqType, PKT_COUNTER counterType)
{
    return 0;
}

static uint64_t GetValue_Stub_MAX_PENDING_REQUEST_COUNT(void *obj, PKT_TYPE reqType, PKT_COUNTER counterType)
{
    return MAX_PENDING_REQUEST_COUNT + 1;
}

static int HandleConnectionException_Stub_Suc(void *obj, std::shared_ptr<SmbContextWrapper> &smbContext,
    SmbContextArgs &contextArgs, int connectRetryTimes) {
    return SUCCESS;
}

static int HandleConnectionException_Stub_Fail(void *obj, std::shared_ptr<SmbContextWrapper> &smbContext,
    SmbContextArgs &contextArgs, int connectRetryTimes) {
    return FAILED;
}

static int ServerCheck_Stub_Suc(void *obj)
{
    return SUCCESS;
}

static int ServerCheck_Stub_Fail(void *obj)
{
    return FAILED;
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

static int ProcessConnectionException_Stub_Suc(void *obj)
{
    return SUCCESS;
}

static int ProcessConnectionException_Stub_Fail(void *obj)
{
    return FAILED;
}

static uint64_t GetValue_Stub_Overmax(void *obj, PKT_TYPE reqType, PKT_COUNTER counterType)
{
    return 1000000;
}

static int Poll_Stub_1(void *obj)
{
    return 1;
}

static int Poll_Stub_Neg1(void *obj)
{
    return -1;
}

static void SmbDisconnectContexts_Stub_Void(void *obj)
{
    return;
}

static void HandleComplete_Stub_Void(void *obj)
{
    return;
}

static void ProcessWriteEntries_Stub_Void(void *obj)
{
    return;
}

static void ClearWriteCache_Stub_Void(void *obj)
{
    return;
}

static bool IsWriterRequestReachThreshold_Stub_True(void *obj)
{
    return true;
}

static bool IsWriterRequestReachThreshold_Stub_False(void *obj)
{
    return false;
}

static void ProcessFileDescState_Stub_Void(void *obj, FileHandle fileHandle)
{
    return;
}

static int WriteMeta_Stub_Suc(void *obj, FileHandle &fileHandle)
{
    return SUCCESS;
}

static int WriteData_Stub_Suc(void *obj, FileHandle &fileHandle)
{
    return SUCCESS;
}

static int OpenFile_Stub_Suc(void *obj, FileHandle &fileHandle)
{
    return SUCCESS;
}

static int DeleteFile_Stub_Suc(void *obj, FileHandle &fileHandle)
{
    return SUCCESS;
}

static int CloseFile_Stub_Suc(void *obj, FileHandle &fileHandle)
{
    return SUCCESS;
}

static bool IsOpenBlock_Stub_True(void *obj, FileHandle &fileHandle)
{
    return true;
}

static bool IsOpenBlock_Stub_False(void *obj, FileHandle &fileHandle)
{
    return false;
}

static FileDescState GetDstState_Stub_DST_CLOSED(void *obj)
{
    return FileDescState::DST_CLOSED;
}

static FileDescState GetDstState_Stub_INIT(void *obj)
{
    return FileDescState::INIT;
}

static FileDescState GetDstState_Stub_DST_OPENED(void *obj)
{
    return FileDescState::DST_OPENED;
}

static FileDescState GetDstState_Stub_PARTIAL_WRITED(void *obj)
{
    return FileDescState::PARTIAL_WRITED;
}

static FileDescState GetDstState_Stub_WRITED(void *obj)
{
    return FileDescState::WRITED;
}

static FileDescState GetDstState_Stub_LSTAT(void *obj)
{
    return FileDescState::LSTAT;
}

static FileDescState GetDstState_Stub_DIR_DEL(void *obj)
{
    return FileDescState::DIR_DEL;
}

static FileDescState GetSrcState_Stub_READ_FAILED(void *obj)
{
    return FileDescState::READ_FAILED;
}

static FileDescState GetSrcState_Stub_INIT(void *obj)
{
    return FileDescState::INIT;
}

static bool IsFileReadOrWriteFailed_Stub_True(void *obj, FileHandle &fileHandle)
{
    return true;
}

static bool IsFileReadOrWriteFailed_Stub_False(void *obj, FileHandle &fileHandle)
{
    return false;
}

static bool IsBackupTask_Stub_True(void *obj, BackupType type)
{
    return true;
}

static bool IsBackupTask_Stub_False(void *obj, BackupType type)
{
    return false;
}

/*
 * 用例名称：验证LibsmbCopyWriter线程启动
 * 前置条件：无
 * check点：线程正常启动后，正常退出
 */
TEST_F(LibsmbCopyWriterTest, Start) {
    BackupParams backupParams;
    backupParams.dstAdvParams = std::make_shared<LibsmbBackupAdvanceParams>();

    WriterParams copyWriterParams {};
    copyWriterParams.backupParams = backupParams;
    copyWriterParams.writeQueuePtr = nullptr;
    copyWriterParams.readQueuePtr = nullptr;
    copyWriterParams.controlInfo = std::make_shared<BackupControlInfo>();
    copyWriterParams.blockBufferMap = nullptr;

    LibsmbCopyWriter libsmbCopyWriter(copyWriterParams);

    Stub stub;
    stub.set(ADDR(LibsmbCopyWriter, SmbConnectContexts), SmbConnectContexts_stub2);
    stub.set(ADDR(LibsmbCopyWriter, SmbDisconnectContexts), SmbDisconnectContexts_stub2);
    // smb初始化失败，start返回failed
    EXPECT_EQ(libsmbCopyWriter.Start(), BackupRetCode::FAILED);
    stub.reset(ADDR(LibsmbCopyWriter, SmbConnectContexts));
    stub.reset(ADDR(LibsmbCopyWriter, SmbDisconnectContexts));
}

/*
 * 用例名称：验证LibsmbCopyWriter的WriteData接口
 * 前置条件：无
 * check点：WriteData返回成功，计数加1
 */
TEST_F(LibsmbCopyWriterTest, WriteData) {
    BackupParams backupParams;
    backupParams.dstAdvParams = std::make_shared<LibsmbBackupAdvanceParams>();

    WriterParams copyWriterParams {};
    copyWriterParams.backupParams = backupParams;
    copyWriterParams.writeQueuePtr = nullptr;
    copyWriterParams.readQueuePtr = nullptr;
    copyWriterParams.controlInfo = std::make_shared<BackupControlInfo>();
    copyWriterParams.blockBufferMap = nullptr;

    LibsmbCopyWriter libsmbCopyWriter(copyWriterParams);

    FileHandle fh;
    fh.m_file = make_shared<FileDesc>(BackupIOEngine::LIBSMB, BackupIOEngine::LIBSMB);
    fh.m_file->m_fileName = "opendstcb_test.txt";
    fh.m_file->m_size = 0;
    // EXPECT_EQ(libsmbCopyWriter.WriteData(fh), Module::SUCCESS);
}

/*
 * 用例名称：验证LibsmbCopyWriter的WriteMeta接口
 * 前置条件：无
 * check点：WriteMeta返回成功，计数加1
 */
TEST_F(LibsmbCopyWriterTest, WriteMeta) {
    BackupParams backupParams;
    backupParams.dstAdvParams = std::make_shared<LibsmbBackupAdvanceParams>();

    WriterParams copyWriterParams {};
    copyWriterParams.backupParams = backupParams;
    copyWriterParams.writeQueuePtr = nullptr;
    copyWriterParams.readQueuePtr = nullptr;
    copyWriterParams.controlInfo = std::make_shared<BackupControlInfo>();
    copyWriterParams.blockBufferMap = nullptr;

    LibsmbCopyWriter libsmbCopyWriter(copyWriterParams);

    FileHandle fh;
    fh.m_file = make_shared<FileDesc>(BackupIOEngine::LIBSMB, BackupIOEngine::LIBSMB);
    fh.m_file->m_fileName = "opendstcb_test.txt";
    fh.m_file->m_size = 0;
    // EXPECT_EQ(libsmbCopyWriter.WriteMeta(fh), Module::SUCCESS);
}

/*
 * 用例名称：验证LibsmbCopyWriter的IsComplete接口
 * 前置条件：无
 * check点：在ControlFileReader没有file的情况下，IsComplete返回true
 */
// TEST_F(LibsmbCopyWriterTest, IsComplete) {
//     BackupParams backupParams;
//     backupParams.dstAdvParams = std::make_shared<LibsmbBackupAdvanceParams>();

//     WriterParams copyWriterParams {};
//     copyWriterParams.backupParams = backupParams;
//     copyWriterParams.writeQueuePtr = nullptr;
//     copyWriterParams.readQueuePtr = nullptr;
//     copyWriterParams.controlInfo = std::make_shared<BackupControlInfo>();
//     copyWriterParams.blockBufferMap = std::make_shared<BlockBufferMap>();

//     LibsmbCopyWriter libsmbCopyWriter(copyWriterParams);

//     copyWriterParams.controlInfo->m_aggregatePhaseComplete = true;
//     EXPECT_EQ(libsmbCopyWriter.IsComplete(), true);
// }

/*
 * 用例名称：验证LibsmbCopyWriter的IsMkdirComplete接口
 * 前置条件：无
 * check点：在writeQueue没有任何项的情况下，IsMkdirComplete返回true
 */
TEST_F(LibsmbCopyWriterTest, IsMkdirComplete) {
    BackupParams backupParams;
    backupParams.dstAdvParams = std::make_shared<LibsmbBackupAdvanceParams>();

    WriterParams copyWriterParams {};
    copyWriterParams.backupParams = backupParams;
    copyWriterParams.writeQueuePtr = nullptr;
    copyWriterParams.readQueuePtr = nullptr;
    copyWriterParams.controlInfo = std::make_shared<BackupControlInfo>();
    copyWriterParams.blockBufferMap = std::make_shared<BlockBufferMap>();

    LibsmbCopyWriter libsmbCopyWriter(copyWriterParams);

    BackupQueueConfig config;
    config.maxSize = DEFAULT_BACKUP_QUEUE_SIZE;
    config.maxMemorySize = DEFAULT_BACKUP_QUEUE_MEMORY_SIZE;
    libsmbCopyWriter.m_dirQueue = std::make_shared<BackupQueue<FileHandle>>(config);
    copyWriterParams.controlInfo->m_aggregatePhaseComplete = true;
    EXPECT_NO_THROW(libsmbCopyWriter.IsMkdirComplete());
}

static std::shared_ptr<SmbContextWrapper> SmbConnectContext_Stub(void *obj, const SmbContextArgs &args)
{
    return nullptr;
}

/*
 * 用例名称：CheckSmbConnectContexts
 * 前置条件：无
 * check点：验证SmbConnectContexts返回值
 */
TEST_F(LibsmbCopyWriterTest, CheckSmbConnectContexts) {
    EXPECT_EQ(m_libsmbCopyWriter->SmbConnectContexts(), -1);
}

/*
 * 用例名称：CheckSmbDisconnectContexts
 * 前置条件：无
 * check点：验证SmbDisconnectContexts返回值
 */
TEST_F(LibsmbCopyWriterTest, CheckSmbDisconnectContexts) {
    EXPECT_NO_THROW(m_libsmbCopyWriter->SmbDisconnectContexts());
}

/*
 * 用例名称：CheckSmbDisconnectSyncContexts
 * 前置条件：无
 * check点：验证SmbDisconnectSyncContexts返回值
 */
TEST_F(LibsmbCopyWriterTest, CheckSmbDisconnectSyncContexts)
{
    EXPECT_NO_THROW(m_libsmbCopyWriter->SmbDisconnectSyncContexts());
}

/*
* 用例名称：CheckAbort
* 前置条件：
* check点：检查Abort返回值
*/
TEST_F(LibsmbCopyWriterTest, CheckAbort)
{
    EXPECT_EQ(m_libsmbCopyWriter->Abort(), BackupRetCode::SUCCESS);
}

/*
* 用例名称：CheckGetStatus
* 前置条件：
* check点：检查GetStatus返回值
*/
TEST_F(LibsmbCopyWriterTest, CheckGetStatus)
{
    m_libsmbCopyWriter->m_controlInfo->m_writePhaseComplete = false;
    EXPECT_EQ(m_libsmbCopyWriter->GetStatus(), BackupPhaseStatus::INPROGRESS);

    m_libsmbCopyWriter->m_controlInfo->m_writePhaseComplete = true;
    m_libsmbCopyWriter->m_abort = true;
    EXPECT_EQ(m_libsmbCopyWriter->GetStatus(), BackupPhaseStatus::ABORTED);

    m_libsmbCopyWriter->m_abort = false;
    m_libsmbCopyWriter->m_failed = true;
    m_libsmbCopyWriter->m_controlInfo->m_failed = true;
    m_libsmbCopyWriter->m_controlInfo->m_controlReaderFailed = true;
    EXPECT_EQ(m_libsmbCopyWriter->GetStatus(), BackupPhaseStatus::FAILED);

    m_libsmbCopyWriter->m_abort = false;
    m_libsmbCopyWriter->m_failed = false;
    m_libsmbCopyWriter->m_controlInfo->m_failed = false;
    m_libsmbCopyWriter->m_controlInfo->m_controlReaderFailed = false;
    EXPECT_EQ(m_libsmbCopyWriter->GetStatus(), BackupPhaseStatus::COMPLETED);
}

/*
* 用例名称：CheckIsAbort
* 前置条件：
* check点：检查IsAbort返回值
*/
TEST_F(LibsmbCopyWriterTest, CheckIsAbort)
{
    m_libsmbCopyWriter->m_abort = true;
    m_libsmbCopyWriter->m_failed = true;
    m_libsmbCopyWriter->m_controlInfo->m_failed = true;
    m_libsmbCopyWriter->m_controlInfo->m_controlReaderFailed = true;
    EXPECT_EQ(m_libsmbCopyWriter->IsAbort(), true);

    m_libsmbCopyWriter->m_abort = false;
    m_libsmbCopyWriter->m_failed = false;
    m_libsmbCopyWriter->m_controlInfo->m_failed = false;
    m_libsmbCopyWriter->m_controlInfo->m_controlReaderFailed = false;
    EXPECT_EQ(m_libsmbCopyWriter->IsAbort(), false);
}

/*
* 用例名称：CheckIsComplete
* 前置条件：
* check点：检查IsComplete返回值
*/
// TEST_F(LibsmbCopyWriterTest, CheckIsComplete)
// {
//     EXPECT_NO_THROW(m_libsmbCopyWriter->IsComplete());
// }

static bool Empty_Stub_True(void *obj)
{
    return true;
}
/*
* 用例名称：CheckIsMkdirComplete
* 前置条件：
* check点：检查IsMkdirComplete返回值
*/
// TEST_F(LibsmbCopyWriterTest, CheckIsMkdirComplete)
// {
//     Stub stub;

//     m_libsmbCopyWriter->m_controlInfo->m_writePhaseComplete = true;
//     stub.set(ADDR(BackupQueue, Empty), Empty_Stub_True);
//     EXPECT_EQ(m_libsmbCopyWriter->IsMkdirComplete(), true);
//     stub.reset(ADDR(BackupQueue, Empty));

//     m_libsmbCopyWriter->m_controlInfo->m_writePhaseComplete = false;
//     stub.set(ADDR(BackupQueue, Empty), Empty_Stub_False);
//     EXPECT_EQ(m_libsmbCopyWriter->IsMkdirComplete(), false);
//     stub.reset(ADDR(BackupQueue, Empty));
// }

/*
* 用例名称：CheckHandleComplete
* 前置条件：
* check点：检查HandleComplete返回值
*/
TEST_F(LibsmbCopyWriterTest, CheckHandleComplete)
{
    EXPECT_NO_THROW(m_libsmbCopyWriter->HandleComplete());
}

static SmbWriterCommonData* GetSmbWriterCommonData_Stub(void *obj, FileHandle &fileHandle)
{
    auto cbData = new(nothrow) SmbWriterCommonData();
    return cbData;
}

static int SendWriterRequest_Stub_Suc(void *obj, FileHandle &fileHandle, SmbWriterCommonData *cbData, LibsmbEvent event)
{
    return SUCCESS;
}

static int SendWriterRequest_Stub_Fail(void *obj, FileHandle &fileHandle, SmbWriterCommonData *cbData, LibsmbEvent event)
{
    return FAILED;
}

/*
* 用例名称：CheckOpenFile
* 前置条件：
* check点：检查OpenFile返回值
*/
TEST_F(LibsmbCopyWriterTest, CheckOpenFile)
{
    Stub stub;

    BackupParams params;
    params.srcEngine = BackupIOEngine::ARCHIVE_CLIENT;
    params.dstEngine = BackupIOEngine::POSIX;

    FileHandle fileHandle; 
    fileHandle.m_file = std::make_shared<FileDesc>(params.srcEngine, params.dstEngine);
    m_libsmbCopyWriter->m_pktStats = std::shared_ptr<PacketStats>();
    
    stub.set(ADDR(LibsmbCopyWriter, GetSmbWriterCommonData), GetSmbWriterCommonData_Stub);

    stub.set(SendWriterRequest, SendWriterRequest_Stub_Suc);
    EXPECT_EQ(m_libsmbCopyWriter->OpenFile(fileHandle), SUCCESS);
    stub.reset(SendWriterRequest);

    stub.set(SendWriterRequest, SendWriterRequest_Stub_Fail);
    EXPECT_EQ(m_libsmbCopyWriter->OpenFile(fileHandle), FAILED);
    stub.reset(SendWriterRequest);

    stub.reset(ADDR(LibsmbCopyWriter, GetSmbWriterCommonData));
}

/*
* 用例名称：CheckWriteMeta
* 前置条件：
* check点：检查WriteMeta返回值
*/
TEST_F(LibsmbCopyWriterTest, CheckWriteMeta)
{
    Stub stub;

    BackupParams params;
    params.srcEngine = BackupIOEngine::ARCHIVE_CLIENT;
    params.dstEngine = BackupIOEngine::POSIX;

    FileHandle fileHandle; 
    fileHandle.m_file = std::make_shared<FileDesc>(params.srcEngine, params.dstEngine);
    
    stub.set(ADDR(LibsmbCopyWriter, GetSmbWriterCommonData), GetSmbWriterCommonData_Stub);

    stub.set(SendWriterRequest, SendWriterRequest_Stub_Suc);
    EXPECT_EQ(m_libsmbCopyWriter->WriteMeta(fileHandle), SUCCESS);
    stub.reset(SendWriterRequest);

    stub.set(SendWriterRequest, SendWriterRequest_Stub_Fail);
    EXPECT_EQ(m_libsmbCopyWriter->WriteMeta(fileHandle), FAILED);
    stub.reset(SendWriterRequest);

    stub.reset(ADDR(LibsmbCopyWriter, GetSmbWriterCommonData));
}

/*
* 用例名称：CheckWriteData
* 前置条件：
* check点：检查WriteData返回值
*/
TEST_F(LibsmbCopyWriterTest, CheckWriteData)
{
    Stub stub;

    BackupParams params;
    params.srcEngine = BackupIOEngine::ARCHIVE_CLIENT;
    params.dstEngine = BackupIOEngine::POSIX;

    FileHandle fileHandle; 
    fileHandle.m_file = std::make_shared<FileDesc>(params.srcEngine, params.dstEngine);
    
    stub.set(ADDR(LibsmbCopyWriter, GetSmbWriterCommonData), GetSmbWriterCommonData_Stub);

    stub.set(SendWriterRequest, SendWriterRequest_Stub_Suc);
    EXPECT_EQ(m_libsmbCopyWriter->WriteData(fileHandle), SUCCESS);
    stub.reset(SendWriterRequest);

    stub.set(SendWriterRequest, SendWriterRequest_Stub_Fail);
    EXPECT_EQ(m_libsmbCopyWriter->WriteData(fileHandle), FAILED);
    stub.reset(SendWriterRequest);

    stub.reset(ADDR(LibsmbCopyWriter, GetSmbWriterCommonData));
}

/*
* 用例名称：CheckCloseFile
* 前置条件：
* check点：检查CloseFile返回值
*/
TEST_F(LibsmbCopyWriterTest, CheckCloseFile)
{
    Stub stub;

    BackupParams params;
    params.srcEngine = BackupIOEngine::ARCHIVE_CLIENT;
    params.dstEngine = BackupIOEngine::POSIX;

    FileHandle fileHandle; 
    fileHandle.m_file = std::make_shared<FileDesc>(params.srcEngine, params.dstEngine);
    
    stub.set(ADDR(LibsmbCopyWriter, GetSmbWriterCommonData), GetSmbWriterCommonData_Stub);
    stub.set(ADDR(Module::SmbContextWrapper, SmbCloseAsync), SmbCloseAsync_stub);
    
    stub.set(SendWriterRequest, SendWriterRequest_Stub_Suc);
    EXPECT_EQ(m_libsmbCopyWriter->CloseFile(fileHandle), SUCCESS);
    stub.reset(SendWriterRequest);

    stub.set(SendWriterRequest, SendWriterRequest_Stub_Fail);
    EXPECT_EQ(m_libsmbCopyWriter->CloseFile(fileHandle), FAILED);
    stub.reset(SendWriterRequest);

    stub.reset(ADDR(LibsmbCopyWriter, GetSmbWriterCommonData));
}

/*
* 用例名称：CheckDeleteFile
* 前置条件：
* check点：检查DeleteFile返回值
*/
TEST_F(LibsmbCopyWriterTest, CheckDeleteFile)
{
    Stub stub;

    BackupParams params;
    params.srcEngine = BackupIOEngine::ARCHIVE_CLIENT;
    params.dstEngine = BackupIOEngine::POSIX;

    FileHandle fileHandle; 
    fileHandle.m_file = std::make_shared<FileDesc>(params.srcEngine, params.dstEngine);
    
    stub.set(ADDR(LibsmbCopyWriter, GetSmbWriterCommonData), GetSmbWriterCommonData_Stub);

    stub.set(SendWriterRequest, SendWriterRequest_Stub_Suc);
    EXPECT_EQ(m_libsmbCopyWriter->DeleteFile(fileHandle), SUCCESS);
    stub.reset(SendWriterRequest);

    stub.set(SendWriterRequest, SendWriterRequest_Stub_Fail);
    EXPECT_EQ(m_libsmbCopyWriter->DeleteFile(fileHandle), FAILED);
    stub.reset(SendWriterRequest);

    stub.reset(ADDR(LibsmbCopyWriter, GetSmbWriterCommonData));
}

/*
* 用例名称：CheckServerCheck
* 前置条件：
* check点：检查ServerCheck返回值
*/
TEST_F(LibsmbCopyWriterTest, CheckServerCheck)
{
    Stub stub;
    stub.set(ADDR(PacketStats, GetValue), GetValue_Stub_DEFAULT_MAX_NOACCESS);
    EXPECT_EQ(m_libsmbCopyWriter->ServerCheck(), FAILED);
    stub.reset(ADDR(PacketStats, GetValue));

    m_libsmbCopyWriter->m_dstAdvParams->serverCheckMaxCount = 0;
    stub.set(ADDR(PacketStats, GetValue), GetValue_Stub_Zero);
    stub.set(HandleConnectionException, HandleConnectionException_Stub_Suc);
    EXPECT_EQ(m_libsmbCopyWriter->ServerCheck(), SUCCESS);
    stub.reset(ADDR(PacketStats, GetValue));
    stub.reset(HandleConnectionException);

    stub.set(ADDR(PacketStats, GetValue), GetValue_Stub_Zero);
    stub.set(HandleConnectionException, HandleConnectionException_Stub_Fail);
    EXPECT_EQ(m_libsmbCopyWriter->ServerCheck(), FAILED);
    stub.reset(ADDR(PacketStats, GetValue));
    stub.reset(HandleConnectionException);
}

/*
* 用例名称：CheckProcessTimers
* 前置条件：
* check点：检查ProcessTimers返回值
*/
TEST_F(LibsmbCopyWriterTest, CheckProcessTimers)
{
    EXPECT_NO_THROW(m_libsmbCopyWriter->ProcessTimers());
}


/*
* 用例名称：CheckThreadFunc
* 前置条件：
* check点：检查ThreadFunc返回值
*/
TEST_F(LibsmbCopyWriterTest, CheckThreadFunc)
{
    Stub stub;

    stub.set(ADDR(LibsmbCopyWriter, ServerCheck), ServerCheck_Stub_Fail);
    EXPECT_NO_THROW(m_libsmbCopyWriter->ThreadFunc());
    stub.reset(ADDR(LibsmbCopyWriter, ServerCheck));

    stub.set(ADDR(LibsmbCopyWriter, IsComplete), IsComplete_Stub_True);
    EXPECT_NO_THROW(m_libsmbCopyWriter->ThreadFunc());
    stub.reset(ADDR(LibsmbCopyWriter, IsComplete));

    stub.set(ADDR(LibsmbCopyWriter, IsComplete), IsComplete_Stub_False);
    stub.set(ADDR(LibsmbCopyWriter, IsAbort), IsAbort_Stub_True);
    EXPECT_NO_THROW(m_libsmbCopyWriter->ThreadFunc());
    stub.reset(ADDR(LibsmbCopyWriter, IsAbort));
    stub.reset(ADDR(LibsmbCopyWriter, IsComplete));

    stub.set(ADDR(LibsmbCopyWriter, ServerCheck), ServerCheck_Stub_Suc);
    stub.set(ADDR(LibsmbCopyWriter, IsComplete), IsComplete_Stub_False);
    stub.set(ADDR(LibsmbCopyWriter, IsAbort), IsAbort_Stub_False);
    stub.set(ADDR(PacketStats, GetValue), GetValue_Stub_Overmax);
    stub.set(ADDR(Module::SmbContextWrapper, Poll), Poll_Stub_Neg1);
    stub.set(ADDR(LibsmbCopyWriter, ProcessConnectionException), ProcessConnectionException_Stub_Fail);
    EXPECT_NO_THROW(m_libsmbCopyWriter->ThreadFunc());
    stub.reset(ADDR(LibsmbCopyWriter, ServerCheck));
    stub.reset(ADDR(LibsmbCopyWriter, IsComplete));
    stub.reset(ADDR(LibsmbCopyWriter, IsAbort));
    stub.reset(ADDR(PacketStats, GetValue));
    stub.reset(ADDR(Module::SmbContextWrapper, Poll));
    stub.reset(ADDR(LibsmbCopyWriter, ProcessConnectionException));

    stub.set(ADDR(LibsmbCopyWriter, ServerCheck), ServerCheck_Stub_Suc);
    stub.set(ADDR(LibsmbCopyWriter, IsComplete), IsComplete_Stub_False);
    stub.set(ADDR(LibsmbCopyWriter, IsAbort), IsAbort_Stub_False);
    stub.set(ADDR(Module::SmbContextWrapper, Poll), Poll_Stub_Neg1);
    stub.set(ADDR(LibsmbCopyWriter, ProcessWriteEntries), ProcessWriteEntries_Stub_Void);
    stub.set(ADDR(LibsmbCopyWriter, ProcessConnectionException), ProcessConnectionException_Stub_Fail);
    stub.set(ADDR(LibsmbCopyWriter, ClearWriteCache), ClearWriteCache_Stub_Void);
    stub.set(ADDR(LibsmbCopyWriter, SmbDisconnectContexts), SmbDisconnectContexts_Stub_Void);
    stub.set(ADDR(LibsmbCopyWriter, HandleComplete), HandleComplete_Stub_Void);
    EXPECT_NO_THROW(m_libsmbCopyWriter->ThreadFunc());
    stub.reset(ADDR(LibsmbCopyWriter, ServerCheck));
    stub.reset(ADDR(LibsmbCopyWriter, IsComplete));
    stub.reset(ADDR(LibsmbCopyWriter, IsAbort));
    stub.reset(ADDR(Module::SmbContextWrapper, Poll));
    stub.reset(ADDR(LibsmbCopyWriter, ProcessWriteEntries));
    stub.reset(ADDR(LibsmbCopyWriter, ProcessConnectionException));
    stub.reset(ADDR(LibsmbCopyWriter, ClearWriteCache));
    stub.reset(ADDR(LibsmbCopyWriter, SmbDisconnectContexts));
    stub.reset(ADDR(LibsmbCopyWriter, HandleComplete));
}

/*
* 用例名称：CheckProcessConnectionException
* 前置条件：
* check点：检查ProcessConnectionException返回值
*/
TEST_F(LibsmbCopyWriterTest, CheckProcessConnectionException)
{
    Stub stub;
    stub.set(HandleConnectionException, HandleConnectionException_Stub_Suc); 
    EXPECT_EQ(m_libsmbCopyWriter->ProcessConnectionException(), SUCCESS);
    stub.reset(HandleConnectionException);

    stub.set(HandleConnectionException, HandleConnectionException_Stub_Fail); 
    EXPECT_EQ(m_libsmbCopyWriter->ProcessConnectionException(), FAILED);
    stub.reset(HandleConnectionException);
}

/*
* 用例名称：CheckProcessReadEntries
* 前置条件：
* check点：检查ProcessReadEntries返回值
*/
TEST_F(LibsmbCopyWriterTest, CheckProcessReadEntries)
{
    Stub stub;

    BackupParams params;
    params.srcEngine = BackupIOEngine::ARCHIVE_CLIENT;
    params.dstEngine = BackupIOEngine::POSIX;

    FileHandle fileHandle; 
    fileHandle.m_file = std::make_shared<FileDesc>(params.srcEngine, params.dstEngine);

    m_libsmbCopyWriter->m_writeQueue->Push(fileHandle);

    stub.set(ADDR(LibsmbCopyWriter, IsAbort), IsAbort_Stub_True); 
    stub.set(ADDR(LibsmbCopyWriter, IsWriterRequestReachThreshold), IsWriterRequestReachThreshold_Stub_True); 
    stub.reset(ADDR(LibsmbCopyWriter, IsWriterRequestReachThreshold));
    stub.reset(ADDR(LibsmbCopyWriter, IsAbort));

    stub.set(ADDR(LibsmbCopyWriter, IsAbort), IsAbort_Stub_False); 
    stub.set(ADDR(LibsmbCopyWriter, IsWriterRequestReachThreshold), IsWriterRequestReachThreshold_Stub_False); 
    stub.set(ADDR(LibsmbCopyWriter, ProcessFileDescState), ProcessFileDescState_Stub_Void); 
    stub.reset(ADDR(LibsmbCopyWriter, IsWriterRequestReachThreshold));
    stub.reset(ADDR(LibsmbCopyWriter, IsAbort));
    stub.reset(ADDR(LibsmbCopyWriter, ProcessFileDescState)); 
}

/*
* 用例名称：CheckIsReaderRequestReachThreshold
* 前置条件：
* check点：检查IsReaderRequestReachThreshold返回值
*/
TEST_F(LibsmbCopyWriterTest, CheckIsReaderRequestReachThreshold)
{
    Stub stub;

    stub.set(ADDR(PacketStats, GetValue), GetValue_Stub_Overmax);
    EXPECT_EQ(m_libsmbCopyWriter->IsWriterRequestReachThreshold(), true);
    stub.reset(ADDR(PacketStats, GetValue));

    stub.set(ADDR(PacketStats, GetValue), GetValue_Stub_Zero);
    EXPECT_EQ(m_libsmbCopyWriter->IsWriterRequestReachThreshold(), false);
    stub.reset(ADDR(PacketStats, GetValue));
}

/*
* 用例名称：CheckProcessFileDescState
* 前置条件：
* check点：检查ProcessFileDescState返回值
*/
TEST_F(LibsmbCopyWriterTest, CheckProcessFileDescState)
{
    Stub stub;

    BackupParams params;
    params.srcEngine = BackupIOEngine::ARCHIVE_CLIENT;
    params.dstEngine = BackupIOEngine::POSIX;

    FileHandle fileHandle; 
    fileHandle.m_file = std::make_shared<FileDesc>(params.srcEngine, params.dstEngine);
    fileHandle.m_file->m_fileName = "/file.txt";
    fileHandle.m_file->m_size = 0;
    fileHandle.m_block.m_size = 0;
    fileHandle.m_block.m_offset = 0;
    fileHandle.m_block.m_seq = 1;
    fileHandle.m_file->m_blockStats.m_totalCnt = 1;

    typedef int (*fptr)(LibsmbCopyWriter*, FileHandle&);
    
    fptr LibsmbCopyWriter_OpenFile = (fptr)((int(LibsmbCopyWriter::*)(FileHandle&))&LibsmbCopyWriter::OpenFile);
    fptr LibsmbCopyWriter_WriteMeta = (fptr)((int(LibsmbCopyWriter::*)(FileHandle&))&LibsmbCopyWriter::WriteMeta);
    fptr LibsmbCopyWriter_WriteData = (fptr)((int(LibsmbCopyWriter::*)(FileHandle&))&LibsmbCopyWriter::WriteData);
    fptr LibsmbCopyWriter_CloseFile = (fptr)((int(LibsmbCopyWriter::*)(FileHandle&))&LibsmbCopyWriter::CloseFile);


    stub.set(LibsmbCopyWriter_OpenFile, OpenFile_Stub_Suc);
    stub.set(LibsmbCopyWriter_WriteMeta, WriteMeta_Stub_Suc);
    stub.set(LibsmbCopyWriter_WriteData, WriteData_Stub_Suc);
    stub.set(LibsmbCopyWriter_CloseFile, CloseFile_Stub_Suc);

    // enter state == FileDescState::INIT
    stub.set(ADDR(FileDesc, GetDstState), GetDstState_Stub_DST_CLOSED);
    m_libsmbCopyWriter->m_params.writeMeta = true;
    EXPECT_NO_THROW(m_libsmbCopyWriter->ProcessFileDescState(fileHandle));
    stub.reset(ADDR(FileDesc, GetDstState));

    stub.set(ADDR(FileDesc, GetDstState), GetDstState_Stub_DST_CLOSED);
    m_libsmbCopyWriter->m_params.writeMeta = false;
    EXPECT_NO_THROW(m_libsmbCopyWriter->ProcessFileDescState(fileHandle));
    stub.reset(ADDR(FileDesc, GetDstState));

    // enter state == FileDescState::INIT
    stub.set(ADDR(FileDesc, GetDstState), GetDstState_Stub_INIT);
    stub.set(ADDR(FileDesc, GetSrcState), GetSrcState_Stub_READ_FAILED);
    EXPECT_NO_THROW(m_libsmbCopyWriter->ProcessFileDescState(fileHandle));
    stub.reset(ADDR(FileDesc, GetSrcState));
    stub.reset(ADDR(FileDesc, GetDstState));

    stub.set(ADDR(FileDesc, GetDstState), GetDstState_Stub_INIT);
    stub.set(ADDR(FileDesc, GetSrcState), GetSrcState_Stub_INIT);
    // enter if branch 1
    fileHandle.m_file->SetFlag(IS_DIR);
    BackupQueueConfig config;
    config.maxSize = DEFAULT_BACKUP_QUEUE_SIZE;
    config.maxMemorySize = DEFAULT_BACKUP_QUEUE_MEMORY_SIZE;
    m_libsmbCopyWriter->m_dirQueue = std::make_shared<BackupQueue<FileHandle>>(config);
    EXPECT_NO_THROW(m_libsmbCopyWriter->ProcessFileDescState(fileHandle));
    // enter if branch 2
    fileHandle.m_file->ClearFlag(IS_DIR);
    stub.set(ADDR(LibsmbCopyWriter, IsOpenBlock), IsOpenBlock_Stub_True);
    EXPECT_NO_THROW(m_libsmbCopyWriter->ProcessFileDescState(fileHandle));
    stub.reset(ADDR(LibsmbCopyWriter, IsOpenBlock));
    // enter if branch 3
    stub.set(ADDR(LibsmbCopyWriter, IsOpenBlock), IsOpenBlock_Stub_False);
    EXPECT_NO_THROW(m_libsmbCopyWriter->ProcessFileDescState(fileHandle));
    stub.reset(ADDR(LibsmbCopyWriter, IsOpenBlock));
    stub.reset(ADDR(FileDesc, GetSrcState));
    stub.reset(ADDR(FileDesc, GetDstState));

    stub.set(ADDR(FileDesc, GetDstState), GetDstState_Stub_LSTAT);
    stub.set(IsFileReadOrWriteFailed, IsFileReadOrWriteFailed_Stub_True);
    stub.set(IsBackupTask, IsBackupTask_Stub_True);
    stub.set(ADDR(LibsmbCopyWriter, DeleteFile), DeleteFile_Stub_Suc);
    EXPECT_NO_THROW(m_libsmbCopyWriter->ProcessFileDescState(fileHandle));
    stub.reset(IsBackupTask);
    stub.reset(ADDR(LibsmbCopyWriter, DeleteFile));
    stub.set(IsBackupTask, IsBackupTask_Stub_False);
    EXPECT_NO_THROW(m_libsmbCopyWriter->ProcessFileDescState(fileHandle));
    stub.reset(IsBackupTask);
    stub.reset(IsFileReadOrWriteFailed);
    stub.reset(ADDR(FileDesc, GetDstState));

    stub.set(IsFileReadOrWriteFailed, IsFileReadOrWriteFailed_Stub_False);
    stub.set(ADDR(FileDesc, GetDstState), GetDstState_Stub_DST_OPENED);
    EXPECT_NO_THROW(m_libsmbCopyWriter->ProcessFileDescState(fileHandle));
    stub.reset(ADDR(FileDesc, GetDstState));
    stub.set(ADDR(FileDesc, GetDstState), GetDstState_Stub_PARTIAL_WRITED);
    EXPECT_NO_THROW(m_libsmbCopyWriter->ProcessFileDescState(fileHandle));
    stub.reset(ADDR(FileDesc, GetDstState));
    stub.set(ADDR(FileDesc, GetDstState), GetDstState_Stub_WRITED);
    EXPECT_NO_THROW(m_libsmbCopyWriter->ProcessFileDescState(fileHandle));
    stub.reset(ADDR(FileDesc, GetDstState));
    stub.reset(IsFileReadOrWriteFailed);
    
    stub.reset(LibsmbCopyWriter_OpenFile);
    stub.reset(LibsmbCopyWriter_WriteMeta);
    stub.reset(LibsmbCopyWriter_WriteData);
    stub.reset(LibsmbCopyWriter_CloseFile);
}

/*
* 用例名称：CheckIsOpenBlock
* 前置条件：
* check点：检查IsOpenBlock返回值
*/
TEST_F(LibsmbCopyWriterTest, CheckIsOpenBlock)
{
    BackupParams params;
    params.srcEngine = BackupIOEngine::ARCHIVE_CLIENT;
    params.dstEngine = BackupIOEngine::POSIX;

    FileHandle fileHandle; 
    fileHandle.m_file = std::make_shared<FileDesc>(params.srcEngine, params.dstEngine);
    fileHandle.m_block.m_size = 0;
    fileHandle.m_block.m_seq = 0;
    EXPECT_EQ(m_libsmbCopyWriter->IsOpenBlock(fileHandle), true);

    fileHandle.m_block.m_size = 1;
    fileHandle.m_block.m_seq = 1;
    EXPECT_EQ(m_libsmbCopyWriter->IsOpenBlock(fileHandle), false);
}

static bool IsMkdirComplete_Stub_True(void *obj)
{
    return true;
}

static bool IsMkdirComplete_Stub_False(void *obj)
{
    return false;
}

static int SendWriterSyncRequest_Stub_Suc(void *obj, FileHandle &fileHandle, SmbWriterCommonData *cbData, LibsmbEvent event)
{
    return SUCCESS;
}

static void SmbDisconnectSyncContexts_Stub_Void(void *obj)
{
    return;
}

/*
* 用例名称：CheckThreadFunc
* 前置条件：
* check点：检查ThreadFunc返回值
*/
TEST_F(LibsmbCopyWriterTest, CheckSyncThreadFunc)
{
    Stub stub;
    BackupQueueConfig config;
    config.maxSize = DEFAULT_BACKUP_QUEUE_SIZE;
    config.maxMemorySize = DEFAULT_BACKUP_QUEUE_MEMORY_SIZE;
    m_libsmbCopyWriter->m_dirQueue = std::make_shared<BackupQueue<FileHandle>>(config);
    stub.set(ADDR(LibsmbCopyWriter, SmbDisconnectSyncContexts), SmbDisconnectSyncContexts_Stub_Void);

    // enter IsMkdirComplete
    stub.set(ADDR(LibsmbCopyWriter, IsMkdirComplete), IsMkdirComplete_Stub_True);
    EXPECT_NO_THROW(m_libsmbCopyWriter->SyncThreadFunc());
    stub.reset(ADDR(LibsmbCopyWriter, IsMkdirComplete));

    //enter IsAbort
    stub.set(ADDR(LibsmbCopyWriter, IsMkdirComplete), IsMkdirComplete_Stub_False);
    stub.set(ADDR(LibsmbCopyWriter, IsAbort), IsAbort_Stub_True);
    EXPECT_NO_THROW(m_libsmbCopyWriter->SyncThreadFunc());
    stub.reset(ADDR(LibsmbCopyWriter, IsMkdirComplete));
    stub.reset(ADDR(LibsmbCopyWriter, IsAbort));

    // // enter if (ret)
    // stub.set(ADDR(LibsmbCopyWriter, IsMkdirComplete), IsMkdirComplete_Stub_False);
    // stub.set(ADDR(LibsmbCopyWriter, IsAbort), IsAbort_Stub_False);
    // stub.set(ADDR(LibsmbCopyWriter, GetSmbWriterCommonData), GetSmbWriterCommonData_Stub);
    // stub.set(SendWriterSyncRequest, SendWriterSyncRequest_Stub_Suc);
    // // fileHandle.m_file->ClearFlag(IS_DIR);
    // stub.set(ADDR(FileDesc, GetDstState), GetDstState_Stub_DIR_DEL);
    // EXPECT_NO_THROW(m_libsmbCopyWriter->SyncThreadFunc());
    // stub.reset(ADDR(FileDesc, GetDstState));
    // // fileHandle.m_file->SetFlag(IS_DIR);
    // stub.set(ADDR(FileDesc, GetDstState), GetDstState_Stub_INIT);
    // EXPECT_NO_THROW(m_libsmbCopyWriter->SyncThreadFunc());
    // stub.reset(ADDR(FileDesc, GetDstState));
    // stub.reset(ADDR(LibsmbCopyWriter, IsMkdirComplete));
    // stub.reset(ADDR(LibsmbCopyWriter, IsAbort));
    // stub.reset(ADDR(LibsmbCopyWriter, GetSmbWriterCommonData));
    // stub.reset(SendWriterSyncRequest);

    stub.reset(ADDR(LibsmbCopyWriter, SmbDisconnectSyncContexts));
}

/*
* 用例名称：CheckFillSmbWriterCommonData
* 前置条件：
* check点：检查FillSmbWriterCommonData返回值
*/
TEST_F(LibsmbCopyWriterTest, CheckFillSmbWriterCommonData)
{
    m_libsmbCopyWriter->m_asyncContext = std::shared_ptr<Module::SmbContextWrapper>();
    m_libsmbCopyWriter->m_syncContext = std::shared_ptr<Module::SmbContextWrapper>();
    m_libsmbCopyWriter->m_params.writeMeta = true;
    m_libsmbCopyWriter->m_pktStats = std::shared_ptr<PacketStats>();
    auto writerCommonData = new(nothrow) SmbWriterCommonData();
    EXPECT_NO_THROW(m_libsmbCopyWriter->FillSmbWriterCommonData(writerCommonData));
    delete writerCommonData;
}

static void FillSmbWriterCommonData_Stub_Void(void *obj)
{
    return;
}

/*
* 用例名称：CheckGetSmbWriterCommonData
* 前置条件：
* check点：检查GetSmbWriterCommonData返回值
*/
TEST_F(LibsmbCopyWriterTest, CheckGetSmbWriterCommonData)
{
    Stub stub;
    FileHandle fileHandle;

    stub.set(ADDR(LibsmbCopyWriter, FillSmbWriterCommonData), FillSmbWriterCommonData_Stub_Void);
    EXPECT_NO_THROW(m_libsmbCopyWriter->GetSmbWriterCommonData(fileHandle));
    stub.reset(ADDR(LibsmbCopyWriter, FillSmbWriterCommonData));
}

/*
* 用例名称：CheckClearWriteCache
* 前置条件：
* check点：检查ClearWriteCache返回值
*/
TEST_F(LibsmbCopyWriterTest, CheckClearWriteCache)
{
    Stub stub;

    EXPECT_NO_THROW(m_libsmbCopyWriter->ClearWriteCache());

    BackupParams params;
    params.srcEngine = BackupIOEngine::ARCHIVE_CLIENT;
    params.dstEngine = BackupIOEngine::POSIX;
    FileHandle fileHandle;
    fileHandle.m_file = std::make_shared<FileDesc>(params.srcEngine, params.dstEngine);
    m_libsmbCopyWriter->m_writeCache = {{"1", {fileHandle}}};
    stub.set(ADDR(LibsmbCopyWriter, FillSmbWriterCommonData), FillSmbWriterCommonData_Stub_Void);
    EXPECT_NO_THROW(m_libsmbCopyWriter->ClearWriteCache());
    stub.reset(ADDR(LibsmbCopyWriter, FillSmbWriterCommonData));
}

/*
* 用例名称：CheckClearWriteCacheStateOpened
* 前置条件：
* check点：检查ClearWriteCacheStateOpened返回值
*/
TEST_F(LibsmbCopyWriterTest, CheckClearWriteCacheStateOpened)
{
    Stub stub;

    BackupParams params;
    params.srcEngine = BackupIOEngine::ARCHIVE_CLIENT;
    params.dstEngine = BackupIOEngine::POSIX;
    FileHandle fileHandle;
    fileHandle.m_file = std::make_shared<FileDesc>(params.srcEngine, params.dstEngine);
    std::vector<FileHandle> fileHandles = {fileHandle};

    typedef int (*fptr)(LibsmbCopyWriter*, FileHandle&);
    fptr LibsmbCopyWriter_WriteData = (fptr)((int(LibsmbCopyWriter::*)(FileHandle&))&LibsmbCopyWriter::WriteData);
    stub.set(LibsmbCopyWriter_WriteData, WriteData_Stub_Suc);
    EXPECT_NO_THROW(m_libsmbCopyWriter->ClearWriteCacheStateOpened(fileHandles));
    stub.reset(LibsmbCopyWriter_WriteData);

    fileHandle.m_file->m_size = MAX_SMALL_FILE_SIZE + 1;
    fileHandles = {fileHandle};
    EXPECT_NO_THROW(m_libsmbCopyWriter->ClearWriteCacheStateOpened(fileHandles));
}