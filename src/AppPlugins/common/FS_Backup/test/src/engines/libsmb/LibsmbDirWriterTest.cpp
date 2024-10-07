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
#include "LibsmbDirWriter.h"
#include "libsmb_ctx/SmbContextWrapper.h"

using namespace std;
namespace  {
}

class LibsmbDirWriterTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    std::unique_ptr<LibsmbDirWriter> m_ins = nullptr;
};

void LibsmbDirWriterTest::SetUp()
{
    BackupQueueConfig config {DEFAULT_BACKUP_QUEUE_SIZE, DEFAULT_BACKUP_QUEUE_MEMORY_SIZE};
    BackupParams backupParams;
    backupParams.dstAdvParams = std::make_shared<LibsmbBackupAdvanceParams>();

    WriterParams dirWriterParams {};
    dirWriterParams.backupParams = backupParams;
    dirWriterParams.writeQueuePtr = std::make_shared<BackupQueue<FileHandle>>(config);
    dirWriterParams.controlInfo = std::make_shared<BackupControlInfo>();
    dirWriterParams.blockBufferMap = std::make_shared<BlockBufferMap>();

    // LibsmbDirWriter libsmbDirWriter(dirWriterParams);
    m_ins = std::make_unique<LibsmbDirWriter>(dirWriterParams);

}

void LibsmbDirWriterTest::TearDown()
{
    GlobalMockObject::verify(); // 校验mock规范并清除mock规范
}

void LibsmbDirWriterTest::SetUpTestCase()
{}

void LibsmbDirWriterTest::TearDownTestCase()
{}

/*
 * 用例名称：验证LibsmbDirWriter Abort函数
 * 前置条件：无
 * check点：Abort返回正确结果
 */
TEST_F(LibsmbDirWriterTest, Abort) {
    BackupParams backupParams;
    backupParams.dstAdvParams = std::make_shared<LibsmbBackupAdvanceParams>();

    WriterParams dirWriterParams {};
    dirWriterParams.backupParams = backupParams;
    dirWriterParams.writeQueuePtr = nullptr;
    dirWriterParams.controlInfo = std::make_shared<BackupControlInfo>();
    dirWriterParams.blockBufferMap = nullptr;

    LibsmbDirWriter libsmbDirWriter(dirWriterParams);

    EXPECT_EQ(libsmbDirWriter.Abort(), BackupRetCode::SUCCESS);
    EXPECT_EQ(libsmbDirWriter.m_abort, true);
}

/*
 * 用例名称：验证LibsmbDirWriter线程启动
 * 前置条件：无
 * check点：IsComplete返回正确结果
 */
TEST_F(LibsmbDirWriterTest, IsComplete) {
    BackupParams backupParams;
    backupParams.dstAdvParams = std::make_shared<LibsmbBackupAdvanceParams>();

    WriterParams dirWriterParams {};
    dirWriterParams.backupParams = backupParams;
    dirWriterParams.writeQueuePtr = nullptr;
    dirWriterParams.controlInfo = std::make_shared<BackupControlInfo>();
    dirWriterParams.blockBufferMap = nullptr;

    LibsmbDirWriter libsmbDirWriter(dirWriterParams);

    EXPECT_EQ(libsmbDirWriter.IsComplete(), false);
}

/*
 * 用例名称：GetStatus
 * 前置条件：无
 * check点：获取状态
 */
TEST_F(LibsmbDirWriterTest, GetStatus)
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
 * 用例名称：OpenFile
 * 前置条件：无
 * check点：打开文件等
 */
TEST_F(LibsmbDirWriterTest, OpenFile) 
{  
    FileHandle fileHandle;
    EXPECT_EQ(m_ins->OpenFile(fileHandle), 0);
    EXPECT_EQ(m_ins->WriteData(fileHandle), 0);
    EXPECT_EQ(m_ins->CloseFile(fileHandle), 0);
}

/*
 * 用例名称：IsAbort
 * 前置条件：无
 * check点：判断是否终止
 */
TEST_F(LibsmbDirWriterTest, IsAbort) 
{  
    m_ins->m_abort = false;
    m_ins->m_controlInfo->m_failed = false;
    m_ins->m_controlInfo->m_controlReaderFailed = true;
    EXPECT_EQ(m_ins->IsAbort(), true);
    m_ins->m_controlInfo->m_controlReaderFailed = false;
    EXPECT_EQ(m_ins->IsAbort(), false);
}

/*
 * 用例名称：ThreadFunc
 * 前置条件：无
 * check点：线程函数
 */
TEST_F(LibsmbDirWriterTest, ThreadFunc) 
{  
    MOCKER_CPP(&LibsmbDirWriter::IsComplete)
            .stubs()
            .will(returnValue(true))
            .then(returnValue(false))
            .then(returnValue(false))
            .then(returnValue(true));
    MOCKER_CPP(&LibsmbDirWriter::IsAbort)
            .stubs()
            .will(returnValue(true))
            .then(returnValue(false))
            .then(returnValue(true));
    MOCKER_CPP(&BackupQueue<FileHandle>::WaitAndPop, bool(BackupQueue<FileHandle>::*)(FileHandle&, uint32_t)) // 模板类中重载函数打桩
            .stubs()
            .will(returnValue(false))
            .then(returnValue(true));
    MOCKER_CPP(&LibsmbDirWriter::SmbDisconnectContexts)
            .stubs()
            .will(ignoreReturnValue());
    MOCKER_CPP(&Module::SmbContextWrapper::Poll)
            .stubs()
            .will(returnValue(0));
    EXPECT_NO_THROW(m_ins->ThreadFunc());
    EXPECT_NO_THROW(m_ins->ThreadFunc());
    EXPECT_NO_THROW(m_ins->ThreadFunc());
}

