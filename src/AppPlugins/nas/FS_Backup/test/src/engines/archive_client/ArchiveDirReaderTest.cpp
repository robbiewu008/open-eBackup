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
#include "ArchiveDirReader.h"
#include "ThreadPoolFactory.h"
#include "log/Log.h"

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "stub.h"


using namespace std;
using namespace Module;


namespace {

class ArchiveClientTest : public ArchiveClientBase {
public:
    ArchiveClientTest() {}
    ~ArchiveClientTest() {}

    virtual int GetFileData(const ArchiveRequest& req, ArchiveResponse& rsp) override { return SUCCESS; }
};

}

class ArchiveDirReaderTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();

    std::unique_ptr<ArchiveDirReader> archiveDirReader = nullptr;
};

void ArchiveDirReaderTest::SetUp()
{
    BackupQueueConfig config {DEFAULT_BACKUP_QUEUE_SIZE, DEFAULT_BACKUP_QUEUE_MEMORY_SIZE};
    
    BackupParams params;
    params.srcEngine = BackupIOEngine::ARCHIVE_CLIENT;
    params.dstEngine = BackupIOEngine::POSIX;
    params.srcAdvParams = std::make_shared<ArchiveRestoreAdvanceParams>(std::make_shared<ArchiveClientTest>());
    params.dstAdvParams = std::make_shared<HostBackupAdvanceParams>();

    ReaderParams dirReaderParams {};
    dirReaderParams.backupParams = params;
    dirReaderParams.readQueuePtr = std::make_shared<BackupQueue<FileHandle>>(config);
    dirReaderParams.aggregateQueuePtr = std::make_shared<BackupQueue<FileHandle>>(config);
    dirReaderParams.blockBufferMap = std::make_shared<BlockBufferMap>();
    dirReaderParams.controlInfo = std::make_shared<BackupControlInfo>();

    archiveDirReader = std::make_unique<ArchiveDirReader>(dirReaderParams);
}

void ArchiveDirReaderTest::TearDown()
{}

void ArchiveDirReaderTest::SetUpTestCase()
{}

void ArchiveDirReaderTest::TearDownTestCase()
{}

void Function_Void()
{}

/*
* 用例名称：Start
* 前置条件：
* check点：ArchiveDirReader启动
*/
TEST_F(ArchiveDirReaderTest, Start)
{
    auto ret = archiveDirReader->Start();
    sleep(2); // sleep for 2 second
    archiveDirReader->m_abort = true;
    EXPECT_EQ(ret, BackupRetCode::SUCCESS);
}

/*
* 用例名称：Abort
* 前置条件：
* check点：中止
*/
TEST_F(ArchiveDirReaderTest, Abort)
{
    BackupRetCode ret = archiveDirReader -> Abort();
    EXPECT_EQ(ret, BackupRetCode::SUCCESS);
}

/*
* 用例名称：Destroy
* 前置条件：
* check点：销毁
*/
TEST_F(ArchiveDirReaderTest, Destroy)
{
    archiveDirReader->m_threadDone = false;
    BackupRetCode ret = archiveDirReader->Destroy();
    EXPECT_EQ(ret, BackupRetCode::DESTROY_FAILED_BACKUP_IN_PROGRESS);

    archiveDirReader->m_threadDone = true;
    ret = archiveDirReader->Destroy();
    EXPECT_EQ(ret, BackupRetCode::SUCCESS);
    archiveDirReader->m_threadDone = false;
}

/*
* 用例名称：GetStatus
* 前置条件：
* check点：获取状态
*/
TEST_F(ArchiveDirReaderTest, GetStatus)
{
    BackupPhaseStatus ret = archiveDirReader -> GetStatus();
    EXPECT_EQ(ret, BackupPhaseStatus::INPROGRESS);
}

/*
* 用例名称：IsComplete
* 前置条件：
* check点：文件拷贝状态完成
*/
TEST_F(ArchiveDirReaderTest, IsComplete)
{
    archiveDirReader -> HandleComplete();
    bool ret = archiveDirReader -> IsComplete();
    EXPECT_EQ(ret, false);

    archiveDirReader->m_controlInfo->m_controlReaderFailed = true;
    ret = archiveDirReader->IsComplete();
    EXPECT_EQ(ret, true);
}

TEST_F(ArchiveDirReaderTest, Others)
{
    FileHandle fileHandle;
    EXPECT_EQ(archiveDirReader->OpenFile(fileHandle), Module::SUCCESS);
    EXPECT_EQ(archiveDirReader->ReadData(fileHandle), Module::SUCCESS);
    EXPECT_EQ(archiveDirReader->ReadMeta(fileHandle), Module::SUCCESS);
    EXPECT_EQ(archiveDirReader->CloseFile(fileHandle), Module::SUCCESS);
}
