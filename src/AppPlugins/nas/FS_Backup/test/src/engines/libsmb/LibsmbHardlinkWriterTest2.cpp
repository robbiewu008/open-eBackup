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
#include "LibsmbHardlinkWriter.h"
#include <sys/stat.h>
#include "Libsmb.h"
#include "log/Log.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "mockcpp/mockcpp.hpp"
#include "llt_stub/named_stub.h"
#include "BackupStructs.h"

using namespace std;
using namespace Module;
using namespace FS_Backup;

using namespace std;
namespace  {
}

class LibsmbHardlinkWriterTest2 : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    std::shared_ptr<LibsmbHardlinkWriter> m_ins = nullptr;
};

void LibsmbHardlinkWriterTest2::SetUp()
{
    MOCKER_CPP(FillContextParams)
            .stubs()
            .will(ignoreReturnValue());
    WriterParams hardlinkWriterParams;
    BackupParams backupParams;
    std::shared_ptr<LibsmbBackupAdvanceParams> srcAdvParams = std::make_shared<LibsmbBackupAdvanceParams>();
    srcAdvParams->serverCheckMaxCount = 0;
    backupParams.srcAdvParams = srcAdvParams;
    backupParams.dstAdvParams = std::make_shared<LibsmbBackupAdvanceParams>();
    hardlinkWriterParams.backupParams = backupParams;
    hardlinkWriterParams.controlInfo = std::make_shared<BackupControlInfo>();
    hardlinkWriterParams.blockBufferMap = std::make_shared<BlockBufferMap>(); 
    BackupQueueConfig config;
    config.maxSize = 100;
    config.maxMemorySize = 100;
    hardlinkWriterParams.writeQueuePtr = std::make_shared<BackupQueue<FileHandle>>(config);
//     std::shared_ptr<BackupQueue<FileHandle>> m_writeQueue 
    m_ins = std::make_unique<LibsmbHardlinkWriter>(hardlinkWriterParams);
}

void LibsmbHardlinkWriterTest2::TearDown()
{
    GlobalMockObject::verify(); // 校验mock规范并清除mock规范
}

void LibsmbHardlinkWriterTest2::SetUpTestCase()
{}

void LibsmbHardlinkWriterTest2::TearDownTestCase()
{}

/*
 * 用例名称：SmbConnectContexts
 * 前置条件：无
 * check点：smb连接
 */
TEST_F(LibsmbHardlinkWriterTest2, SmbConnectContexts) 
{
    std::shared_ptr<Module::SmbContextWrapper> ptr = nullptr;
    MOCKER_CPP(SmbConnectContext)
            .stubs()
            .will(returnValue(ptr)); 
    MOCKER_CPP(SmbDisconnectContext)
            .stubs()
            .will(ignoreReturnValue());             
    EXPECT_NO_THROW(m_ins->SmbConnectContexts());
    EXPECT_NO_THROW(m_ins->SmbDisconnectContexts());
    EXPECT_NO_THROW(m_ins->SmbDisconnectSyncContexts());
}

/*
* 用例名称：Abort
* 前置条件：无
* check点：终止
*/
TEST_F(LibsmbHardlinkWriterTest2, Abort)
{           
    EXPECT_EQ(m_ins->Abort(), BackupRetCode::SUCCESS);
}

/*
 * 用例名称：GetStatus
 * 前置条件：无
 * check点：获取状态
 */
TEST_F(LibsmbHardlinkWriterTest2, GetStatus)
{   
    m_ins->m_controlInfo->m_writePhaseComplete = false;
    EXPECT_EQ(m_ins->GetStatus(), BackupPhaseStatus::INPROGRESS);
    m_ins->m_controlInfo->m_writePhaseComplete = true;
    m_ins->m_abort = true;
    EXPECT_EQ(m_ins->GetStatus(), BackupPhaseStatus::ABORTED);
    m_ins->m_abort = false;
    m_ins->m_controlInfo->m_failed  = true;
    EXPECT_EQ(m_ins->GetStatus(), BackupPhaseStatus::FAILED);
    m_ins->m_controlInfo->m_failed  = false;
    EXPECT_EQ(m_ins->GetStatus(), BackupPhaseStatus::COMPLETED);
}

