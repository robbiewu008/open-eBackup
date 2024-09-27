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
#include "gtest/gtest.h"
#include "mockcpp/mockcpp.hpp"
#include "PosixHardlinkWriter.h"
#include "log/Log.h"
#include "ThreadPool.h"
#include "ThreadPoolFactory.h"
#include "BackupStructs.h"

using namespace std;

class PosixHardLinkWriterTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();

    std::unique_ptr<PosixHardlinkWriter> m_posixHardLinkWriter = nullptr;
};

void PosixHardLinkWriterTest::SetUp()
{
    BackupQueueConfig config { DEFAULT_BACKUP_QUEUE_SIZE, DEFAULT_BACKUP_QUEUE_MEMORY_SIZE };
    BackupParams params;
    params.srcEngine = BackupIOEngine::ARCHIVE_CLIENT;
    params.dstEngine = BackupIOEngine::POSIX;
    params.srcAdvParams = std::make_shared<HostBackupAdvanceParams>();
    params.dstAdvParams = std::make_shared<HostBackupAdvanceParams>();

    WriterParams hardlinkWriterParams {};
    hardlinkWriterParams.backupParams = params;
    hardlinkWriterParams.readQueuePtr = std::make_shared<BackupQueue<FileHandle>>(config);
    hardlinkWriterParams.writeQueuePtr = std::make_shared<BackupQueue<FileHandle>>(config);
    hardlinkWriterParams.controlInfo = std::make_shared<BackupControlInfo>();
    hardlinkWriterParams.hardlinkMap = std::make_shared<HardLinkMap>();

    m_posixHardLinkWriter = std::make_unique<PosixHardlinkWriter>(hardlinkWriterParams);
    m_posixHardLinkWriter->m_jsPtr =
        make_shared<Module::JobScheduler>(*Module::ThreadPoolFactory::GetThreadPoolInstance("test", 2));
}

void PosixHardLinkWriterTest::TearDown()
{
    GlobalMockObject::verify(); // 校验mock规范并清除mock规范
}

void PosixHardLinkWriterTest::SetUpTestCase() {}

void PosixHardLinkWriterTest::TearDownTestCase() {}


/*
 * 用例名称：Abort任务
 * 前置条件：
 * check点：成功
 */
TEST_F(PosixHardLinkWriterTest, Abort)
{
    EXPECT_EQ(m_posixHardLinkWriter->Abort(), BackupRetCode::SUCCESS);
}

/*
 * 用例名称：Destroy
 * 前置条件：无
 * check点：校验Destroy
 */
TEST_F(PosixHardLinkWriterTest, Destroy)
{
    m_posixHardLinkWriter->m_threadDone = true;
    m_posixHardLinkWriter->m_pollThreadDone = true;
    EXPECT_EQ(m_posixHardLinkWriter->Destroy(), BackupRetCode::SUCCESS);

    m_posixHardLinkWriter->m_pollThreadDone = false;
    EXPECT_EQ(m_posixHardLinkWriter->Destroy(), BackupRetCode::DESTROY_FAILED_BACKUP_IN_PROGRESS);

    m_posixHardLinkWriter->m_threadDone = false;
    m_posixHardLinkWriter->m_pollThreadDone = true;
    EXPECT_EQ(m_posixHardLinkWriter->Destroy(), BackupRetCode::DESTROY_FAILED_BACKUP_IN_PROGRESS);
}

/*
 * 用例名称：GetStatus
 * 前置条件：
 * check点：成功
 */
TEST_F(PosixHardLinkWriterTest, GetStatus)
{
    std::shared_ptr<BackupControlInfo> controlInfo = make_shared<BackupControlInfo>();
    m_posixHardLinkWriter->m_controlInfo = controlInfo;
    EXPECT_EQ(m_posixHardLinkWriter->GetStatus(), BackupPhaseStatus::INPROGRESS);

    controlInfo->m_writePhaseComplete = true;
    m_posixHardLinkWriter->m_controlInfo = controlInfo;
    m_posixHardLinkWriter->m_abort = true;
    EXPECT_EQ(m_posixHardLinkWriter->GetStatus(), BackupPhaseStatus::ABORTED);

    m_posixHardLinkWriter->m_abort = false;
    m_posixHardLinkWriter->m_controlInfo->m_controlReaderFailed = false;
    m_posixHardLinkWriter->m_controlInfo->m_failed = true;
    EXPECT_EQ(m_posixHardLinkWriter->GetStatus(), BackupPhaseStatus::FAILED);

    m_posixHardLinkWriter->m_controlInfo->m_failed = false;
    EXPECT_EQ(m_posixHardLinkWriter->GetStatus(), BackupPhaseStatus::COMPLETED);
}

