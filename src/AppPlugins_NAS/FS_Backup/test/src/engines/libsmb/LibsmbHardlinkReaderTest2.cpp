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
#include "mockcpp/mockcpp.hpp"
#include "llt_stub/named_stub.h"
#include "llt_stub/addr_pri.h"
#include "LibsmbHardlinkReader.h"

using namespace std;
using namespace Module;

class LibsmbHardlinkReaderTest2 : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    std::shared_ptr<LibsmbHardlinkReader> m_ins2 = nullptr;
};

static bool StubFillContextParams(Module::SmbContextArgs &smbContextArgs, std::shared_ptr<LibsmbBackupAdvanceParams> advParams)
{
    Module::SmbContextArgs smbContextArgs_t;
    smbContextArgs = smbContextArgs_t;
    advParams = std::make_shared<LibsmbBackupAdvanceParams>();
    advParams->serverCheckMaxCount = 0;
    return true;
}

void LibsmbHardlinkReaderTest2::SetUp()
{  
    MOCKER_CPP(FillContextParams)
            .stubs()
            .will(ignoreReturnValue());
    ReaderParams hardlinkReaderParams;
    BackupParams backupParams;
    std::shared_ptr<LibsmbBackupAdvanceParams> srcAdvParams = std::make_shared<LibsmbBackupAdvanceParams>();
    srcAdvParams->serverCheckMaxCount = 0;
    backupParams.srcAdvParams = srcAdvParams;
    hardlinkReaderParams.backupParams = backupParams;
    hardlinkReaderParams.controlInfo = std::make_shared<BackupControlInfo>();
    std::shared_ptr<LibsmbHardlinkReader> m_ins2 = std::make_unique<LibsmbHardlinkReader>(hardlinkReaderParams);
}

void LibsmbHardlinkReaderTest2::TearDown()
{
    GlobalMockObject::verify(); // 校验mock规范并清除mock规范
}

void LibsmbHardlinkReaderTest2::SetUpTestCase()
{}

void LibsmbHardlinkReaderTest2::TearDownTestCase()
{}

/*
* 用例名称：SmbConnectContexts
* 前置条件：无
* check点：smb连接内容
*/
TEST_F(LibsmbHardlinkReaderTest2, SmbConnectContexts)
{
    // LLTSTUB::Stub stub;
    // stub.set(FillContextParams, StubFillContextParams);
    MOCKER_CPP(FillContextParams)
            .stubs()
            .will(ignoreReturnValue());
    ReaderParams hardlinkReaderParams;
    BackupParams backupParams;
    std::shared_ptr<LibsmbBackupAdvanceParams> srcAdvParams = std::make_shared<LibsmbBackupAdvanceParams>();
    srcAdvParams->serverCheckMaxCount = 0;
    backupParams.srcAdvParams = srcAdvParams;
    hardlinkReaderParams.backupParams = backupParams;
    hardlinkReaderParams.controlInfo = std::make_shared<BackupControlInfo>();
    std::shared_ptr<LibsmbHardlinkReader> m_ins = std::make_unique<LibsmbHardlinkReader>(hardlinkReaderParams);
    std::shared_ptr<Module::SmbContextWrapper> ptr = nullptr;
    MOCKER_CPP(SmbConnectContext)
            .stubs()
            .will(returnValue(ptr)); 
    MOCKER_CPP(SmbDisconnectContext)
            .stubs()
            .will(ignoreReturnValue());             
    EXPECT_NO_THROW(m_ins->SmbConnectContexts());
    EXPECT_NO_THROW(m_ins->SmbDisconnectContexts());
}

/*
* 用例名称：Abort
* 前置条件：无
* check点：终止
*/
TEST_F(LibsmbHardlinkReaderTest2, Abort)
{
    MOCKER_CPP(FillContextParams)
            .stubs()
            .will(ignoreReturnValue());
    ReaderParams hardlinkReaderParams;
    BackupParams backupParams;
    std::shared_ptr<LibsmbBackupAdvanceParams> srcAdvParams = std::make_shared<LibsmbBackupAdvanceParams>();
    srcAdvParams->serverCheckMaxCount = 0;
    backupParams.srcAdvParams = srcAdvParams;
    hardlinkReaderParams.backupParams = backupParams;
    std::shared_ptr<LibsmbHardlinkReader> m_ins = std::make_unique<LibsmbHardlinkReader>(hardlinkReaderParams);            
    EXPECT_NO_THROW(m_ins->Abort());
}

/*
 * 用例名称：GetStatus
 * 前置条件：无
 * check点：获取状态
 */