/*
 * 用例名称：IsAbort
 * 前置条件：无
 * check点：判断是否终止
 */
TEST_F(LibsmbHardlinkWriterTest2, IsAbort) 
{
    m_ins->m_abort = false;
    m_ins->m_controlInfo->m_failed = false;
    m_ins->m_controlInfo->m_controlReaderFailed = true;
    EXPECT_EQ(m_ins->IsAbort(), true);
    m_ins->m_controlInfo->m_controlReaderFailed = false;
    EXPECT_EQ(m_ins->IsAbort(), false);
    EXPECT_NO_THROW(m_ins->HandleComplete());
}

/*
 * 用例名称：IsMkdirComplete
 * 前置条件：无
 * check点：判断是否终止
 */
TEST_F(LibsmbHardlinkWriterTest2, IsMkdirComplete) 
{
    FileHandle fileHandle;
    BackupIOEngine srcIoEngine;
    BackupIOEngine dstIoEngine;
    fileHandle.m_file = std::make_shared<FileDesc>(srcIoEngine, dstIoEngine);
    BackupQueueConfig config;
    config.maxSize = 100;
    config.maxMemorySize = 100;
    m_ins->m_dirQueue = std::make_shared<BackupQueue<FileHandle>>(config);
    m_ins->m_controlInfo->m_writePhaseComplete = true;
    EXPECT_EQ(m_ins->IsMkdirComplete(), true);
    m_ins->m_dirQueue->Push(fileHandle);
    EXPECT_EQ(m_ins->IsMkdirComplete(), false);
}

/*
 * 用例名称：OpenFile
 * 前置条件：无
 * check点：打开文件
 */
TEST_F(LibsmbHardlinkWriterTest2, OpenFile) 
{   
    MOCKER_CPP(SendWriterRequest)
            .stubs()
            .will(returnValue(-1))
            .then(returnValue(0))
            .then(returnValue(-1))
            .then(returnValue(0))
            .then(returnValue(-1))
            .then(returnValue(0))
            .then(returnValue(-1))
            .then(returnValue(0));
    FileHandle fileHandle;
    BackupIOEngine srcIoEngine;
    BackupIOEngine dstIoEngine;
    fileHandle.m_file = std::make_shared<FileDesc>(srcIoEngine, dstIoEngine); // 桩函数中创建智能指针，需要形参
    fileHandle.m_file->m_fileName = "nanjing";
    fileHandle.m_file->SetFlag(IS_DIR);
    EXPECT_EQ(m_ins->OpenFile(fileHandle), -1);
    EXPECT_EQ(m_ins->OpenFile(fileHandle), 0);
    EXPECT_EQ(m_ins->CloseFile(fileHandle), 0);
    EXPECT_EQ(m_ins->CloseFile(fileHandle), 0);
    EXPECT_EQ(m_ins->WriteData(fileHandle), -1);
    EXPECT_EQ(m_ins->WriteData(fileHandle), 0);
    EXPECT_EQ(m_ins->DeleteFile(fileHandle), -1);
    EXPECT_EQ(m_ins->DeleteFile(fileHandle), 0);
}

/*
 * 用例名称：CloseFile
 * 前置条件：无
 * check点：关闭文件
 */
struct smb2fh {
    struct smb2fh *next;
    smb2_command_cb cb;
    void *cb_data;

    uint8_t file_id[SMB2_FD_SIZE];
    int64_t offset;
    int64_t end_of_file;
};
TEST_F(LibsmbHardlinkWriterTest2, CloseFile) 
{   
    MOCKER_CPP(SendWriterRequest)
            .stubs()
            .will(returnValue(-1))
            .then(returnValue(0));
    FileHandle fileHandle;
    BackupIOEngine srcIoEngine;
    BackupIOEngine dstIoEngine;
    fileHandle.m_file = std::make_shared<FileDesc>(srcIoEngine, dstIoEngine); // 桩函数中创建智能指针，需要形参
    fileHandle.m_file->m_fileName = "nanjing";
    fileHandle.m_file->SetFlag(IS_DIR);
    smb2fh smbF;
    fileHandle.m_file->dstIOHandle.smbFh = &smbF;
    EXPECT_EQ(m_ins->CloseFile(fileHandle), -1);
    EXPECT_EQ(m_ins->CloseFile(fileHandle), 0);
}