static bool StubWaitAndPop(BackupQueue<FileHandle>* This, FileHandle& args, uint32_t args2)
{
    FileHandle fileHandle;
    BackupIOEngine srcIoEngine;
    BackupIOEngine dstIoEngine;
    fileHandle.m_file = std::make_shared<FileDesc>(srcIoEngine, dstIoEngine); // 桩函数中创建智能指针，需要形参
    args = fileHandle;
    return true;
}

TEST_F(LibsmbDirWriterTest, ThreadFunc_2) 
{  
    MOCKER_CPP(&LibsmbDirWriter::IsComplete)
            .stubs()
            .will(returnValue(false))
            .then(returnValue(true));
    MOCKER_CPP(&LibsmbDirWriter::IsAbort)
            .stubs()
            .will(returnValue(false));
    MOCKER_CPP(&BackupQueue<FileHandle>::WaitAndPop, bool(BackupQueue<FileHandle>::*)(FileHandle&, uint32_t)) // 模板类中重载函数打桩
            .stubs()
            .will(invoke(StubWaitAndPop));
    MOCKER_CPP(&BackupQueue<FileHandle>::Push, void(BackupQueue<FileHandle>::*)(FileHandle)) // 模板类中重载函数打桩
            .stubs()
            .will(ignoreReturnValue()); 
    MOCKER_CPP(&FileDesc::GetDstState)
            .stubs()
            .will(returnValue(FileDescState::LSTAT));
    MOCKER_CPP(&Module::SmbContextWrapper::Poll)
            .stubs()
            .will(returnValue(0));
    // MOCKER_CPP(&LibsmbDirWriter::WriteMeta) // 普通类内成员函数打桩报错segmatation fault
    //         .stubs()
    //         .will(ignoreReturnValue());
    MOCKER_CPP(&LibsmbDirWriter::SmbDisconnectContexts)
            .stubs()
            .will(ignoreReturnValue());
    EXPECT_NO_THROW(m_ins->ThreadFunc());
}

/*
 * 用例名称：SmbConnectContexts
 * 前置条件：无
 * check点：smb连接
 */
TEST_F(LibsmbDirWriterTest, SmbConnectContexts) 
{  
    std::shared_ptr<Module::SmbContextWrapper> ptr = nullptr;
    MOCKER_CPP(SmbConnectContext)
            .stubs()
            .will(returnValue(ptr));
    EXPECT_NO_THROW(m_ins->SmbConnectContexts());
}

/*
 * 用例名称：WriteMeta
 * 前置条件：无
 * check点：写meta信息
 */
static SmbWriterCommonData* GetSmbWriterCommonData_Stub(void *obj, FileHandle &fileHandle)
{
    auto cbData = nullptr;
    return cbData;
}

TEST_F(LibsmbDirWriterTest, WriteMeta) 
{
    string smbPath = "/file.txt";
    MOCKER_CPP(RemoveFirstSeparator)
            .stubs()
            .will(returnValue(smbPath));
    MOCKER_CPP(ConcatRootPath)
            .stubs()
            .will(ignoreReturnValue());
    MOCKER_CPP(&Module::SmbContextWrapper::SmbSetBasicInfo)
            .stubs()
            .will(returnValue(0))
            .then(returnValue(-ENOENT))
            .then(returnValue(-EIO))
            .then(returnValue(-EISDIR));
    MOCKER_CPP(&Module::SmbContextWrapper::SmbSetBasicInfoAsync)
            .stubs()
            .will(returnValue(0))
            .then(returnValue(-ENOENT))
            .then(returnValue(-EIO))
            .then(returnValue(-EISDIR));
    string errMsg = "err";
    MOCKER_CPP(&Module::SmbContextWrapper::SmbGetError)
            .stubs()
            .will(returnValue(errMsg));
    MOCKER_CPP(&BackupQueue<FileHandle>::Push, void(BackupQueue<FileHandle>::*)(FileHandle)) // 模板类中重载函数打桩
            .stubs()
            .will(ignoreReturnValue());
    FileHandle fileHandle;
    BackupIOEngine srcIoEngine;
    BackupIOEngine dstIoEngine;
    fileHandle.m_file = std::make_shared<FileDesc>(srcIoEngine, dstIoEngine); 
    EXPECT_EQ(m_ins->WriteMeta(fileHandle), 0);
    EXPECT_EQ(m_ins->WriteMeta(fileHandle), -1);
    EXPECT_EQ(m_ins->WriteMeta(fileHandle), -1);
    EXPECT_EQ(m_ins->WriteMeta(fileHandle), -1);
}
