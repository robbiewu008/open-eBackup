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
#include "mockcpp/mockcpp.hpp"
#include "llt_stub/named_stub.h"
#include "llt_stub/addr_pri.h"
#include "LibsmbCopyReader.h"

#include "SmbContextWrapper.h"

using namespace std;
using namespace Module;
using namespace FS_Backup;

namespace {
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

class LibsmbCopyReaderTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();

    std::unique_ptr<LibsmbCopyReader> m_libsmbCopyReader = nullptr;
};

void LibsmbCopyReaderTest::SetUp()
{

    BackupQueueConfig config {DEFAULT_BACKUP_QUEUE_SIZE, DEFAULT_BACKUP_QUEUE_MEMORY_SIZE};
    BackupParams params;
    params.srcEngine = BackupIOEngine::LIBSMB;
    params.dstEngine = BackupIOEngine::LIBSMB;
    params.srcAdvParams = std::make_shared<LibsmbBackupAdvanceParams>();
    params.dstAdvParams = std::make_shared<LibsmbBackupAdvanceParams>();

    ReaderParams copyReaderParams {};
    copyReaderParams.backupParams = params;
    copyReaderParams.readQueuePtr = std::make_shared<BackupQueue<FileHandle>>(config);
    copyReaderParams.aggregateQueuePtr = std::make_shared<BackupQueue<FileHandle>>(config);
    copyReaderParams.controlInfo = std::make_shared<BackupControlInfo>();
    copyReaderParams.blockBufferMap = std::make_shared<BlockBufferMap>();

    m_libsmbCopyReader = std::make_unique<LibsmbCopyReader>(copyReaderParams);

}

void LibsmbCopyReaderTest::TearDown()
{
    GlobalMockObject::verify(); // 校验mock规范并清除mock规范
}

void LibsmbCopyReaderTest::SetUpTestCase()
{}

void LibsmbCopyReaderTest::TearDownTestCase()
{}

FileDescState GetSrcState_Ret_META_READED()
{
    return FileDescState::META_READED;
}

FileDescState GetSrcState_Ret_SRC_OPENED()
{
    return FileDescState::SRC_OPENED;
}

FileDescState GetSrcState_Ret_READED()
{
    return FileDescState::READED;
}

void FillSmbReaderCommonData_Void(SmbReaderCommonData *readerCommonData)
{}

std::shared_ptr<Module::SmbContextWrapper> SmbConnectContext_stub1(const Module::SmbContextArgs &args)
{
    return nullptr;
}

int SmbDisconnectContext_stub1(std::shared_ptr<Module::SmbContextWrapper> context)
{
    return 0;
}

/*
 * 用例名称：验证LibsmbCopyReader线程启动
 * 前置条件：无
 * check点：线程正常启动后，正常退出
 */
// TEST_F(LibsmbCopyReaderTest, Start) {
//     BackupParams backupParams;
//     backupParams.srcAdvParams = std::make_shared<LibsmbBackupAdvanceParams>();
//     std::shared_ptr<BackupControlInfo> backupControlInfo = std::make_shared<BackupControlInfo>();
//     LibsmbCopyReader libsmbCopyReader(backupParams, nullptr, nullptr, backupControlInfo, nullptr);

//     Stub stub;
//     stub.set(SmbConnectContext, SmbConnectContext_stub1);
//     stub.set(SmbDisconnectContext, SmbDisconnectContext_stub1);
//     // smb初始化失败，start返回failed
//     EXPECT_EQ(libsmbCopyReader.Start(), BackupRetCode::FAILED);
//     stub.reset(SmbConnectContext);
//     stub.reset(SmbDisconnectContext);
// }

int SmbConnectContexts_SUCCESS()
{
    return SUCCESS;
}
int Poll_Ret(int expireTime)
{
    return 0;
}

static bool IsReaderRequestReachThreshold_Stub_True(void *obj)
{
    return true;
}