/*
 * 用例名称：ProcessTimers
 * 前置条件：无
 * check点：处理定时器
 */
//  int64_t GetExpiredEventAndTime(std::vector<FileHandle> &fileHandles, int expiredCount)
static int64_t StubGetExpiredEventAndTime(BackupTimer* This, std::vector<FileHandle> &fileHandles) 
{
    FileHandle fileHandle;
    BackupIOEngine srcIoEngine;
    BackupIOEngine dstIoEngine;
    fileHandle.m_file = std::make_shared<FileDesc>(srcIoEngine, dstIoEngine);
    fileHandles.push_back(fileHandle);
    return 600;
}
static int64_t StubGetExpiredEventAndTime2(BackupTimer* This, std::vector<FileHandle> &fileHandles) 
{
    FileHandle fileHandle;
    BackupIOEngine srcIoEngine;
    BackupIOEngine dstIoEngine;
    fileHandle.m_file = std::make_shared<FileDesc>(srcIoEngine, dstIoEngine);
    fileHandles.push_back(fileHandle);
    return 100;
}
TEST_F(LibsmbHardlinkWriterTest2, ProcessTimers) 
{
    vector<FileHandle> fileHandles;
    FileHandle fileHandle;
    BackupIOEngine srcIoEngine;
    BackupIOEngine dstIoEngine;
    fileHandle.m_file = std::make_shared<FileDesc>(srcIoEngine, dstIoEngine);
    fileHandles.push_back(fileHandle);
    MOCKER_CPP(&BackupTimer::GetExpiredEventAndTime, int64_t(BackupTimer::*)(std::vector<FileHandle>&))
            .stubs()
            .will(invoke(StubGetExpiredEventAndTime))
            .then(invoke(StubGetExpiredEventAndTime2));            
    MOCKER_CPP(&LibsmbHardlinkWriter::ProcessFileDescState)
            .stubs()
            .will(ignoreReturnValue());
    EXPECT_EQ(m_ins->ProcessTimers(), 500);
//     EXPECT_EQ(m_ins->ProcessTimers(), 100);
}

/*
 * 用例名称：ThreadFunc
 * 前置条件：无
 * check点：线程函数
 */
TEST_F(LibsmbHardlinkWriterTest2, ThreadFunc) 
{           
    MOCKER_CPP(&LibsmbHardlinkWriter::ServerCheck)
            .stubs()
            .will(returnValue(-1))
            .then(returnValue(0));
    MOCKER_CPP(&LibsmbHardlinkWriter::SmbDisconnectContexts)
            .stubs()
            .will(ignoreReturnValue());
    MOCKER_CPP(&LibsmbHardlinkWriter::HandleComplete)
            .stubs()
            .will(ignoreReturnValue());
    MOCKER_CPP(&LibsmbHardlinkWriter::IsComplete)
            .stubs()
            .will(returnValue(true))
            .then(returnValue(false));
    MOCKER_CPP(&LibsmbHardlinkWriter::IsAbort)
            .stubs()
            .will(returnValue(true))
            .then(returnValue(false)); 
    EXPECT_NO_THROW(m_ins->ThreadFunc());
    EXPECT_NO_THROW(m_ins->ThreadFunc());
    EXPECT_NO_THROW(m_ins->ThreadFunc());
}

/*
 * 用例名称：ProcessReadEntries
 * 前置条件：无
 * check点：处理读entries
 */