/*
 * 用例名称：IsComplete
 * 前置条件：
 * check点：成功
 */
TEST_F(PosixHardLinkWriterTest, IsComplete)
{
    EXPECT_EQ(m_posixHardLinkWriter->IsComplete(), false);

    MOCKER_CPP(&FSBackupUtils::GetCurrentTime)
            .stubs()
            .will(returnValue(20));
    m_posixHardLinkWriter->m_isCompleteTimer = 5;
    m_posixHardLinkWriter->m_controlInfo->m_aggregatePhaseComplete = true;
    m_posixHardLinkWriter->m_writeQueue->Clear();
    m_posixHardLinkWriter->m_writeCache.clear();
    m_posixHardLinkWriter->m_timer.m_map.clear();
    m_posixHardLinkWriter->m_writeTaskProduce = 0;
    m_posixHardLinkWriter->m_writeTaskConsume = 0;
    EXPECT_EQ(m_posixHardLinkWriter->IsComplete(), true);
}

/*
 * 用例名称：IsAbort
 * 前置条件：
 * check点：成功
 */
TEST_F(PosixHardLinkWriterTest, IsAbort)
{
    m_posixHardLinkWriter->m_abort = true;
    m_posixHardLinkWriter->m_controlInfo->m_failed = false;
    m_posixHardLinkWriter->m_controlInfo->m_controlReaderFailed = false;
    EXPECT_EQ(m_posixHardLinkWriter->IsAbort(), true);

    m_posixHardLinkWriter->m_abort = false;
    EXPECT_EQ(m_posixHardLinkWriter->IsAbort(), false);
}

/*
 * 用例名称：OpenFile
 * 前置条件：
 * check点：成功
 */
TEST_F(PosixHardLinkWriterTest, OpenFile)
{
    FileHandle fileHandle;
    std::shared_ptr<FileDesc> fileDesc = make_shared<FileDesc>(BackupIOEngine::POSIX, BackupIOEngine::POSIX);
    fileHandle.m_file = fileDesc;
    fileHandle.m_file->m_fileName = "opencb_test.txt";
    m_posixHardLinkWriter->m_writeTaskProduce = 1;
    MOCKER_CPP(&Module::JobScheduler::Put)
            .stubs()
            .will(returnValue(false))
            .then(returnValue(true));
    EXPECT_EQ(m_posixHardLinkWriter->OpenFile(fileHandle), -1);
    EXPECT_EQ(m_posixHardLinkWriter->OpenFile(fileHandle), 0);
}

/*
 * 用例名称：WriteMeta
 * 前置条件：
 * check点：成功
 */
TEST_F(PosixHardLinkWriterTest, WriteMeta)
{
    FileHandle fileHandle;
    std::shared_ptr<FileDesc> fileDesc = make_shared<FileDesc>(BackupIOEngine::POSIX, BackupIOEngine::POSIX);
    fileHandle.m_file = fileDesc;
    fileHandle.m_file->m_fileName = "opencb_test.txt";
    m_posixHardLinkWriter->m_writeTaskProduce = 1;
    MOCKER_CPP(&Module::JobScheduler::Put)
            .stubs()
            .will(returnValue(false))
            .then(returnValue(true));
    EXPECT_EQ(m_posixHardLinkWriter->WriteMeta(fileHandle), -1);
    EXPECT_EQ(m_posixHardLinkWriter->WriteMeta(fileHandle), 0);
}

/*
 * 用例名称：WriteData
 * 前置条件：
 * check点：成功
 */
