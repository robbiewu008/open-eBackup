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
#include <stdexcept>
#include "gtest/gtest.h"
#include "mockcpp/mockcpp.hpp"
#include "NfsWormWriter.h"

using namespace std;
using namespace FS_Backup;
using namespace Module;

namespace  {
    const int SUCCESS = 0;
    const int FAILED = -1;
}

class NfsWormWriterTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    BackupParams m_backupParams {};
    std::unique_ptr<NfsWormWriter> m_nfsWormWriter = nullptr;
    std::shared_ptr<NfsAntiRansomwareAdvanceParams> m_advParams = nullptr;
};

void NfsWormWriterTest::SetUp()
{
    m_backupParams.backupType = BackupType::BACKUP_FULL;
    m_backupParams.srcEngine = BackupIOEngine::NFS_ANTI_ANSOMWARE;
    m_backupParams.dstEngine = BackupIOEngine::NFS_ANTI_ANSOMWARE;

    NfsAntiRansomwareAdvanceParams nfsAntiRansomwareAdvanceParams {};
    m_backupParams.srcAdvParams = make_shared<NfsAntiRansomwareAdvanceParams>(nfsAntiRansomwareAdvanceParams);
    m_backupParams.dstAdvParams = make_shared<NfsAntiRansomwareAdvanceParams>(nfsAntiRansomwareAdvanceParams);

    BackupQueueConfig config {DEFAULT_BACKUP_QUEUE_SIZE, DEFAULT_BACKUP_QUEUE_MEMORY_SIZE};
    AntiWriterParams antiWriterParams;
    antiWriterParams.controlInfo = make_shared<BackupControlInfo>();
    antiWriterParams.backupParams = m_backupParams;
    antiWriterParams.readQueuePtr = make_shared<BackupQueue<FileHandle>>(config);
    antiWriterParams.writeQueuePtr = make_shared<BackupQueue<FileHandle>>(config);
    antiWriterParams.blockBufferMap = make_shared<BlockBufferMap>();

    m_advParams = dynamic_pointer_cast<NfsAntiRansomwareAdvanceParams>(m_backupParams.srcAdvParams);
    m_nfsWormWriter = make_unique<NfsWormWriter>(antiWriterParams);
}

void NfsWormWriterTest::TearDown()
{
    GlobalMockObject::verify(); // 校验mock规范并清除mock规范
}

void NfsWormWriterTest::SetUpTestCase()
{}

void NfsWormWriterTest::TearDownTestCase()
{}

/*
 * 用例名称：CheckFillWriteContainers
 * 前置条件：
 * check点：FillWriteContainers进入不同分支
 */
TEST_F(NfsWormWriterTest, CheckFillWriteContainers)
{
    MOCKER_CPP(&Libnfscommonmethods::FillNfsContextContainer)
            .stubs()
            .will(returnValue(false))
            .then(returnValue(true));
    EXPECT_NO_THROW(m_nfsWormWriter->FillWriteContainers());
    EXPECT_NO_THROW(m_nfsWormWriter->FillWriteContainers());
}

/*
 * 用例名称：CheckAbort
 * 前置条件：
 * check点：Abort返回BackupRetCode::SUCCESS
 */
TEST_F(NfsWormWriterTest, CheckAbort)
{
    EXPECT_EQ(m_nfsWormWriter->Abort(), BackupRetCode::SUCCESS);
}

/*
 * 用例名称：CheckGetStatus
 * 前置条件：
 * check点：GetStatus进入不同分支
 */
TEST_F(NfsWormWriterTest, CheckGetStatus)
{
    m_nfsWormWriter->m_controlInfo->m_writePhaseComplete = false;
    EXPECT_EQ(m_nfsWormWriter->GetStatus(), BackupPhaseStatus::INPROGRESS);

    m_nfsWormWriter->m_controlInfo->m_writePhaseComplete = true;
    m_nfsWormWriter->m_abort = true;
    EXPECT_EQ(m_nfsWormWriter->GetStatus(), BackupPhaseStatus::ABORTED);

    m_nfsWormWriter->m_abort = false;
    m_nfsWormWriter->m_controlInfo->m_failed = true;
    m_nfsWormWriter->m_controlInfo->m_controlReaderFailed = true;
    EXPECT_EQ(m_nfsWormWriter->GetStatus(), BackupPhaseStatus::FAILED);

    m_nfsWormWriter->m_controlInfo->m_failed = false;
    m_nfsWormWriter->m_controlInfo->m_controlReaderFailed = false;
    EXPECT_EQ(m_nfsWormWriter->GetStatus(), BackupPhaseStatus::COMPLETED);
}

/*
 * 用例名称：CheckIsAbort
 * 前置条件：
 * check点：IsAbort进入不同分支
 */
TEST_F(NfsWormWriterTest, CheckIsAbort)
{
    m_nfsWormWriter->m_abort = true;
    m_nfsWormWriter->m_controlInfo->m_failed = true;
    m_nfsWormWriter->m_controlInfo->m_controlReaderFailed = true;
    EXPECT_EQ(m_nfsWormWriter->IsAbort(), true);

    m_nfsWormWriter->m_abort = false;
    m_nfsWormWriter->m_controlInfo->m_failed = false;
    m_nfsWormWriter->m_controlInfo->m_controlReaderFailed = false;
    EXPECT_EQ(m_nfsWormWriter->IsAbort(), false);
}

/*
 * 用例名称：CheckIsComplete
 * 前置条件：
 * check点：IsComplete进入不同分支
 */
TEST_F(NfsWormWriterTest, CheckIsComplete)
{
    m_nfsWormWriter->m_controlInfo->m_readPhaseComplete = 1;
    EXPECT_NO_THROW(m_nfsWormWriter->IsComplete());
}