static bool StubWaitAndPopTrue(BackupQueue<FileHandle>* This, FileHandle& args, uint32_t args2)
{
    FileHandle fileHandle;
    BackupIOEngine srcIoEngine;
    BackupIOEngine dstIoEngine;
    fileHandle.m_file = std::make_shared<FileDesc>(srcIoEngine, dstIoEngine); // 桩函数中创建智能指针，需要形参
    fileHandle.m_file->m_fileName = "nanjing";
    args = fileHandle;
    return true;
}
static bool StubWaitAndPopFalse(BackupQueue<FileHandle>* This, FileHandle& args, uint32_t args2)
{
    FileHandle fileHandle;
    BackupIOEngine srcIoEngine;
    BackupIOEngine dstIoEngine;
    fileHandle.m_file = std::make_shared<FileDesc>(srcIoEngine, dstIoEngine); // 桩函数中创建智能指针，需要形参
    fileHandle.m_file->m_fileName = "nanjing";
    args = fileHandle;
    return false;
}
TEST_F(LibsmbHardlinkWriterTest2, ProcessWriteEntries) 
{   
    MOCKER_CPP(&BackupQueue<FileHandle>::WaitAndPop, bool(BackupQueue<FileHandle>::*)(FileHandle&, uint32_t)) // 模板类中重载函数打桩
            .stubs()
            .will(invoke(StubWaitAndPopTrue))
            .then(invoke(StubWaitAndPopFalse));
    MOCKER_CPP(&LibsmbHardlinkWriter::ProcessFileDescState)
            .stubs()
            .will(ignoreReturnValue());
    EXPECT_NO_THROW(m_ins->ProcessWriteEntries());
    EXPECT_NO_THROW(m_ins->ProcessWriteEntries());
}

TEST_F(LibsmbHardlinkWriterTest2, ProcessWriteEntries_2) 
{   
    MOCKER_CPP(&BackupQueue<FileHandle>::WaitAndPop, bool(BackupQueue<FileHandle>::*)(FileHandle&, uint32_t)) // 模板类中重载函数打桩
            .stubs()
            .will(returnValue(false));
    MOCKER_CPP(&LibsmbHardlinkWriter::ProcessFileDescState)
            .stubs()
            .will(ignoreReturnValue());
    MOCKER_CPP(&LibsmbHardlinkWriter::IsAbort)
            .stubs()
            .will(returnValue(true))
            .then(returnValue(false));
    FileHandle fileHandle;
    BackupIOEngine srcIoEngine;
    BackupIOEngine dstIoEngine;
    fileHandle.m_file = std::make_shared<FileDesc>(srcIoEngine, dstIoEngine); // 桩函数中创建智能指针，需要形参
    fileHandle.m_file->m_fileName = "nanjing";
    m_ins->m_writeQueue->Push(fileHandle);
    EXPECT_NO_THROW(m_ins->ProcessWriteEntries());
    EXPECT_NO_THROW(m_ins->ProcessWriteEntries());
}

/*
 * 用例名称：ServerCheck
 * 前置条件：无
 * check点：服务器检查
 */
TEST_F(LibsmbHardlinkWriterTest2, ServerCheck) 
{
    MOCKER_CPP(&PacketStats::GetValue)
            .stubs()
            .will(returnValue(105))
            .then(returnValue(95))
            .then(returnValue(105));
    MOCKER_CPP(&PacketStats::ResetErrorCounter)
            .stubs()
            .will(ignoreReturnValue());
    MOCKER_CPP(HandleConnectionException)
            .stubs()
            .will(returnValue(-1))
            .then(returnValue(0));
    EXPECT_EQ(m_ins->ServerCheck(), -1);
    EXPECT_EQ(m_ins->ServerCheck(), -1);
    EXPECT_EQ(m_ins->ServerCheck(), -1);
}
TEST_F(LibsmbHardlinkWriterTest2, ServerCheck_2) 
{
    MOCKER_CPP(&PacketStats::GetValue)
            .stubs()
            .will(returnValue(95))
            .then(returnValue(95))
            .then(returnValue(105));
    MOCKER_CPP(&PacketStats::ResetErrorCounter)
            .stubs()
            .will(ignoreReturnValue());
    MOCKER_CPP(HandleConnectionException)
            .stubs()
            .will(returnValue(-1))
            .then(returnValue(0));
    EXPECT_EQ(m_ins->ServerCheck(), -1);
    EXPECT_EQ(m_ins->ServerCheck(), -1);
    EXPECT_EQ(m_ins->ServerCheck(), -1);
}