TEST_F(PosixHardLinkWriterTest, WriteData)
{
    FileHandle fileHandle;
    std::shared_ptr<FileDesc> fileDesc = make_shared<FileDesc>(BackupIOEngine::POSIX, BackupIOEngine::POSIX);
    fileHandle.m_file = fileDesc;
    fileHandle.m_file->m_fileName = "opencb_test.txt";
    m_posixHardLinkWriter->m_writeTaskProduce = 1;
    MOCKER_CPP(&Module::JobScheduler::Put)
            .stubs()
            .will(returnValue(false))
            .then(returnValue(true));
    EXPECT_EQ(m_posixHardLinkWriter->WriteData(fileHandle), -1);
    EXPECT_EQ(m_posixHardLinkWriter->WriteData(fileHandle), 0);
}

/*
 * 用例名称：CloseFile
 * 前置条件：
 * check点：成功
 */
TEST_F(PosixHardLinkWriterTest, CloseFile)
{
    FileHandle fileHandle;
    std::shared_ptr<FileDesc> fileDesc = make_shared<FileDesc>(BackupIOEngine::POSIX, BackupIOEngine::POSIX);
    fileHandle.m_file = fileDesc;
    fileHandle.m_file->m_fileName = "opencb_test.txt";
    m_posixHardLinkWriter->m_writeTaskProduce = 1;
    MOCKER_CPP(&Module::JobScheduler::Put)
            .stubs()
            .will(returnValue(false))
            .then(returnValue(true));
    EXPECT_EQ(m_posixHardLinkWriter->CloseFile(fileHandle), -1);
    EXPECT_EQ(m_posixHardLinkWriter->CloseFile(fileHandle), 0);
}

/*
 * 用例名称：InsertWriteCache
 * 前置条件：
 * check点：成功
 */
TEST_F(PosixHardLinkWriterTest, InsertWriteCache)
{
    FileHandle fileHandle;
    std::shared_ptr<FileDesc> fileDesc = make_shared<FileDesc>(BackupIOEngine::POSIX, BackupIOEngine::POSIX);
    fileHandle.m_file = fileDesc;
    fileHandle.m_file->m_fileName = "opencb_test.txt";
    EXPECT_NO_THROW(m_posixHardLinkWriter->InsertWriteCache(fileHandle));
}

/*
 * 用例名称：ClearWriteCache
 * 前置条件：
 * check点：成功
 */
TEST_F(PosixHardLinkWriterTest, ClearWriteCache)
{
    m_posixHardLinkWriter->m_writeCache.clear();
    EXPECT_NO_THROW(m_posixHardLinkWriter->ClearWriteCache());

    FileHandle fileHandle;
    std::shared_ptr<FileDesc> fileDesc = make_shared<FileDesc>(BackupIOEngine::POSIX, BackupIOEngine::POSIX);
    fileHandle.m_file = fileDesc;
    fileHandle.m_file->m_fileName = "opencb_test.txt";
    std::vector<FileHandle> fileHandleList = {fileHandle};
    m_posixHardLinkWriter->m_writeCache.insert(std::make_pair(fileHandle.m_file->m_fileName, fileHandleList));
    m_posixHardLinkWriter->m_writeCache.insert(std::make_pair("test.txt", fileHandleList));
    EXPECT_NO_THROW(m_posixHardLinkWriter->ClearWriteCache());
}

/*
 * 用例名称：InsertWriteCache
 * 前置条件：
 * check点：成功
 */