/*
 * 用例名称：CheckHandleComplete
 * 前置条件：
 * check点：m_writePhaseComplete成功被修改
 */
TEST_F(NfsWormWriterTest, CheckHandleComplete)
{
    m_nfsWormWriter->HandleComplete();
    EXPECT_EQ(m_nfsWormWriter->m_controlInfo->m_writePhaseComplete, true);
}

/*
 * 用例名称：CheckThreadFunc
 * 前置条件：
 * check点：ThreadFunc进入不同分支
 */
TEST_F(NfsWormWriterTest, CheckThreadFunc)
{
    MOCKER_CPP(&NfsWormWriter::IsAbort)
            .stubs()
            .will(returnValue(false))
            .then(returnValue(true));
    MOCKER_CPP(&NfsWormWriter::NfsServerCheck)
            .stubs()
            .will(returnValue(0));
    MOCKER_CPP(&NfsWormWriter::IsComplete)
            .stubs()
            .will(returnValue(true))
            .then(returnValue(false));
    EXPECT_NO_THROW(m_nfsWormWriter->ThreadFunc());
    EXPECT_NO_THROW(m_nfsWormWriter->ThreadFunc());
    EXPECT_NO_THROW(m_nfsWormWriter->ThreadFunc());
    EXPECT_NO_THROW(m_nfsWormWriter->ThreadFunc());
}

/*
 * 用例名称：CheckSendSetMetaRequest
 * 前置条件：
 * check点：SendSetMetaRequest进入不同分支
 */
TEST_F(NfsWormWriterTest, CheckSendSetMetaRequest)
{
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(m_backupParams.srcEngine, m_backupParams.dstEngine);

    MOCKER_CPP(&SendNfsRequest)
            .stubs()
            .will(returnValue(0))
            .then(returnValue(1));
    MOCKER_CPP(&NfsWormWriter::HandleSendNfsRequestFailure)
            .stubs()
            .will(ignoreReturnValue());
    EXPECT_EQ(m_nfsWormWriter->SendSetMetaRequest(fileHandle), 0);
    EXPECT_EQ(m_nfsWormWriter->SendSetMetaRequest(fileHandle), 1);
}

/*
 * 用例名称：CheckHandleSendNfsRequestFailure
 * 前置条件：
 * check点：HandleSendNfsRequestFailure运行无异常
 */
TEST_F(NfsWormWriterTest, CheckHandleSendNfsRequestFailure)
{
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(m_backupParams.srcEngine, m_backupParams.dstEngine);

    MOCKER_CPP(&FileDesc::SetSrcState)
            .stubs()
            .will(returnValue(FileDescState::INIT));
    MOCKER_CPP(&FileDesc::SetDstState)
            .stubs()
            .will(returnValue(FileDescState::INIT));

    m_nfsWormWriter->m_backupParams.commonParams.skipFailure = false;
    EXPECT_NO_THROW(m_nfsWormWriter->HandleSendNfsRequestFailure(fileHandle));
}

/*
 * 用例名称：CheckNfsServerCheck
 * 前置条件：
 * check点：NfsServerCheck进入不同分支
 */
TEST_F(NfsWormWriterTest, CheckNfsServerCheck)
{
    m_nfsWormWriter->m_advParams->serverCheckMaxCount = 10;
    MOCKER_CPP(&PacketStats::GetValue)
            .stubs()
            .will(returnValue(10));
    MOCKER_CPP(&Libnfscommonmethods::NasServerCheck)
            .stubs()
            .will(returnValue(1))
            .then(returnValue(0));
    EXPECT_EQ(m_nfsWormWriter->NfsServerCheck(), 1);
    EXPECT_EQ(m_nfsWormWriter->NfsServerCheck(), 0);
}

/*
 * 用例名称：CheckOpenFile
 * 前置条件：
 * check点：OpenFile返回SUCCESS
 */
TEST_F(NfsWormWriterTest, CheckOpenFile)
{
    FileHandle fileHandle {};
    EXPECT_EQ(m_nfsWormWriter->OpenFile(fileHandle), 0);
}

/*
 * 用例名称：CheckWriteData
 * 前置条件：
 * check点：WriteData返回SUCCESS
 */
TEST_F(NfsWormWriterTest, CheckWriteData)
{
    FileHandle fileHandle {};
    EXPECT_EQ(m_nfsWormWriter->WriteData(fileHandle), 0);
}

/*
 * 用例名称：CheckWriteMeta
 * 前置条件：
 * check点：WriteMeta返回SUCCESS
 */
TEST_F(NfsWormWriterTest, CheckWriteMeta)
{
    FileHandle fileHandle {};
    EXPECT_EQ(m_nfsWormWriter->WriteMeta(fileHandle), 0);
}

/*
 * 用例名称：CheckCloseFile
 * 前置条件：
 * check点：CloseFile返回SUCCESS
 */
TEST_F(NfsWormWriterTest, CheckCloseFile)
{
    FileHandle fileHandle {};
    EXPECT_EQ(m_nfsWormWriter->CloseFile(fileHandle), 0);
}

/*
 * 用例名称：CheckProcRetryTimers
 * 前置条件：
 * check点：ProcRetryTimers运行无异常
 */
TEST_F(NfsWormWriterTest, CheckProcRetryTimers)
{
    EXPECT_NO_THROW(m_nfsWormWriter->ProcRetryTimers());
}

/*
 * 用例名称：CheckGetRetryTimerCnt
 * 前置条件：
 * check点：GetRetryTimerCnt运行无异常
 */
TEST_F(NfsWormWriterTest, CheckGetRetryTimerCnt)
{
    EXPECT_NO_THROW(m_nfsWormWriter->GetRetryTimerCnt());
}