/*
 * 用例名称：ProcessFileDescState
 * 前置条件：无
 * check点：处理文件状态
 */
TEST_F(LibsmbHardlinkWriterTest2, ProcessFileDescState) 
{   
    MOCKER_CPP(FillContextParams)
            .stubs()
            .will(ignoreReturnValue());
    WriterParams hardlinkWriterParams;
    BackupParams backupParams;
    std::shared_ptr<LibsmbBackupAdvanceParams> srcAdvParams = std::make_shared<LibsmbBackupAdvanceParams>();
    srcAdvParams->serverCheckMaxCount = 0;
    backupParams.srcAdvParams = srcAdvParams;
    backupParams.dstAdvParams = std::make_shared<LibsmbBackupAdvanceParams>();
    hardlinkWriterParams.backupParams = backupParams;
    hardlinkWriterParams.controlInfo = std::make_shared<BackupControlInfo>();
    hardlinkWriterParams.blockBufferMap = std::make_shared<BlockBufferMap>();
    LibsmbHardlinkWriter libsmbHardlinkWriter(hardlinkWriterParams);
    BackupQueueConfig config;
    config.maxSize = 100;
    config.maxMemorySize = 100;
    libsmbHardlinkWriter.m_dirQueue = std::make_shared<BackupQueue<FileHandle>>(config);
    MOCKER_CPP(&BackupQueue<FileHandle>::Push, void(BackupQueue<FileHandle>::*)(FileHandle)) // 模板类中重载函数打桩
            .stubs()
            .will(ignoreReturnValue()); 
    MOCKER_CPP(&FileDesc::GetDstState)
            .stubs()
            .will(returnValue(FileDescState::INIT))
            .then(returnValue(FileDescState::INIT))
            .then(returnValue(FileDescState::INIT))
            .then(returnValue(FileDescState::DST_OPENED))
            .then(returnValue(FileDescState::READED))
            .then(returnValue(FileDescState::WRITED));  
            // .will(returnValue(FileDescState::DST_CLOSED)) 
    MOCKER_CPP(&FileDesc::GetSrcState)
            .stubs()
            .will(returnValue(FileDescState::READ_FAILED))        
            .then(returnValue(FileDescState::INIT));         
    MOCKER_CPP(&FileDesc::SetDstState)
            .stubs()
            .will(ignoreReturnValue());
    MOCKER_CPP(IsFileReadOrWriteFailed)
            .stubs()
            .will(returnValue(true))
            .then(returnValue(false));
    MOCKER_CPP_VIRTUAL(libsmbHardlinkWriter, &LibsmbHardlinkWriter::OpenFile) // 虚函数打桩问题
            .stubs()
            .will(ignoreReturnValue());
    MOCKER_CPP_VIRTUAL(libsmbHardlinkWriter, &LibsmbHardlinkWriter::CloseFile)
            .stubs()
            .will(ignoreReturnValue());
    MOCKER_CPP_VIRTUAL(libsmbHardlinkWriter, &LibsmbHardlinkWriter::WriteData)
            .stubs()
            .will(ignoreReturnValue());
    MOCKER_CPP(&LibsmbHardlinkWriter::DeleteFile)
            .stubs()
            .will(ignoreReturnValue());
    FileHandle fileHandle;
    BackupIOEngine srcIoEngine;
    BackupIOEngine dstIoEngine;
    fileHandle.m_file = std::make_shared<FileDesc>(srcIoEngine, dstIoEngine); // 桩函数中创建智能指针，需要形参
    fileHandle.m_file->m_fileName = "nanjing";
    fileHandle.m_block.m_size = 0;
    fileHandle.m_block.m_seq = 0;
    fileHandle.m_file->SetFlag(IS_DIR); 
    EXPECT_NO_THROW(libsmbHardlinkWriter.ProcessFileDescState(fileHandle));
    EXPECT_NO_THROW(libsmbHardlinkWriter.ProcessFileDescState(fileHandle));
    EXPECT_NO_THROW(libsmbHardlinkWriter.ProcessFileDescState(fileHandle));
    fileHandle.m_file->ClearFlag(IS_DIR);
    fileHandle.m_file->m_mode = FILE_HAVE_ADS;
    EXPECT_NO_THROW(libsmbHardlinkWriter.ProcessFileDescState(fileHandle));
    EXPECT_NO_THROW(libsmbHardlinkWriter.ProcessFileDescState(fileHandle));
    EXPECT_NO_THROW(libsmbHardlinkWriter.ProcessFileDescState(fileHandle));
    EXPECT_NO_THROW(libsmbHardlinkWriter.ProcessFileDescState(fileHandle));
}