TEST_F(PosixHardLinkWriterTest, ProcessWriteEntries)
{
    FileHandle fileHandle;
    std::shared_ptr<FileDesc> fileDesc = make_shared<FileDesc>(BackupIOEngine::POSIX, BackupIOEngine::POSIX);
    fileHandle.m_file = fileDesc;
    fileHandle.m_file->m_fileName = "opencb_test.txt";
    fileHandle.m_block.m_size = 0;
    fileHandle.m_block.m_seq = 0;
    fileHandle.m_file->m_size = 1;
    m_posixHardLinkWriter->m_params.blockSize = 3;
    EXPECT_NO_THROW(m_posixHardLinkWriter->ProcessWriteEntries(fileHandle));

    fileHandle.m_block.m_size = 2;
    EXPECT_NO_THROW(m_posixHardLinkWriter->ProcessWriteEntries(fileHandle));

    fileHandle.m_file->SetDstState(FileDescState::INIT);
    fileHandle.m_file->m_size = 4;
    fileHandle.m_block.m_size = 0;
    fileHandle.m_block.m_seq = 0;
    EXPECT_NO_THROW(m_posixHardLinkWriter->ProcessWriteEntries(fileHandle));
    fileHandle.m_block.m_size = 2;
    EXPECT_NO_THROW(m_posixHardLinkWriter->ProcessWriteEntries(fileHandle));

    fileHandle.m_file->SetDstState(FileDescState::DST_OPENED);
    EXPECT_NO_THROW(m_posixHardLinkWriter->ProcessWriteEntries(fileHandle));

    fileHandle.m_file->SetDstState(FileDescState::WRITED);
    EXPECT_NO_THROW(m_posixHardLinkWriter->ProcessWriteEntries(fileHandle));

    fileHandle.m_file->SetDstState(FileDescState::DST_CLOSED);
    EXPECT_NO_THROW(m_posixHardLinkWriter->ProcessWriteEntries(fileHandle));
}

/*
 * 用例名称：ProcessTimers
 * 前置条件：
 * check点：成功
 */
TEST_F(PosixHardLinkWriterTest, ProcessTimers)
{
    EXPECT_EQ(m_posixHardLinkWriter->ProcessTimers(), INT64_MAX);
}

/*
 * 用例名称：HandleFailedEvent
 * 前置条件：
 * check点：成功
 */
TEST_F(PosixHardLinkWriterTest, HandleFailedEvent)
{
    FileHandle fileHandle;
    std::shared_ptr<FileDesc> fileDesc = make_shared<FileDesc>(BackupIOEngine::POSIX, BackupIOEngine::POSIX);
    fileHandle.m_file = fileDesc;
    fileHandle.m_retryCnt= 1;
    fileHandle.m_file->m_fileName = "test";
    m_posixHardLinkWriter->m_blockBufferMap = make_shared<BlockBufferMap>();
    auto task = make_shared<PosixServiceTask>(
            HostEvent::LINK, m_posixHardLinkWriter->m_blockBufferMap, fileHandle, m_posixHardLinkWriter->m_params);
    EXPECT_NO_THROW(m_posixHardLinkWriter->HandleFailedEvent(task));

    fileHandle.m_retryCnt= 3;
    std::shared_ptr<BackupControlInfo> controlInfo = make_shared<BackupControlInfo>();
    controlInfo->m_noOfFilesFailed = 0;
    m_posixHardLinkWriter->m_controlInfo = controlInfo;
    task->m_backupFailReason = BackupPhaseStatus::FAILED;
    MOCKER_CPP(&BlockBufferMap::Delete)
            .stubs()
            .will(ignoreReturnValue());
    EXPECT_NO_THROW(m_posixHardLinkWriter->HandleFailedEvent(task));
}

/*
 * 用例名称：HandleFailedEvent
 * 前置条件：
 * check点：成功
 */
TEST_F(PosixHardLinkWriterTest, HandleFailedEvent1)
{
    FileHandle fileHandle;
    std::shared_ptr<FileDesc> fileDesc = make_shared<FileDesc>(BackupIOEngine::POSIX, BackupIOEngine::POSIX);
    fileHandle.m_file = fileDesc;
    fileHandle.m_retryCnt= 1;
    fileHandle.m_file->m_fileName = "test";
    fileHandle.m_file->SetDstState(FileDescState::DST_CLOSED);
    fileHandle.m_file->SetSrcState(FileDescState::SRC_CLOSED);
    fileHandle.m_retryCnt = 3;
    m_posixHardLinkWriter->m_blockBufferMap = make_shared<BlockBufferMap>();
    auto task = make_shared<PosixServiceTask>(
            HostEvent::LINK, m_posixHardLinkWriter->m_blockBufferMap, fileHandle, m_posixHardLinkWriter->m_params);
    EXPECT_NO_THROW(m_posixHardLinkWriter->HandleFailedEvent(task));

    fileHandle.m_retryCnt= 3;
    std::shared_ptr<BackupControlInfo> controlInfo = make_shared<BackupControlInfo>();
    controlInfo->m_noOfFilesFailed = 0;
    m_posixHardLinkWriter->m_controlInfo = controlInfo;
    task->m_backupFailReason = BackupPhaseStatus::FAILED;
    MOCKER_CPP(&BlockBufferMap::Delete)
            .stubs()
            .will(ignoreReturnValue());
    EXPECT_NO_THROW(m_posixHardLinkWriter->HandleFailedEvent(task));
}