static bool IsReaderRequestReachThreshold_Stub_False(void *obj)
{
    return false;
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

static void SmbDisconnectContexts_Stub_Void(void *obj)
{
    return;
}

static void HandleComplete_Stub_Void(void *obj)
{
    return;
}

static int Poll_Stub_1(void *obj)
{
    return 1;
}

static int Poll_Stub_Neg1(void *obj)
{
    return -1;
}

static int HandleConnectionException_Stub_Suc(void *obj, std::shared_ptr<SmbContextWrapper> &smbContext,
    SmbContextArgs &contextArgs, int connectRetryTimes) {
    return SUCCESS;
}

static int HandleConnectionException_Stub_Fail(void *obj, std::shared_ptr<SmbContextWrapper> &smbContext,
    SmbContextArgs &contextArgs, int connectRetryTimes) {
    return FAILED;
}

static int OpenFile_Stub_Suc(void *obj, FileHandle &fileHandle)
{
    return SUCCESS;
}

static int ReadData_Stub_Suc(void *obj, FileHandle &fileHandle)
{
    return SUCCESS;
}

static int CloseFile_Stub_Suc(void *obj, FileHandle &fileHandle)
{
    return SUCCESS;
}

static uint64_t GetValue_Stub_Overmax(void *obj, PKT_TYPE reqType, PKT_COUNTER counterType)
{
    return MAX_PENDING_REQUEST_COUNT + 1;
}

static uint64_t GetTotalBufferSize_Stub_MEMORY_THRESHOLD_HIGH(void *obj)
{
    return MEMORY_THRESHOLD_HIGH + 1;
}

static uint64_t GetTotalBufferSize_Stub_Zero(void *obj)
{
    return 0;
}

static void ProcessFileDescState_Stub_Void(void *obj, FileHandle fileHandle)
{
    return;
}

static FileDescState GetSrcState_Stub_AGGREGATED(void *obj)
{
    return FileDescState::AGGREGATED;
}

static FileDescState GetSrcState_Stub_WRITE_SKIP(void *obj)
{
    return FileDescState::WRITE_SKIP;
}

static FileDescState GetSrcState_Stub_INIT(void *obj)
{
    return FileDescState::INIT;
}

static FileDescState GetSrcState_Stub_META_READED(void *obj)
{
    return FileDescState::META_READED;
}

static FileDescState GetSrcState_Stub_SRC_OPENED(void *obj)
{
    return FileDescState::SRC_OPENED;
}

static FileDescState GetSrcState_Stub_PARTIAL_READED(void *obj)
{
    return FileDescState::PARTIAL_READED;
}

static FileDescState GetSrcState_Stub_READED(void *obj)
{
    return FileDescState::READED;
}

static SmbReaderCommonData* GetSmbReaderCommonData_Stub(void *obj, FileHandle &fileHandle)
{
    auto cbData = new(nothrow) SmbReaderCommonData();
    return cbData;
}

static SmbReaderCommonData* GetSmbReaderCommonData_Stub1(void *obj, FileHandle &fileHandle)
{
    auto cbData = nullptr;
    return cbData;
}

static int SendReaderRequest_Stub_Suc(void *obj, FileHandle &fileHandle, SmbReaderCommonData *cbData, LibsmbEvent event)
{
    return SUCCESS;
}

static int SendReaderRequest_Stub_Fail(void *obj, FileHandle &fileHandle, SmbReaderCommonData *cbData, LibsmbEvent event)
{
    return FAILED;
}

static int SmbCloseAsync_Stub(void *obj)
{
    return SUCCESS;
}

TEST_F(LibsmbCopyReaderTest, Start)
{

    BackupRetCode ret = m_libsmbCopyReader->Start();
    EXPECT_EQ(ret, BackupRetCode::FAILED);
}

/*
* 用例名称：SmbConnectContexts
* 前置条件：
* check点：Create Context 
*/
TEST_F(LibsmbCopyReaderTest, SmbConnectContexts)
{
    m_libsmbCopyReader->SmbDisconnectContexts();
    m_libsmbCopyReader->GetStatus();
    m_libsmbCopyReader->HandleComplete();
    m_libsmbCopyReader->ServerCheck();

    int ret = m_libsmbCopyReader->SmbConnectContexts();
    EXPECT_EQ(ret, -1);
}

/*
* 用例名称：ProcessFileDescState
* 前置条件：
* check点：根据文件状态进行处理
*/
// TEST_F(LibsmbCopyReaderTest, ProcessFileDescState)
// { 

//     Stub stub;
//     BackupParams params;
//     params.srcEngine = BackupIOEngine::ARCHIVE_CLIENT;
//     params.dstEngine = BackupIOEngine::POSIX;

//     FileHandle fileHandle; 
//     fileHandle.m_file = std::make_shared<FileDesc>(params.srcEngine, params.dstEngine);
//     fileHandle.m_file->m_fileName = "/file.txt";
//     fileHandle.m_file->m_size = 0;

//     stub.set(ADDR(LibsmbCopyReader, FillSmbReaderCommonData), FillSmbReaderCommonData_Void);  
//     m_libsmbCopyReader->ProcessFileDescState(fileHandle);

//     // stub.set(ADDR(FileDesc, GetSrcState), GetSrcState_Ret_META_READED);  
//     // m_libsmbCopyReader->ProcessFileDescState(fileHandle);
//     // stub.reset(ADDR(FileDesc, GetSrcState));
  
//     // stub.set(ADDR(FileDesc, GetSrcState), GetSrcState_Ret_SRC_OPENED);  
//     // m_libsmbCopyReader->ProcessFileDescState(fileHandle);
//     // stub.reset(ADDR(FileDesc, GetSrcState));

//     // stub.set(ADDR(FileDesc, GetSrcState), GetSrcState_Ret_READED);  
//     // m_libsmbCopyReader->ProcessFileDescState(fileHandle);
//     // stub.reset(ADDR(FileDesc, GetSrcState));

//     stub.reset(ADDR(LibsmbCopyReader, FillSmbReaderCommonData));
// }

/*
* 用例名称：ReadNormalData
* 前置条件：
* check点：读正常数据
*/
// TEST_F(LibsmbCopyReaderTest, ReadNormalData)
// {

//     Stub stub;
//     stub.set(ADDR(LibsmbCopyReader, FillSmbReaderCommonData), FillSmbReaderCommonData_Void);  
    
//     BackupParams params;
//     params.srcEngine = BackupIOEngine::ARCHIVE_CLIENT;
//     params.dstEngine = BackupIOEngine::POSIX;

//     FileHandle fileHandle; 
//     fileHandle.m_file = std::make_shared<FileDesc>(params.srcEngine, params.dstEngine);
//     fileHandle.m_file->m_fileName = "/file.txt";
//     fileHandle.m_file->m_size = 4194304*3;
//     fileHandle.m_block.m_size = 3;
//     fileHandle.m_block.m_offset = 0;
//     fileHandle.m_block.m_seq = 1;
//     fileHandle.m_file->m_blockStats.m_totalCnt = 1;

//     int ret = m_libsmbCopyReader->ReadNormalData(fileHandle);
//     EXPECT_EQ(ret, SUCCESS);

//     stub.reset(ADDR(LibsmbCopyReader, FillSmbReaderCommonData));
// }

/*
* 用例名称：OpenFile
* 前置条件：
* check点：空文件跳过
*/
TEST_F(LibsmbCopyReaderTest, OpenFile)
{
    BackupParams params;
    params.srcEngine = BackupIOEngine::LIBSMB;
    params.dstEngine = BackupIOEngine::LIBSMB;

    string fileName = "file.txt";

    FileHandle fileHandle; 
    fileHandle.m_file = std::make_shared<FileDesc>(params.srcEngine, params.dstEngine);
    fileHandle.m_file->m_fileName = fileName;
    fileHandle.m_file->m_size = 0;
    fileHandle.m_block.m_size = 0;
    fileHandle.m_block.m_offset = 0;
    fileHandle.m_block.m_seq = 0;
    fileHandle.m_file->m_blockStats.m_totalCnt = 1;

    int ret = m_libsmbCopyReader->OpenFile(fileHandle);

    FileHandle fileHandle2;
    m_libsmbCopyReader->m_aggregateQueue->WaitAndPop(fileHandle2);
    EXPECT_EQ(fileHandle2.m_file->m_fileName, fileName);
    EXPECT_EQ(ret, Module::SUCCESS);
    EXPECT_EQ(m_libsmbCopyReader->m_controlInfo->m_readProduce, 1);
}

/*
* 用例名称：CheckAbort
* 前置条件：
* check点：检查Abort返回值
*/
TEST_F(LibsmbCopyReaderTest, CheckAbort)
{
    EXPECT_EQ(m_libsmbCopyReader->Abort(), BackupRetCode::SUCCESS);
}

/*
* 用例名称：CheckGetStatus
* 前置条件：
* check点：检查GetStatus返回值
*/
TEST_F(LibsmbCopyReaderTest, CheckGetStatus)
{
    m_libsmbCopyReader->m_controlInfo->m_readPhaseComplete = true;
    m_libsmbCopyReader->m_abort = true;
    EXPECT_EQ(m_libsmbCopyReader->GetStatus(), BackupPhaseStatus::ABORTED);

    m_libsmbCopyReader->m_abort = false;
    m_libsmbCopyReader->m_controlInfo->m_controlReaderFailed = true;
    EXPECT_EQ(m_libsmbCopyReader->GetStatus(), BackupPhaseStatus::FAILED);

    m_libsmbCopyReader->m_failed = false;
    m_libsmbCopyReader->m_controlInfo->m_failed = false;
    m_libsmbCopyReader->m_controlInfo->m_controlReaderFailed = false;
    EXPECT_EQ(m_libsmbCopyReader->GetStatus(), BackupPhaseStatus::COMPLETED);
}

/*
* 用例名称：CheckIsAbort
* 前置条件：
* check点：检查IsAbort返回值
*/
TEST_F(LibsmbCopyReaderTest, CheckIsAbort)
{
    m_libsmbCopyReader->m_abort = true;
    m_libsmbCopyReader->m_failed = true;
    m_libsmbCopyReader->m_controlInfo->m_failed = true;
    m_libsmbCopyReader->m_controlInfo->m_controlReaderFailed = true;
    EXPECT_EQ(m_libsmbCopyReader->IsAbort(), true);

    m_libsmbCopyReader->m_abort = false;
    m_libsmbCopyReader->m_failed = false;
    m_libsmbCopyReader->m_controlInfo->m_failed = false;
    m_libsmbCopyReader->m_controlInfo->m_controlReaderFailed = false;
    EXPECT_EQ(m_libsmbCopyReader->IsAbort(), false);
}

/*
* 用例名称：CheckIsComplete
* 前置条件：
* check点：检查IsComplete返回值
*/
TEST_F(LibsmbCopyReaderTest, CheckIsComplete)
{
    EXPECT_NO_THROW(m_libsmbCopyReader->IsComplete());
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

/*
* 用例名称：CheckServerCheck
* 前置条件：
* check点：检查ServerCheck返回值
*/
TEST_F(LibsmbCopyReaderTest, CheckServerCheck)
{
    LLTSTUB::Stub stub;
    stub.set(ADDR(PacketStats, GetValue), GetValue_Stub_DEFAULT_MAX_NOACCESS);
    EXPECT_EQ(m_libsmbCopyReader->ServerCheck(), FAILED);
    stub.reset(ADDR(PacketStats, GetValue));

    m_libsmbCopyReader->m_srcAdvParams->serverCheckMaxCount = 0;
    stub.set(ADDR(PacketStats, GetValue), GetValue_Stub_Zero);
    stub.set(HandleConnectionException, HandleConnectionException_Stub_Suc);
    EXPECT_EQ(m_libsmbCopyReader->ServerCheck(), SUCCESS);
    stub.reset(ADDR(PacketStats, GetValue));
    stub.reset(HandleConnectionException);

    stub.set(ADDR(PacketStats, GetValue), GetValue_Stub_Zero);
    stub.set(HandleConnectionException, HandleConnectionException_Stub_Fail);
    EXPECT_EQ(m_libsmbCopyReader->ServerCheck(), FAILED);
    stub.reset(ADDR(PacketStats, GetValue));
    stub.reset(HandleConnectionException);
}

/*
* 用例名称：CheckProcessTimers
* 前置条件：
* check点：检查ProcessTimers返回值
*/
TEST_F(LibsmbCopyReaderTest, CheckProcessTimers)
{
    LLTSTUB::Stub stub;
    stub.set(ADDR(LibsmbCopyReader, IsReaderRequestReachThreshold), IsReaderRequestReachThreshold_Stub_True); 
    EXPECT_EQ(m_libsmbCopyReader->ProcessTimers(), 0);
    stub.reset(ADDR(LibsmbCopyReader, IsReaderRequestReachThreshold));

    stub.set(ADDR(LibsmbCopyReader, IsReaderRequestReachThreshold), IsReaderRequestReachThreshold_Stub_False); 
    EXPECT_NO_THROW(m_libsmbCopyReader->ProcessTimers());
    stub.reset(ADDR(LibsmbCopyReader, IsReaderRequestReachThreshold));
}


/*
* 用例名称：CheckThreadFunc
* 前置条件：
* check点：检查ThreadFunc返回值
*/
TEST_F(LibsmbCopyReaderTest, CheckThreadFunc)
{
    LLTSTUB::Stub stub;

    stub.set(ADDR(LibsmbCopyReader, ServerCheck), ServerCheck_Stub_Fail);
    EXPECT_NO_THROW(m_libsmbCopyReader->ThreadFunc());
    stub.reset(ADDR(LibsmbCopyReader, ServerCheck));

    stub.set(ADDR(LibsmbCopyReader, IsComplete), IsComplete_Stub_True);
    EXPECT_NO_THROW(m_libsmbCopyReader->ThreadFunc());
    stub.reset(ADDR(LibsmbCopyReader, IsComplete));

    stub.set(ADDR(LibsmbCopyReader, IsAbort), IsAbort_Stub_True);
    EXPECT_NO_THROW(m_libsmbCopyReader->ThreadFunc());
    stub.reset(ADDR(LibsmbCopyReader, IsAbort));

    stub.set(ADDR(LibsmbCopyReader, ServerCheck), ServerCheck_Stub_Suc);
    stub.set(ADDR(LibsmbCopyReader, IsComplete), IsComplete_Stub_False);
    stub.set(ADDR(LibsmbCopyReader, IsAbort), IsAbort_Stub_False);
    stub.set(ADDR(PacketStats, GetValue), GetValue_Stub_Overmax);
    stub.set(ADDR(Module::SmbContextWrapper, Poll), Poll_Stub_Neg1);
    stub.set(ADDR(LibsmbCopyReader, ProcessConnectionException), ProcessConnectionException_Stub_Fail);
    EXPECT_NO_THROW(m_libsmbCopyReader->ThreadFunc());
    stub.reset(ADDR(LibsmbCopyReader, ServerCheck));
    stub.reset(ADDR(LibsmbCopyReader, IsComplete));
    stub.reset(ADDR(LibsmbCopyReader, IsAbort));
    stub.reset(ADDR(PacketStats, GetValue));
    stub.reset(ADDR(Module::SmbContextWrapper, Poll));
    stub.reset(ADDR(LibsmbCopyReader, ProcessConnectionException));

    stub.set(ADDR(LibsmbCopyReader, ServerCheck), ServerCheck_Stub_Suc);
    stub.set(ADDR(LibsmbCopyReader, IsComplete), IsComplete_Stub_False);
    stub.set(ADDR(LibsmbCopyReader, IsAbort), IsAbort_Stub_False);
    stub.set(ADDR(Module::SmbContextWrapper, Poll), Poll_Stub_Neg1);
    stub.set(ADDR(LibsmbCopyReader, ProcessConnectionException), ProcessConnectionException_Stub_Fail);
    stub.set(ADDR(LibsmbCopyReader, SmbDisconnectContexts), SmbDisconnectContexts_Stub_Void);
    stub.set(ADDR(LibsmbCopyReader, HandleComplete), HandleComplete_Stub_Void);
    EXPECT_NO_THROW(m_libsmbCopyReader->ThreadFunc());
    stub.reset(ADDR(LibsmbCopyReader, ServerCheck));
    stub.reset(ADDR(LibsmbCopyReader, IsComplete));
    stub.reset(ADDR(LibsmbCopyReader, IsAbort));
    stub.reset(ADDR(Module::SmbContextWrapper, Poll));
    stub.reset(ADDR(LibsmbCopyReader, ProcessConnectionException));
    stub.reset(ADDR(LibsmbCopyReader, SmbDisconnectContexts));
    stub.reset(ADDR(LibsmbCopyReader, HandleComplete));
}

/*
* 用例名称：CheckProcessConnectionException
* 前置条件：
* check点：检查ProcessConnectionException返回值
*/
TEST_F(LibsmbCopyReaderTest, CheckProcessConnectionException)
{
    LLTSTUB::Stub stub;
    stub.set(HandleConnectionException, HandleConnectionException_Stub_Suc); 
    EXPECT_EQ(m_libsmbCopyReader->ProcessConnectionException(), SUCCESS);
    stub.reset(HandleConnectionException);

    stub.set(HandleConnectionException, HandleConnectionException_Stub_Fail); 
    EXPECT_EQ(m_libsmbCopyReader->ProcessConnectionException(), FAILED);
    stub.reset(HandleConnectionException);
}

/*
* 用例名称：CheckProcessReadEntries
* 前置条件：
* check点：检查ProcessReadEntries返回值
*/
TEST_F(LibsmbCopyReaderTest, CheckProcessReadEntries)
{
    LLTSTUB::Stub stub;

    BackupParams params;
    params.srcEngine = BackupIOEngine::ARCHIVE_CLIENT;
    params.dstEngine = BackupIOEngine::POSIX;

    FileHandle fileHandle; 
    fileHandle.m_file = std::make_shared<FileDesc>(params.srcEngine, params.dstEngine);

    m_libsmbCopyReader->m_readQueue->Push(fileHandle);

    stub.set(ADDR(LibsmbCopyReader, IsAbort), IsAbort_Stub_True); 
    EXPECT_NO_THROW(m_libsmbCopyReader->ProcessReadEntries());
    stub.reset(ADDR(LibsmbCopyReader, IsAbort));

    stub.set(ADDR(LibsmbCopyReader, IsAbort), IsAbort_Stub_False); 
    stub.set(ADDR(LibsmbCopyReader, IsReaderRequestReachThreshold), IsReaderRequestReachThreshold_Stub_True); 
    EXPECT_NO_THROW(m_libsmbCopyReader->ProcessReadEntries());
    stub.reset(ADDR(LibsmbCopyReader, IsReaderRequestReachThreshold));
    stub.reset(ADDR(LibsmbCopyReader, IsAbort));

    stub.set(ADDR(LibsmbCopyReader, IsAbort), IsAbort_Stub_False); 
    stub.set(ADDR(LibsmbCopyReader, IsReaderRequestReachThreshold), IsReaderRequestReachThreshold_Stub_False); 
    stub.set(ADDR(LibsmbCopyReader, ProcessFileDescState), ProcessFileDescState_Stub_Void); 
    EXPECT_NO_THROW(m_libsmbCopyReader->ProcessReadEntries());
    stub.reset(ADDR(LibsmbCopyReader, IsReaderRequestReachThreshold));
    stub.reset(ADDR(LibsmbCopyReader, IsAbort));
    stub.reset(ADDR(LibsmbCopyReader, ProcessFileDescState)); 
}

static bool Empty_Stub_False(void *obj)
{
    return false;
}
/*
* 用例名称：CheckProcessPartialReadEntries
* 前置条件：
* check点：检查ProcessPartialReadEntries返回值
*/
TEST_F(LibsmbCopyReaderTest, CheckProcessPartialReadEntries)
{
    LLTSTUB::Stub stub;

    BackupParams params;
    params.srcEngine = BackupIOEngine::ARCHIVE_CLIENT;
    params.dstEngine = BackupIOEngine::POSIX;

    FileHandle fileHandle; 
    fileHandle.m_file = std::make_shared<FileDesc>(params.srcEngine, params.dstEngine);
    BackupQueueConfig config {DEFAULT_BACKUP_QUEUE_SIZE, DEFAULT_BACKUP_QUEUE_MEMORY_SIZE};
    
    m_libsmbCopyReader->m_partialReadQueue = std::make_shared<BackupQueue<FileHandle>>(config);
    m_libsmbCopyReader->m_partialReadQueue->WaitAndPush(fileHandle);

    stub.set(ADDR(LibsmbCopyReader, ProcessFileDescState), ProcessFileDescState_Stub_Void); 
    
    stub.set(ADDR(BlockBufferMap, GetTotalBufferSize), GetTotalBufferSize_Stub_MEMORY_THRESHOLD_HIGH);
    stub.set(ADDR(PacketStats, GetValue), GetValue_Stub_MAX_PENDING_REQUEST_COUNT);
    EXPECT_NO_THROW(m_libsmbCopyReader->ProcessPartialReadEntries());
    stub.reset(ADDR(PacketStats, GetValue));
    stub.reset(ADDR(BlockBufferMap, GetTotalBufferSize));
    
    stub.set(ADDR(BlockBufferMap, GetTotalBufferSize), GetTotalBufferSize_Stub_Zero);
    stub.set(ADDR(PacketStats, GetValue), GetValue_Stub_Zero);
    EXPECT_NO_THROW(m_libsmbCopyReader->ProcessPartialReadEntries());
    stub.reset(ADDR(PacketStats, GetValue));
    stub.reset(ADDR(BlockBufferMap, GetTotalBufferSize));

    stub.reset(ADDR(LibsmbCopyReader, ProcessFileDescState));

}

/*
* 用例名称：CheckIsReaderRequestReachThreshold
* 前置条件：
* check点：检查IsReaderRequestReachThreshold返回值
*/
TEST_F(LibsmbCopyReaderTest, CheckIsReaderRequestReachThreshold)
{
    LLTSTUB::Stub stub;

    stub.set(ADDR(BlockBufferMap, GetTotalBufferSize), GetTotalBufferSize_Stub_MEMORY_THRESHOLD_HIGH);
    stub.set(ADDR(PacketStats, GetValue), GetValue_Stub_Overmax);
    EXPECT_EQ(m_libsmbCopyReader->IsReaderRequestReachThreshold(), true);
    stub.reset(ADDR(PacketStats, GetValue));
    stub.reset(ADDR(BlockBufferMap, GetTotalBufferSize)); 
}

/*
* 用例名称：CheckProcessFileDescState
* 前置条件：
* check点：检查ProcessFileDescState返回值
*/
TEST_F(LibsmbCopyReaderTest, CheckProcessFileDescState)
{
    LLTSTUB::Stub stub;

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

    typedef int (*fptr)(LibsmbCopyReader*, FileHandle&);
    
    fptr LibsmbCopyReader_OpenFile = (fptr)((int(LibsmbCopyReader::*)(FileHandle&))&LibsmbCopyReader::OpenFile);
    fptr LibsmbCopyReader_ReadData = (fptr)((int(LibsmbCopyReader::*)(FileHandle&))&LibsmbCopyReader::ReadData);
    fptr LibsmbCopyReader_CloseFile = (fptr)((int(LibsmbCopyReader::*)(FileHandle&))&LibsmbCopyReader::CloseFile);

    stub.set(LibsmbCopyReader_OpenFile, OpenFile_Stub_Suc);
    stub.set(LibsmbCopyReader_ReadData, ReadData_Stub_Suc);
    stub.set(LibsmbCopyReader_CloseFile, CloseFile_Stub_Suc);

    // EXPECT_NO_THROW(m_libsmbCopyReader->ProcessFileDescState(fileHandle));
    
    // stub.set(ADDR(FileDesc, GetSrcState), GetSrcState_Stub_AGGREGATED);
    // EXPECT_NO_THROW(m_libsmbCopyReader->ProcessFileDescState(fileHandle));
    // stub.reset(ADDR(FileDesc, GetSrcState));

    // stub.set(ADDR(FileDesc, GetSrcState), GetSrcState_Stub_WRITE_SKIP);
    // EXPECT_NO_THROW(m_libsmbCopyReader->ProcessFileDescState(fileHandle));
    // stub.reset(ADDR(FileDesc, GetSrcState));

    // stub.set(ADDR(FileDesc, GetSrcState), GetSrcState_Stub_INIT);
    // EXPECT_NO_THROW(m_libsmbCopyReader->ProcessFileDescState(fileHandle));
    // stub.reset(ADDR(FileDesc, GetSrcState));

    // stub.set(ADDR(FileDesc, GetSrcState), GetSrcState_Stub_META_READED);
    // EXPECT_NO_THROW(m_libsmbCopyReader->ProcessFileDescState(fileHandle));
    // stub.reset(ADDR(FileDesc, GetSrcState));

    // stub.set(ADDR(FileDesc, GetSrcState), GetSrcState_Stub_SRC_OPENED);
    // EXPECT_NO_THROW(m_libsmbCopyReader->ProcessFileDescState(fileHandle));
    // stub.reset(ADDR(FileDesc, GetSrcState));

    // stub.set(ADDR(FileDesc, GetSrcState), GetSrcState_Stub_PARTIAL_READED);
    // EXPECT_NO_THROW(m_libsmbCopyReader->ProcessFileDescState(fileHandle));
    // stub.reset(ADDR(FileDesc, GetSrcState));

    // stub.set(ADDR(FileDesc, GetSrcState), GetSrcState_Stub_READED);
    // EXPECT_NO_THROW(m_libsmbCopyReader->ProcessFileDescState(fileHandle));
    // stub.reset(ADDR(FileDesc, GetSrcState));

    // fileHandle.m_file->SetFalg(IS_DIR);
    // EXPECT_NO_THROW(m_libsmbCopyReader->ProcessFileDescState(fileHandle));

    stub.reset(LibsmbCopyReader_OpenFile);
    stub.reset(LibsmbCopyReader_ReadData);
    stub.reset(LibsmbCopyReader_CloseFile);
}

/*
* 用例名称：CheckInit
* 前置条件：
* check点：检查Init返回值
*/
TEST_F(LibsmbCopyReaderTest, CheckInit)
{
    BackupParams params;
    params.srcEngine = BackupIOEngine::ARCHIVE_CLIENT;
    params.dstEngine = BackupIOEngine::POSIX;

    FileHandle fileHandle; 
    fileHandle.m_file = std::make_shared<FileDesc>(params.srcEngine, params.dstEngine);
    m_libsmbCopyReader->m_controlInfo->m_readConsume = 1;
    EXPECT_NO_THROW(m_libsmbCopyReader->Init(fileHandle));
}

/*
* 用例名称：CheckOpenFile
* 前置条件：
* check点：检查OpenFile返回值
*/
TEST_F(LibsmbCopyReaderTest, CheckOpenFile)
{
    LLTSTUB::Stub stub;

    BackupParams params;
    params.srcEngine = BackupIOEngine::ARCHIVE_CLIENT;
    params.dstEngine = BackupIOEngine::POSIX;

    FileHandle fileHandle; 
    fileHandle.m_file = std::make_shared<FileDesc>(params.srcEngine, params.dstEngine);
    
    stub.set(ADDR(LibsmbCopyReader, GetSmbReaderCommonData), GetSmbReaderCommonData_Stub);
    stub.set(SendReaderRequest, SendReaderRequest_Stub_Suc);
    EXPECT_NO_THROW(m_libsmbCopyReader->OpenFile(fileHandle));
    stub.reset(SendReaderRequest);

    stub.set(SendReaderRequest, SendReaderRequest_Stub_Fail);
    EXPECT_NO_THROW(m_libsmbCopyReader->OpenFile(fileHandle));
    stub.reset(SendReaderRequest);
    stub.reset(ADDR(LibsmbCopyReader, GetSmbReaderCommonData));

    stub.set(ADDR(LibsmbCopyReader, GetSmbReaderCommonData), GetSmbReaderCommonData_Stub1);
    EXPECT_NO_THROW(m_libsmbCopyReader->OpenFile(fileHandle));
    stub.reset(ADDR(LibsmbCopyReader, GetSmbReaderCommonData));
}

/*
* 用例名称：CheckReadData
* 前置条件：
* check点：检查ReadData返回值
*/
TEST_F(LibsmbCopyReaderTest, CheckReadData)
{
    LLTSTUB::Stub stub;
    BackupParams params;
    params.srcEngine = BackupIOEngine::ARCHIVE_CLIENT;
    params.dstEngine = BackupIOEngine::POSIX;

    FileHandle fileHandle; 
    fileHandle.m_file = std::make_shared<FileDesc>(params.srcEngine, params.dstEngine);

    
    MOCKER_CPP(&LibsmbCopyReader::ReadNormalData)
            .stubs()
            .will(returnValue(0));
    EXPECT_NO_THROW(m_libsmbCopyReader->ReadData(fileHandle));
}

/*
* 用例名称：CheckReadMeta
* 前置条件：
* check点：检查ReadMeta返回值
*/
TEST_F(LibsmbCopyReaderTest, CheckReadMeta)
{
    LLTSTUB::Stub stub;
    BackupParams params;
    params.srcEngine = BackupIOEngine::ARCHIVE_CLIENT;
    params.dstEngine = BackupIOEngine::POSIX;

    FileHandle fileHandle; 
    fileHandle.m_file = std::make_shared<FileDesc>(params.srcEngine, params.dstEngine);

    //EXPECT_NO_THROW(m_libsmbCopyReader->ReadMeta(fileHandle));
}

/*
* 用例名称：CheckCloseFile
* 前置条件：
* check点：检查CloseFile返回值
*/
TEST_F(LibsmbCopyReaderTest, CheckCloseFile)
{
    LLTSTUB::Stub stub;

    BackupParams params;
    params.srcEngine = BackupIOEngine::ARCHIVE_CLIENT;
    params.dstEngine = BackupIOEngine::POSIX;

    FileHandle fileHandle; 
    fileHandle.m_file = std::make_shared<FileDesc>(params.srcEngine, params.dstEngine);

    stub.set(ADDR(LibsmbCopyReader, GetSmbReaderCommonData), GetSmbReaderCommonData_Stub);
    stub.set(ADDR(SmbContextWrapper, SmbCloseAsync), SmbCloseAsync_Stub);
    stub.set(SendReaderRequest, SendReaderRequest_Stub_Suc);
    EXPECT_EQ(m_libsmbCopyReader->CloseFile(fileHandle), SUCCESS);
    stub.reset(SendReaderRequest);

    stub.set(SendReaderRequest, SendReaderRequest_Stub_Fail);
    EXPECT_EQ(m_libsmbCopyReader->CloseFile(fileHandle), FAILED);
    stub.reset(SendReaderRequest);
    stub.reset(ADDR(LibsmbCopyReader, GetSmbReaderCommonData));
    stub.reset(ADDR(SmbContextWrapper, SmbCloseAsync));

    stub.set(ADDR(LibsmbCopyReader, GetSmbReaderCommonData), GetSmbReaderCommonData_Stub1);
    EXPECT_EQ(m_libsmbCopyReader->CloseFile(fileHandle), FAILED);
    stub.reset(ADDR(LibsmbCopyReader, GetSmbReaderCommonData));
}

/*
* 用例名称：CheckReadNormalData
* 前置条件：
* check点：检查ReadNormalData返回值
*/
TEST_F(LibsmbCopyReaderTest, CheckReadNormalData)
{
    LLTSTUB::Stub stub;

    BackupParams params;
    params.srcEngine = BackupIOEngine::ARCHIVE_CLIENT;
    params.dstEngine = BackupIOEngine::POSIX;

    FileHandle fileHandle; 
    fileHandle.m_file = std::make_shared<FileDesc>(params.srcEngine, params.dstEngine);

    stub.set(SendReaderRequest, SendReaderRequest_Stub_Suc);
    EXPECT_EQ(m_libsmbCopyReader->ReadNormalData(fileHandle), SUCCESS);
    stub.reset(SendReaderRequest);

    stub.set(SendReaderRequest, SendReaderRequest_Stub_Fail);
    EXPECT_EQ(m_libsmbCopyReader->ReadNormalData(fileHandle), FAILED);
    stub.reset(SendReaderRequest);
}