/*
 * 用例名称：ClearWriteCache
 * 前置条件：无
 * check点：清除写cache
 */
TEST_F(LibsmbHardlinkWriterTest2, ClearWriteCache) 
{
    FileHandle fileHandle;
    BackupIOEngine srcIoEngine;
    BackupIOEngine dstIoEngine;
    fileHandle.m_file = std::make_shared<FileDesc>(srcIoEngine, dstIoEngine); // 桩函数中创建智能指针，需要形参
    fileHandle.m_file->m_fileName = "nanjing";
    fileHandle.m_block.m_size = 0;
    fileHandle.m_block.m_seq = 0;
    fileHandle.m_file->SetFlag(IS_DIR); 
    std::vector<FileHandle> fileHandles = {fileHandle};
    // std::unordered_map<std::string, std::vector<FileHandle>> m_writeCache;
    EXPECT_NO_THROW(m_ins->ClearWriteCache());
    m_ins->m_writeCache["xxx"] = fileHandles;
    EXPECT_NO_THROW(m_ins->ClearWriteCache());
    MOCKER_CPP(&FileDesc::GetDstState)
            .stubs() 
            .will(returnValue(FileDescState::DST_OPENED))
            .then(returnValue(FileDescState::WRITED)); 
    MOCKER_CPP(IsFileReadOrWriteFailed)
            .stubs()
            .will(returnValue(true));
    EXPECT_NO_THROW(m_ins->ClearWriteCache());
    m_ins->m_writeCache["xxx"] = fileHandles;
    EXPECT_NO_THROW(m_ins->ClearWriteCache());
}

/*
 * 用例名称：SyncThreadFunc
 * 前置条件：无
 * check点：同步线程函数
 */
TEST_F(LibsmbHardlinkWriterTest2, SyncThreadFunc) 
{
    MOCKER_CPP(&LibsmbHardlinkWriter::IsMkdirComplete)
            .stubs()
            .will(returnValue(true))
            .then(returnValue(false));
    MOCKER_CPP(&LibsmbHardlinkWriter::IsAbort)
            .stubs()
            .will(returnValue(true))
            .then(returnValue(false));
    EXPECT_NO_THROW(m_ins->SyncThreadFunc());
    EXPECT_NO_THROW(m_ins->SyncThreadFunc());
}

/*
 * 用例名称：Start
 * 前置条件：无
 * check点：开启线程
 */
// TEST_F(LibsmbHardlinkWriterTest2, Start) 
// {
//     MOCKER_CPP(FillContextParams)
//             .stubs()
//             .will(ignoreReturnValue());
//     WriterParams hardlinkWriterParams;
//     BackupParams backupParams;
//     std::shared_ptr<LibsmbBackupAdvanceParams> srcAdvParams = std::make_shared<LibsmbBackupAdvanceParams>();
//     srcAdvParams->serverCheckMaxCount = 0;
//     backupParams.srcAdvParams = srcAdvParams;
//     backupParams.dstAdvParams = std::make_shared<LibsmbBackupAdvanceParams>();
//     hardlinkWriterParams.backupParams = backupParams;
//     hardlinkWriterParams.controlInfo = std::make_shared<BackupControlInfo>();
//     hardlinkWriterParams.blockBufferMap = std::make_shared<BlockBufferMap>();
//     LibsmbHardlinkWriter libsmbHardlinkWriter(hardlinkWriterParams);