/*
 * 用例名称：HandleFailedEvent
 * 前置条件：
 * check点：成功
 */
TEST_F(PosixHardLinkWriterTest, HandleFailedEvent2)
{
    FileHandle fileHandle;
    std::shared_ptr<FileDesc> fileDesc = make_shared<FileDesc>(BackupIOEngine::POSIX, BackupIOEngine::POSIX);
    fileHandle.m_file = fileDesc;
    fileHandle.m_retryCnt= 1;
    fileHandle.m_file->m_fileName = "test";
    fileHandle.m_file->SetDstState(FileDescState::DST_CLOSED);
    fileHandle.m_file->SetSrcState(FileDescState::SRC_CLOSED);
    fileHandle.m_retryCnt = 3;
    m_posixHardLinkWriter->m_blockBufferMap = make_shared<BlockBufferMap>();
    auto task = make_shared<PosixServiceTask>(
            HostEvent::WRITE_DATA, m_posixHardLinkWriter->m_blockBufferMap, fileHandle, m_posixHardLinkWriter->m_params);
    EXPECT_NO_THROW(m_posixHardLinkWriter->HandleFailedEvent(task));

    fileHandle.m_retryCnt= 3;
    std::shared_ptr<BackupControlInfo> controlInfo = make_shared<BackupControlInfo>();
    controlInfo->m_noOfFilesFailed = 0;
    m_posixHardLinkWriter->m_controlInfo = controlInfo;
    task->m_backupFailReason = BackupPhaseStatus::FAILED;
    MOCKER_CPP(&BlockBufferMap::Delete)
            .stubs()
            .will(ignoreReturnValue());
    EXPECT_NO_THROW(m_posixHardLinkWriter->HandleFailedEvent(task));
}

/*
 * 用例名称：HandleFailedEvent
 * 前置条件：
 * check点：成功
 */
TEST_F(PosixHardLinkWriterTest, HandleSuccessEvent)
{
    FileHandle fileHandle;
    std::shared_ptr<FileDesc> fileDesc = make_shared<FileDesc>(BackupIOEngine::POSIX, BackupIOEngine::POSIX);
    fileHandle.m_file = fileDesc;
    fileHandle.m_retryCnt= 1;
    fileHandle.m_file->m_fileName = "test";
    fileHandle.m_file->SetDstState(FileDescState::WRITE_SKIP);
    m_posixHardLinkWriter->m_blockBufferMap = make_shared<BlockBufferMap>();
    auto task = make_shared<PosixServiceTask>(
            HostEvent::OPEN_DST, m_posixHardLinkWriter->m_blockBufferMap, fileHandle, m_posixHardLinkWriter->m_params);
    EXPECT_NO_THROW(m_posixHardLinkWriter->HandleSuccessEvent(task));

    task->m_event = HostEvent::WRITE_META;
    fileHandle.m_file->SetDstState(FileDescState::WRITE_SKIP);
    std::shared_ptr<BackupControlInfo> controlInfo = make_shared<BackupControlInfo>();
    controlInfo->m_noOfFilesFailed = 0;
    m_posixHardLinkWriter->m_controlInfo = controlInfo;
    EXPECT_NO_THROW(m_posixHardLinkWriter->HandleSuccessEvent(task));

    task->m_event = HostEvent::CLOSE_DST;
    m_posixHardLinkWriter->m_params.writeMeta = true;
    EXPECT_NO_THROW(m_posixHardLinkWriter->HandleSuccessEvent(task));

    m_posixHardLinkWriter->m_params.writeMeta = false;
    EXPECT_NO_THROW(m_posixHardLinkWriter->HandleSuccessEvent(task));

    task->m_event = HostEvent::WRITE_DATA;
    EXPECT_NO_THROW(m_posixHardLinkWriter->HandleSuccessEvent(task));
}