TEST_F(LibsmbHardlinkReaderTest2, GetStatus)
{   
    MOCKER_CPP(FillContextParams)
            .stubs()
            .will(ignoreReturnValue());
    ReaderParams hardlinkReaderParams;
    BackupParams backupParams;
    std::shared_ptr<LibsmbBackupAdvanceParams> srcAdvParams = std::make_shared<LibsmbBackupAdvanceParams>();
    srcAdvParams->serverCheckMaxCount = 0;
    backupParams.srcAdvParams = srcAdvParams;
    hardlinkReaderParams.backupParams = backupParams;
    hardlinkReaderParams.controlInfo = std::make_shared<BackupControlInfo>();
    std::shared_ptr<LibsmbHardlinkReader> m_ins = std::make_unique<LibsmbHardlinkReader>(hardlinkReaderParams);
    m_ins->m_controlInfo->m_readPhaseComplete = false;
    EXPECT_EQ(m_ins->GetStatus(), BackupPhaseStatus::INPROGRESS);
    m_ins->m_controlInfo->m_readPhaseComplete = true;
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
TEST_F(LibsmbHardlinkReaderTest2, IsAbort) 
{
    MOCKER_CPP(FillContextParams)
            .stubs()
            .will(ignoreReturnValue());
    ReaderParams hardlinkReaderParams;
    BackupParams backupParams;
    std::shared_ptr<LibsmbBackupAdvanceParams> srcAdvParams = std::make_shared<LibsmbBackupAdvanceParams>();
    srcAdvParams->serverCheckMaxCount = 0;
    backupParams.srcAdvParams = srcAdvParams;
    hardlinkReaderParams.backupParams = backupParams;
    hardlinkReaderParams.controlInfo = std::make_shared<BackupControlInfo>();
    std::shared_ptr<LibsmbHardlinkReader> m_ins = std::make_unique<LibsmbHardlinkReader>(hardlinkReaderParams);
    m_ins->m_abort = false;
    m_ins->m_controlInfo->m_failed = false;
    m_ins->m_controlInfo->m_controlReaderFailed = true;
    EXPECT_EQ(m_ins->IsAbort(), true);
    m_ins->m_controlInfo->m_controlReaderFailed = false;
    EXPECT_EQ(m_ins->IsAbort(), false);
    EXPECT_NO_THROW(m_ins->HandleComplete());
}

/*
 * 用例名称：ServerCheck
 * 前置条件：无
 * check点：服务器检查
 */
TEST_F(LibsmbHardlinkReaderTest2, ServerCheck) 
{
    MOCKER_CPP(FillContextParams)
            .stubs()
            .will(ignoreReturnValue());
    ReaderParams hardlinkReaderParams;
    BackupParams backupParams;
    std::shared_ptr<LibsmbBackupAdvanceParams> srcAdvParams = std::make_shared<LibsmbBackupAdvanceParams>();
    srcAdvParams->serverCheckMaxCount = 0;
    backupParams.srcAdvParams = srcAdvParams;
    hardlinkReaderParams.backupParams = backupParams;
    hardlinkReaderParams.controlInfo = std::make_shared<BackupControlInfo>();
    std::shared_ptr<LibsmbHardlinkReader> m_ins = std::make_unique<LibsmbHardlinkReader>(hardlinkReaderParams);
    MOCKER_CPP(&PacketStats::GetValue)
            .stubs()
            .will(returnValue(105))
            .then(returnValue(95))
            .then(returnValue(105))
            .then(returnValue(95));
    MOCKER_CPP(&PacketStats::ResetErrorCounter)
            .stubs()
            .will(ignoreReturnValue());
    MOCKER_CPP(HandleConnectionException)
            .stubs()
            .will(returnValue(-1))
            .then(returnValue(0));
    EXPECT_EQ(m_ins->ServerCheck(), -1);
    EXPECT_EQ(m_ins->ServerCheck(), -1);
    EXPECT_EQ(m_ins->ServerCheck(), 0);
    EXPECT_EQ(m_ins->ServerCheck(), 0);
}

/*
 * 用例名称：ProcessTimers
 * 前置条件：无
 * check点：处理定时器
 */
TEST_F(LibsmbHardlinkReaderTest2, ProcessTimers) 
{
    MOCKER_CPP(FillContextParams)
            .stubs()
            .will(ignoreReturnValue());
    ReaderParams hardlinkReaderParams;
    BackupParams backupParams;
    std::shared_ptr<LibsmbBackupAdvanceParams> srcAdvParams = std::make_shared<LibsmbBackupAdvanceParams>();
    srcAdvParams->serverCheckMaxCount = 0;
    backupParams.srcAdvParams = srcAdvParams;
    hardlinkReaderParams.backupParams = backupParams;
    hardlinkReaderParams.controlInfo = std::make_shared<BackupControlInfo>();
    hardlinkReaderParams.blockBufferMap = std::make_shared<BlockBufferMap>();
    std::shared_ptr<LibsmbHardlinkReader> m_ins = std::make_unique<LibsmbHardlinkReader>(hardlinkReaderParams);
    vector<FileHandle> fileHandles;
    FileHandle fileHandle;
    BackupIOEngine srcIoEngine;
    BackupIOEngine dstIoEngine;
    fileHandle.m_file = std::make_shared<FileDesc>(srcIoEngine, dstIoEngine);
    fileHandles.push_back(fileHandle);
    MOCKER_CPP(&PacketStats::GetValue)
            .stubs()
            .will(returnValue(105));
    MOCKER_CPP(&LibsmbHardlinkReader::IsReaderRequestReachThreshold)
            .stubs()
            .will(returnValue(true))
            .then(returnValue(false));
    EXPECT_EQ(m_ins->ProcessTimers(), 0);
    EXPECT_EQ(m_ins->ProcessTimers(), 500);
}

/*
 * 用例名称：ThreadFunc
 * 前置条件：无
 * check点：线程函数
 */
TEST_F(LibsmbHardlinkReaderTest2, ThreadFunc) 
{
    MOCKER_CPP(FillContextParams)
            .stubs()
            .will(ignoreReturnValue());
    ReaderParams hardlinkReaderParams;
    BackupParams backupParams;
    std::shared_ptr<LibsmbBackupAdvanceParams> srcAdvParams = std::make_shared<LibsmbBackupAdvanceParams>();
    srcAdvParams->serverCheckMaxCount = 0;
    backupParams.srcAdvParams = srcAdvParams;
    hardlinkReaderParams.backupParams = backupParams;
    hardlinkReaderParams.controlInfo = std::make_shared<BackupControlInfo>();
    hardlinkReaderParams.blockBufferMap = std::make_shared<BlockBufferMap>();
    std::shared_ptr<LibsmbHardlinkReader> m_ins = std::make_unique<LibsmbHardlinkReader>(hardlinkReaderParams);
            
    MOCKER_CPP(&LibsmbHardlinkReader::ServerCheck)
            .stubs()
            .will(returnValue(-1))
            .then(returnValue(0));
    MOCKER_CPP(&LibsmbHardlinkReader::SmbDisconnectContexts)
            .stubs()
            .will(ignoreReturnValue());
    MOCKER_CPP(&LibsmbHardlinkReader::HandleComplete)
            .stubs()
            .will(ignoreReturnValue());
    MOCKER_CPP(&LibsmbHardlinkReader::IsComplete)
            .stubs()
            .will(returnValue(true))
            .then(returnValue(false));
    MOCKER_CPP(&LibsmbHardlinkReader::IsAbort)
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

// TEST_F(LibsmbHardlinkReaderTest2, ProcessReadEntries) 
// {   
//     MOCKER_CPP(FillContextParams)
//             .stubs()
//             .will(ignoreReturnValue());
//     ReaderParams hardlinkReaderParams;
//     BackupParams backupParams;
//     std::shared_ptr<LibsmbBackupAdvanceParams> srcAdvParams = std::make_shared<LibsmbBackupAdvanceParams>();
//     srcAdvParams->serverCheckMaxCount = 0;
//     backupParams.srcAdvParams = srcAdvParams;
//     hardlinkReaderParams.backupParams = backupParams;
//     hardlinkReaderParams.controlInfo = std::make_shared<BackupControlInfo>();
//     hardlinkReaderParams.blockBufferMap = std::make_shared<BlockBufferMap>();
//     std::shared_ptr<LibsmbHardlinkReader> m_ins = std::make_unique<LibsmbHardlinkReader>(hardlinkReaderParams);
//     MOCKER_CPP(&BackupQueue<FileHandle>::WaitAndPop, bool(BackupQueue<FileHandle>::*)(FileHandle&, uint32_t)) // 模板类中重载函数打桩
//             .stubs()
//             .will(invoke(StubWaitAndPopTrue))
//             .then(invoke(StubWaitAndPopFalse));
//     MOCKER_CPP(&LibsmbHardlinkReader::ProcessFileDescState)
//             .stubs()
//             .will(ignoreReturnValue());
//     EXPECT_NO_THROW(m_ins->ProcessReadEntries());
//     EXPECT_NO_THROW(m_ins->ProcessReadEntries());
// }

/*
 * 用例名称：ProcessFileDescState
 * 前置条件：无
 * check点：处理文件状态
 */
TEST_F(LibsmbHardlinkReaderTest2, ProcessFileDescState) 
{   
    MOCKER_CPP(FillContextParams)
            .stubs()
            .will(ignoreReturnValue());
    ReaderParams hardlinkReaderParams;
    BackupParams backupParams;
    std::shared_ptr<LibsmbBackupAdvanceParams> srcAdvParams = std::make_shared<LibsmbBackupAdvanceParams>();
    srcAdvParams->serverCheckMaxCount = 0;
    backupParams.srcAdvParams = srcAdvParams;
    hardlinkReaderParams.backupParams = backupParams;
    hardlinkReaderParams.controlInfo = std::make_shared<BackupControlInfo>();
    hardlinkReaderParams.blockBufferMap = std::make_shared<BlockBufferMap>();
    LibsmbHardlinkReader libsmbHardlinkReader(hardlinkReaderParams);
    MOCKER_CPP(&BackupQueue<FileHandle>::Push, void(BackupQueue<FileHandle>::*)(FileHandle)) // 模板类中重载函数打桩
            .stubs()
            .will(ignoreReturnValue()); 
    MOCKER_CPP(&FileDesc::GetSrcState)
            .stubs()
            .will(returnValue(FileDescState::LSTAT))
            .then(returnValue(FileDescState::INIT))
            .then(returnValue(FileDescState::SRC_OPENED))
            .then(returnValue(FileDescState::READED))
            .then(returnValue(FileDescState::LINK));            
    MOCKER_CPP(&FileDesc::SetDstState)
            .stubs()
            .will(ignoreReturnValue());
    MOCKER_CPP_VIRTUAL(libsmbHardlinkReader, &LibsmbHardlinkReader::OpenFile) // 虚函数打桩问题
            .stubs()
            .will(ignoreReturnValue());
    MOCKER_CPP_VIRTUAL(libsmbHardlinkReader, &LibsmbHardlinkReader::ReadData)
            .stubs()
            .will(ignoreReturnValue());
    MOCKER_CPP_VIRTUAL(libsmbHardlinkReader, &LibsmbHardlinkReader::CloseFile)
            .stubs()
            .will(ignoreReturnValue());
    FileHandle fileHandle;
    BackupIOEngine srcIoEngine;
    BackupIOEngine dstIoEngine;
    fileHandle.m_file = std::make_shared<FileDesc>(srcIoEngine, dstIoEngine); // 桩函数中创建智能指针，需要形参
    fileHandle.m_file->m_fileName = "nanjing";
    fileHandle.m_file->SetFlag(IS_DIR);
    EXPECT_NO_THROW(libsmbHardlinkReader.ProcessFileDescState(fileHandle));
    fileHandle.m_file->ClearFlag(IS_DIR);
    fileHandle.m_file->m_mode = FILE_HAVE_ADS;
    EXPECT_NO_THROW(libsmbHardlinkReader.ProcessFileDescState(fileHandle));
    EXPECT_NO_THROW(libsmbHardlinkReader.ProcessFileDescState(fileHandle));
    EXPECT_NO_THROW(libsmbHardlinkReader.ProcessFileDescState(fileHandle));
    EXPECT_NO_THROW(libsmbHardlinkReader.ProcessFileDescState(fileHandle));
}

/*
 * 用例名称：Init
 * 前置条件：无
 * check点：初始化
 */
TEST_F(LibsmbHardlinkReaderTest2, Init) 
{   
    MOCKER_CPP(FillContextParams)
            .stubs()
            .will(ignoreReturnValue());
    ReaderParams hardlinkReaderParams;
    BackupParams backupParams;
    std::shared_ptr<LibsmbBackupAdvanceParams> srcAdvParams = std::make_shared<LibsmbBackupAdvanceParams>();
    srcAdvParams->serverCheckMaxCount = 0;
    backupParams.srcAdvParams = srcAdvParams;
    hardlinkReaderParams.backupParams = backupParams;
    hardlinkReaderParams.controlInfo = std::make_shared<BackupControlInfo>();
    hardlinkReaderParams.blockBufferMap = std::make_shared<BlockBufferMap>();
    LibsmbHardlinkReader libsmbHardlinkReader(hardlinkReaderParams);
    FileHandle fileHandle;
    BackupIOEngine srcIoEngine;
    BackupIOEngine dstIoEngine;
    fileHandle.m_file = std::make_shared<FileDesc>(srcIoEngine, dstIoEngine); // 桩函数中创建智能指针，需要形参
    fileHandle.m_file->m_fileName = "nanjing";
    fileHandle.m_file->SetFlag(IS_DIR);
    EXPECT_NO_THROW(libsmbHardlinkReader.Init(fileHandle));
}

/*
 * 用例名称：OpenFile
 * 前置条件：无
 * check点：打开文件
 */
TEST_F(LibsmbHardlinkReaderTest2, OpenFile) 
{   
    MOCKER_CPP(FillContextParams)
            .stubs()
            .will(ignoreReturnValue());
    ReaderParams hardlinkReaderParams;
    BackupParams backupParams;
    std::shared_ptr<LibsmbBackupAdvanceParams> srcAdvParams = std::make_shared<LibsmbBackupAdvanceParams>();
    srcAdvParams->serverCheckMaxCount = 0;
    backupParams.srcAdvParams = srcAdvParams;
    hardlinkReaderParams.backupParams = backupParams;
    hardlinkReaderParams.controlInfo = std::make_shared<BackupControlInfo>();
    hardlinkReaderParams.blockBufferMap = std::make_shared<BlockBufferMap>();
    LibsmbHardlinkReader libsmbHardlinkReader(hardlinkReaderParams);
    BackupQueueConfig config;
    config.maxSize = 100;
    config.maxMemorySize = 100;
    libsmbHardlinkReader.m_readQueue = std::make_shared<BackupQueue<FileHandle>>(config);
    libsmbHardlinkReader.m_aggregateQueue = std::make_shared<BackupQueue<FileHandle>>(config);
    MOCKER_CPP(&FileDesc::SetSrcState)
            .stubs()
            .will(ignoreReturnValue());
    MOCKER_CPP(SendReaderRequest)
            .stubs()
            .will(returnValue(-1))
            .then(returnValue(0))
            .then(returnValue(-1))
            .then(returnValue(0));
    FileHandle fileHandle;
    BackupIOEngine srcIoEngine;
    BackupIOEngine dstIoEngine;
    fileHandle.m_file = std::make_shared<FileDesc>(srcIoEngine, dstIoEngine); // 桩函数中创建智能指针，需要形参
    fileHandle.m_file->m_fileName = "nanjing";
    fileHandle.m_file->SetFlag(IS_DIR);
    fileHandle.m_file->m_size = 0;
    EXPECT_EQ(libsmbHardlinkReader.OpenFile(fileHandle), 0);
    fileHandle.m_file->m_size = 1;
    EXPECT_EQ(libsmbHardlinkReader.OpenFile(fileHandle), -1);
    EXPECT_EQ(libsmbHardlinkReader.OpenFile(fileHandle), 0);
}

/*
 * 用例名称：CloseFile
 * 前置条件：无
 * check点：关闭文件
 */
TEST_F(LibsmbHardlinkReaderTest2, CloseFile) 
{   
    MOCKER_CPP(FillContextParams)
            .stubs()
            .will(ignoreReturnValue());
    ReaderParams hardlinkReaderParams;
    BackupParams backupParams;
    std::shared_ptr<LibsmbBackupAdvanceParams> srcAdvParams = std::make_shared<LibsmbBackupAdvanceParams>();
    srcAdvParams->serverCheckMaxCount = 0;
    backupParams.srcAdvParams = srcAdvParams;
    hardlinkReaderParams.backupParams = backupParams;
    hardlinkReaderParams.controlInfo = std::make_shared<BackupControlInfo>();
    hardlinkReaderParams.blockBufferMap = std::make_shared<BlockBufferMap>();
    LibsmbHardlinkReader libsmbHardlinkReader(hardlinkReaderParams);
    MOCKER_CPP(SendReaderRequest)
            .stubs()
            .will(returnValue(-1))
            .then(returnValue(0));
    FileHandle fileHandle;
    BackupIOEngine srcIoEngine;
    BackupIOEngine dstIoEngine;
    fileHandle.m_file = std::make_shared<FileDesc>(srcIoEngine, dstIoEngine); // 桩函数中创建智能指针，需要形参
    fileHandle.m_file->m_fileName = "nanjing";
    fileHandle.m_file->SetFlag(IS_DIR);
    EXPECT_EQ(libsmbHardlinkReader.CloseFile(fileHandle), -1);
    EXPECT_EQ(libsmbHardlinkReader.CloseFile(fileHandle), 0);
}

/*
 * 用例名称：ReadMeta
 * 前置条件：无
 * check点：关闭文件
 */
TEST_F(LibsmbHardlinkReaderTest2, ReadMeta) 
{   
    MOCKER_CPP(FillContextParams)
            .stubs()
            .will(ignoreReturnValue());
    ReaderParams hardlinkReaderParams;
    BackupParams backupParams;
    std::shared_ptr<LibsmbBackupAdvanceParams> srcAdvParams = std::make_shared<LibsmbBackupAdvanceParams>();
    srcAdvParams->serverCheckMaxCount = 0;
    backupParams.srcAdvParams = srcAdvParams;
    hardlinkReaderParams.backupParams = backupParams;
    hardlinkReaderParams.controlInfo = std::make_shared<BackupControlInfo>();
    hardlinkReaderParams.blockBufferMap = std::make_shared<BlockBufferMap>();
    LibsmbHardlinkReader libsmbHardlinkReader(hardlinkReaderParams);
    BackupQueueConfig config;
    config.maxSize = 100;
    config.maxMemorySize = 100;
    libsmbHardlinkReader.m_readQueue = std::make_shared<BackupQueue<FileHandle>>(config);
    MOCKER_CPP(&FileDesc::SetSrcState)
            .stubs()
            .will(ignoreReturnValue());  
    FileHandle fileHandle;
    BackupIOEngine srcIoEngine;
    BackupIOEngine dstIoEngine;
    fileHandle.m_file = std::make_shared<FileDesc>(srcIoEngine, dstIoEngine); // 桩函数中创建智能指针，需要形参
    fileHandle.m_file->m_fileName = "nanjing";
    fileHandle.m_file->SetFlag(IS_DIR);
    EXPECT_EQ(libsmbHardlinkReader.ReadMeta(fileHandle), 0);
}

/*
 * 用例名称：ReadData
 * 前置条件：无
 * check点：读数据
 */
TEST_F(LibsmbHardlinkReaderTest2, ReadData) 
{   
    MOCKER_CPP(FillContextParams)
            .stubs()
            .will(ignoreReturnValue());
    ReaderParams hardlinkReaderParams;
    BackupParams backupParams;
    std::shared_ptr<LibsmbBackupAdvanceParams> srcAdvParams = std::make_shared<LibsmbBackupAdvanceParams>();
    srcAdvParams->serverCheckMaxCount = 0;
    backupParams.srcAdvParams = srcAdvParams;
    hardlinkReaderParams.backupParams = backupParams;
    hardlinkReaderParams.controlInfo = std::make_shared<BackupControlInfo>();
    hardlinkReaderParams.blockBufferMap = std::make_shared<BlockBufferMap>();
    LibsmbHardlinkReader libsmbHardlinkReader(hardlinkReaderParams);
    MOCKER_CPP(&LibsmbHardlinkReader::ReadNormalData)
            .stubs()
            .will(returnValue(0));
    FileHandle fileHandle;
    BackupIOEngine srcIoEngine;
    BackupIOEngine dstIoEngine;
    fileHandle.m_file = std::make_shared<FileDesc>(srcIoEngine, dstIoEngine); // 桩函数中创建智能指针，需要形参
    fileHandle.m_file->m_fileName = "nanjing";
    fileHandle.m_file->SetFlag(IS_DIR);
    fileHandle.m_file->m_size = 0;
    EXPECT_NO_THROW(libsmbHardlinkReader.ReadData(fileHandle));
    fileHandle.m_file->m_size = 1;
    EXPECT_NO_THROW(libsmbHardlinkReader.ReadData(fileHandle));
}

/*
 * 用例名称：ReadNormalData
 * 前置条件：无
 * check点：读meta
 */
TEST_F(LibsmbHardlinkReaderTest2, ReadNormalData) 
{   
    MOCKER_CPP(FillContextParams)
            .stubs()
            .will(ignoreReturnValue());
    ReaderParams hardlinkReaderParams;
    BackupParams backupParams;
    std::shared_ptr<LibsmbBackupAdvanceParams> srcAdvParams = std::make_shared<LibsmbBackupAdvanceParams>();
    srcAdvParams->serverCheckMaxCount = 0;
    backupParams.srcAdvParams = srcAdvParams;
    hardlinkReaderParams.backupParams = backupParams;
    hardlinkReaderParams.controlInfo = std::make_shared<BackupControlInfo>();
    hardlinkReaderParams.blockBufferMap = std::make_shared<BlockBufferMap>();
    LibsmbHardlinkReader libsmbHardlinkReader(hardlinkReaderParams);
    MOCKER_CPP(SendReaderRequest)
            .stubs()
            .will(returnValue(-1))
            .then(returnValue(0));
    MOCKER_CPP(&BlockBufferMap::Add)
            .stubs()
            .will(ignoreReturnValue()); 
    FileHandle fileHandle;
    BackupIOEngine srcIoEngine;
    BackupIOEngine dstIoEngine;
    fileHandle.m_file = std::make_shared<FileDesc>(srcIoEngine, dstIoEngine); // 桩函数中创建智能指针，需要形参
    fileHandle.m_file->m_fileName = "nanjing";
    fileHandle.m_file->SetFlag(IS_DIR);
    fileHandle.m_file->m_size = 0;
    EXPECT_NO_THROW(libsmbHardlinkReader.ReadNormalData(fileHandle));
    EXPECT_NO_THROW(libsmbHardlinkReader.ReadNormalData(fileHandle));
}

/*
 * 用例名称：Start
 * 前置条件：无
 * check点：开启线程
 */
TEST_F(LibsmbHardlinkReaderTest2, Start) 
{
    MOCKER_CPP(FillContextParams)
            .stubs()
            .will(ignoreReturnValue());
    ReaderParams hardlinkReaderParams;
    BackupParams backupParams;
    std::shared_ptr<LibsmbBackupAdvanceParams> srcAdvParams = std::make_shared<LibsmbBackupAdvanceParams>();
    srcAdvParams->serverCheckMaxCount = 0;
    backupParams.srcAdvParams = srcAdvParams;
    hardlinkReaderParams.backupParams = backupParams;
    hardlinkReaderParams.controlInfo = std::make_shared<BackupControlInfo>();
    hardlinkReaderParams.blockBufferMap = std::make_shared<BlockBufferMap>();
    LibsmbHardlinkReader libsmbHardlinkReader(hardlinkReaderParams);
    MOCKER_CPP(&LibsmbHardlinkReader::SmbConnectContexts)
            .stubs()
            .will(returnValue(-1))
            .then(returnValue(0));
    EXPECT_EQ(libsmbHardlinkReader.Start(), BackupRetCode::FAILED);
}

/*
 * 用例名称：IsComplete
 * 前置条件：无
 * check点：判断是否完成
 */
TEST_F(LibsmbHardlinkReaderTest2, IsComplete) 
{
    MOCKER_CPP(FillContextParams)
            .stubs()
            .will(ignoreReturnValue());
    ReaderParams hardlinkReaderParams;
    BackupParams backupParams;
    std::shared_ptr<LibsmbBackupAdvanceParams> srcAdvParams = std::make_shared<LibsmbBackupAdvanceParams>();
    srcAdvParams->serverCheckMaxCount = 0;
    backupParams.srcAdvParams = srcAdvParams;
    hardlinkReaderParams.backupParams = backupParams;
    hardlinkReaderParams.controlInfo = std::make_shared<BackupControlInfo>();
    hardlinkReaderParams.blockBufferMap = std::make_shared<BlockBufferMap>();
    LibsmbHardlinkReader libsmbHardlinkReader(hardlinkReaderParams);
    MOCKER_CPP(&PacketStats::GetValue)
            .stubs()
            .will(returnValue(105));
    BackupQueueConfig config;
    config.maxSize = 100;
    config.maxMemorySize = 100;
    libsmbHardlinkReader.m_readQueue = std::make_shared<BackupQueue<FileHandle>>(config);
    libsmbHardlinkReader.m_controlInfo->m_aggregatePhaseComplete = false;
    EXPECT_EQ(libsmbHardlinkReader.IsComplete(), false);
    EXPECT_EQ(libsmbHardlinkReader.IsComplete(), false);
}

/*
 * 用例名称：LibsmbHardlinkReaderTest2
 * 前置条件：无
 * check点：处理连接异常
 */
TEST_F(LibsmbHardlinkReaderTest2, ProcessConnectionException) 
{
    MOCKER_CPP(FillContextParams)
            .stubs()
            .will(ignoreReturnValue());
    ReaderParams hardlinkReaderParams;
    BackupParams backupParams;
    std::shared_ptr<LibsmbBackupAdvanceParams> srcAdvParams = std::make_shared<LibsmbBackupAdvanceParams>();
    srcAdvParams->serverCheckMaxCount = 0;
    backupParams.srcAdvParams = srcAdvParams;
    hardlinkReaderParams.backupParams = backupParams;
    hardlinkReaderParams.controlInfo = std::make_shared<BackupControlInfo>();
    hardlinkReaderParams.blockBufferMap = std::make_shared<BlockBufferMap>();
    LibsmbHardlinkReader libsmbHardlinkReader(hardlinkReaderParams);
    MOCKER_CPP(&PacketStats::ResetErrorCounter)
            .stubs()
            .will(ignoreReturnValue());
    MOCKER_CPP(HandleConnectionException)
            .stubs()
            .will(returnValue(-1))
            .then(returnValue(0));
    EXPECT_EQ(libsmbHardlinkReader.ProcessConnectionException(), -1);
    EXPECT_EQ(libsmbHardlinkReader.ProcessConnectionException(), 0);
}

/*
 * 用例名称：ProcessReadEntries
 * 前置条件：无
 * check点：处理读entries
 */
TEST_F(LibsmbHardlinkReaderTest2, ProcessReadEntries) 
{   MOCKER_CPP(FillContextParams)
            .stubs()
            .will(ignoreReturnValue());
    ReaderParams hardlinkReaderParams;
    BackupParams backupParams;
    std::shared_ptr<LibsmbBackupAdvanceParams> srcAdvParams = std::make_shared<LibsmbBackupAdvanceParams>();
    srcAdvParams->serverCheckMaxCount = 0;
    backupParams.srcAdvParams = srcAdvParams;
    hardlinkReaderParams.backupParams = backupParams;
    hardlinkReaderParams.controlInfo = std::make_shared<BackupControlInfo>();
    hardlinkReaderParams.blockBufferMap = std::make_shared<BlockBufferMap>();
    LibsmbHardlinkReader libsmbHardlinkReader(hardlinkReaderParams);
    MOCKER_CPP(&PacketStats::GetValue)
            .stubs()
            .will(returnValue(105));
    BackupQueueConfig config;
    config.maxSize = 100;
    config.maxMemorySize = 100;
    libsmbHardlinkReader.m_readQueue = std::make_shared<BackupQueue<FileHandle>>(config);
    FileHandle fileHandle;
    BackupIOEngine srcIoEngine;
    BackupIOEngine dstIoEngine;
    fileHandle.m_file = std::make_shared<FileDesc>(srcIoEngine, dstIoEngine); // 桩函数中创建智能指针，需要形参
    fileHandle.m_file->m_fileName = "nanjing";
    fileHandle.m_file->SetFlag(IS_DIR);
    fileHandle.m_file->m_size = 0;
    libsmbHardlinkReader.m_readQueue->Push(fileHandle);
    MOCKER_CPP(&BackupQueue<FileHandle>::WaitAndPop, bool(BackupQueue<FileHandle>::*)(FileHandle&, uint32_t)) // 模板类中重载函数打桩
            .stubs()
            .will(invoke(StubWaitAndPopTrue))
            .then(invoke(StubWaitAndPopFalse));
    MOCKER_CPP(&LibsmbHardlinkReader::ProcessFileDescState)
            .stubs()
            .will(ignoreReturnValue());
    MOCKER_CPP(&LibsmbHardlinkReader::IsReaderRequestReachThreshold)
            .stubs()
            .will(ignoreReturnValue());
    EXPECT_NO_THROW(libsmbHardlinkReader.ProcessReadEntries());
    EXPECT_NO_THROW(libsmbHardlinkReader.ProcessReadEntries());
}

/*
 * 用例名称：ProcessPartialReadEntries
 * 前置条件：无
 * check点：处理读entries
 */
TEST_F(LibsmbHardlinkReaderTest2, ProcessPartialReadEntries) 
{   MOCKER_CPP(FillContextParams)
            .stubs()
            .will(ignoreReturnValue());
    ReaderParams hardlinkReaderParams;
    BackupParams backupParams;
    std::shared_ptr<LibsmbBackupAdvanceParams> srcAdvParams = std::make_shared<LibsmbBackupAdvanceParams>();
    srcAdvParams->serverCheckMaxCount = 0;
    backupParams.srcAdvParams = srcAdvParams;
    hardlinkReaderParams.backupParams = backupParams;
    hardlinkReaderParams.controlInfo = std::make_shared<BackupControlInfo>();
    hardlinkReaderParams.blockBufferMap = std::make_shared<BlockBufferMap>();
    LibsmbHardlinkReader libsmbHardlinkReader(hardlinkReaderParams);
    MOCKER_CPP(&PacketStats::GetValue)
            .stubs()
            .will(returnValue(105));
    BackupQueueConfig config;
    config.maxSize = 100;
    config.maxMemorySize = 100;
    libsmbHardlinkReader.m_readQueue = std::make_shared<BackupQueue<FileHandle>>(config);
    libsmbHardlinkReader.m_partialReadQueue = std::make_shared<BackupQueue<FileHandle>>(config);
    FileHandle fileHandle;
    BackupIOEngine srcIoEngine;
    BackupIOEngine dstIoEngine;
    fileHandle.m_file = std::make_shared<FileDesc>(srcIoEngine, dstIoEngine); // 桩函数中创建智能指针，需要形参
    fileHandle.m_file->m_fileName = "nanjing";
    fileHandle.m_file->SetFlag(IS_DIR);
    fileHandle.m_file->m_size = 0;
    libsmbHardlinkReader.m_readQueue->Push(fileHandle);
    libsmbHardlinkReader.m_partialReadQueue->Push(fileHandle);
    MOCKER_CPP(&BackupQueue<FileHandle>::WaitAndPop, bool(BackupQueue<FileHandle>::*)(FileHandle&, uint32_t)) // 模板类中重载函数打桩
            .stubs()
            .will(invoke(StubWaitAndPopTrue))
            .then(invoke(StubWaitAndPopFalse));
    MOCKER_CPP(&LibsmbHardlinkReader::ProcessFileDescState)
            .stubs()
            .will(ignoreReturnValue());
    MOCKER_CPP(&LibsmbHardlinkReader::IsAbort)
            .stubs()
            .will(returnValue(true))
            .then(returnValue(false));
    MOCKER_CPP(&PacketStats::GetValue)
            .stubs()
            .will(returnValue(300))
            .then(returnValue(0));
    EXPECT_NO_THROW(libsmbHardlinkReader.ProcessPartialReadEntries());
    EXPECT_NO_THROW(libsmbHardlinkReader.ProcessPartialReadEntries());
    EXPECT_NO_THROW(libsmbHardlinkReader.ProcessPartialReadEntries());
}