//     MOCKER_CPP(&LibsmbHardlinkWriter::SmbConnectContexts)
//             .stubs()
//             .will(returnValue(-1))
//             .then(returnValue(0));
//     MOCKER_CPP(&LibsmbHardlinkWriter::IsAbort)
//             .stubs()
//             .will(returnValue(true))
//             .then(returnValue(false));
//     MOCKER_CPP_VIRTUAL(libsmbHardlinkWriter, &LibsmbHardlinkWriter::ThreadFunc)
//             .stubs()
//             .will(ignoreReturnValue());
//     MOCKER_CPP(&LibsmbHardlinkWriter::SyncThreadFunc)
//             .stubs()
//             .will(ignoreReturnValue());
//     EXPECT_EQ(m_ins->Start(), BackupRetCode::FAILED);
//     EXPECT_EQ(m_ins->Start(), BackupRetCode::SUCCESS);
// }

/*
 * 用例名称：WriteMeta
 * 前置条件：无
 * check点：写meta
 */
TEST_F(LibsmbHardlinkWriterTest2, WriteMeta) 
{
    FileHandle fileHandle;
    BackupIOEngine srcIoEngine;
    BackupIOEngine dstIoEngine;
    fileHandle.m_file = std::make_shared<FileDesc>(srcIoEngine, dstIoEngine); // 桩函数中创建智能指针，需要形参
    fileHandle.m_file->m_fileName = "nanjing";
    fileHandle.m_block.m_size = 0;
    fileHandle.m_block.m_seq = 0;
    fileHandle.m_file->SetFlag(IS_DIR);
    MOCKER_CPP(SendWriterRequest)
            .stubs()
            .will(returnValue(-1))
            .then(returnValue(0));
    EXPECT_EQ(m_ins->WriteMeta(fileHandle), -1);
}

/*
 * 用例名称：IsComplete
 * 前置条件：无
 * check点：判断是否完成
 */
TEST_F(LibsmbHardlinkWriterTest2, IsComplete) 
{
    MOCKER_CPP(&PacketStats::GetValue)
            .stubs()
            .will(returnValue(105));
    BackupQueueConfig config;
    config.maxSize = 100;
    config.maxMemorySize = 100;
    m_ins->m_dirQueue = std::make_shared<BackupQueue<FileHandle>>(config);
    m_ins->m_controlInfo->m_aggregatePhaseComplete = false;
    EXPECT_EQ(m_ins->IsComplete(), false);
    EXPECT_EQ(m_ins->IsComplete(), false);
}

/*
 * 用例名称：ProcessConnectionException
 * 前置条件：无
 * check点：处理连接异常
 */
TEST_F(LibsmbHardlinkWriterTest2, ProcessConnectionException) 
{
    MOCKER_CPP(&PacketStats::ResetErrorCounter)
            .stubs()
            .will(ignoreReturnValue());
    MOCKER_CPP(HandleConnectionException)
            .stubs()
            .will(returnValue(-1))
            .then(returnValue(0));
    EXPECT_EQ(m_ins->ProcessConnectionException(), -1);
    EXPECT_EQ(m_ins->ProcessConnectionException(), 0);
}

/*
 * 用例名称：IsWriterRequestReachThreshold
 * 前置条件：无
 * check点：判断是否完成
 */
TEST_F(LibsmbHardlinkWriterTest2, IsWriterRequestReachThreshold) 
{   
    MOCKER_CPP(&PacketStats::GetValue)
            .stubs()
            .will(returnValue(150))
            .then(returnValue(200));
    EXPECT_NO_THROW(m_ins->IsWriterRequestReachThreshold());
    EXPECT_NO_THROW(m_ins->IsWriterRequestReachThreshold());
}
