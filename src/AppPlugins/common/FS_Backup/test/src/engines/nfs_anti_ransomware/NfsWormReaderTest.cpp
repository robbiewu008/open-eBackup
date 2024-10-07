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
#include "NfsWormReader.h"

using namespace std;
using namespace FS_Backup;
using namespace Module;

namespace  {
    const int SUCCESS = 0;
    const int FAILED = -1;
}

class NfsWormReaderTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    BackupParams m_backupParams {};
    std::unique_ptr<NfsWormReader> m_nfsWormReader = nullptr;
    std::shared_ptr<NfsAntiRansomwareAdvanceParams> m_advParams = nullptr;
};

void NfsWormReaderTest::SetUp()
{
    m_backupParams.backupType = BackupType::BACKUP_FULL;
    m_backupParams.srcEngine = BackupIOEngine::NFS_ANTI_ANSOMWARE;
    m_backupParams.dstEngine = BackupIOEngine::NFS_ANTI_ANSOMWARE;

    NfsAntiRansomwareAdvanceParams nfsAntiRansomwareAdvanceParams {};
    m_backupParams.srcAdvParams = make_shared<NfsAntiRansomwareAdvanceParams>(nfsAntiRansomwareAdvanceParams);
    m_backupParams.dstAdvParams = make_shared<NfsAntiRansomwareAdvanceParams>(nfsAntiRansomwareAdvanceParams);

    BackupQueueConfig config {DEFAULT_BACKUP_QUEUE_SIZE, DEFAULT_BACKUP_QUEUE_MEMORY_SIZE};
    AntiReaderParams antiReaderParams;
    antiReaderParams.controlInfo = make_shared<BackupControlInfo>();
    antiReaderParams.backupParams = m_backupParams;
    antiReaderParams.readQueuePtr = make_shared<BackupQueue<FileHandle>>(config);
    antiReaderParams.writeQueuePtr = make_shared<BackupQueue<FileHandle>>(config);
    antiReaderParams.blockBufferMap = make_shared<BlockBufferMap>();

    m_advParams = dynamic_pointer_cast<NfsAntiRansomwareAdvanceParams>(m_backupParams.srcAdvParams);
    m_nfsWormReader = make_unique<NfsWormReader>(antiReaderParams);
}

void NfsWormReaderTest::TearDown()
{
    GlobalMockObject::verify(); // 校验mock规范并清除mock规范
}

void NfsWormReaderTest::SetUpTestCase()
{}

void NfsWormReaderTest::TearDownTestCase()
{}

/*
 * 用例名称：CheckAbort
 * 前置条件：
 * check点：Abort返回BackupRetCode::SUCCESS
 */
TEST_F(NfsWormReaderTest, CheckAbort)
{
    EXPECT_EQ(m_nfsWormReader->Abort(), BackupRetCode::SUCCESS);
}

/*
 * 用例名称：CheckGetStatus
 * 前置条件：
 * check点：GetStatus进入不同分支
 */
TEST_F(NfsWormReaderTest, CheckGetStatus)
{
    m_nfsWormReader->m_controlInfo->m_readPhaseComplete = false;
    EXPECT_EQ(m_nfsWormReader->GetStatus(), BackupPhaseStatus::INPROGRESS);

    m_nfsWormReader->m_controlInfo->m_readPhaseComplete = true;
    m_nfsWormReader->m_abort = true;
    EXPECT_EQ(m_nfsWormReader->GetStatus(), BackupPhaseStatus::ABORTED);

    m_nfsWormReader->m_abort = false;
    m_nfsWormReader->m_controlInfo->m_failed = true;
    m_nfsWormReader->m_controlInfo->m_controlReaderFailed = true;
    EXPECT_EQ(m_nfsWormReader->GetStatus(), BackupPhaseStatus::FAILED);

    m_nfsWormReader->m_controlInfo->m_failed = false;
    m_nfsWormReader->m_controlInfo->m_controlReaderFailed = false;
    EXPECT_EQ(m_nfsWormReader->GetStatus(), BackupPhaseStatus::COMPLETED);
}

/*
 * 用例名称：CheckIsAbort
 * 前置条件：
 * check点：IsAbort进入不同分支
 */
TEST_F(NfsWormReaderTest, CheckIsAbort)
{
    m_nfsWormReader->m_abort = true;
    m_nfsWormReader->m_controlInfo->m_failed = true;
    m_nfsWormReader->m_controlInfo->m_controlReaderFailed = true;
    EXPECT_EQ(m_nfsWormReader->IsAbort(), true);

    m_nfsWormReader->m_abort = false;
    m_nfsWormReader->m_controlInfo->m_failed = false;
    m_nfsWormReader->m_controlInfo->m_controlReaderFailed = false;
    EXPECT_EQ(m_nfsWormReader->IsAbort(), false);
}

/*
 * 用例名称：CheckIsComplete
 * 前置条件：
 * check点：IsComplete进入不同分支
 */
TEST_F(NfsWormReaderTest, CheckIsComplete)
{
    m_nfsWormReader->m_controlInfo->m_controlFileReaderProduce = 1;
    m_nfsWormReader->m_controlInfo->m_readConsume = 1;
    m_nfsWormReader->m_controlInfo->m_controlReaderPhaseComplete = true;
    EXPECT_EQ(m_nfsWormReader->IsComplete(), true);

    m_nfsWormReader->m_controlInfo->m_controlFileReaderProduce = 0;
    m_nfsWormReader->m_controlInfo->m_controlReaderPhaseComplete = false;
    EXPECT_EQ(m_nfsWormReader->IsComplete(), false);
}

/*
 * 用例名称：CheckHandleComplete
 * 前置条件：
 * check点：HandleComplete返回true
 */
TEST_F(NfsWormReaderTest, CheckHandleComplete)
{
    m_nfsWormReader->HandleComplete();
    EXPECT_EQ(m_nfsWormReader->m_controlInfo->m_readPhaseComplete, true);
}

/*
 * 用例名称：CheckThreadFunc
 * 前置条件：
 * check点：ThreadFunc进入不同分支
 */
TEST_F(NfsWormReaderTest, CheckThreadFunc)
{
    MOCKER_CPP(&NfsWormReader::IsAbort)
            .stubs()
            .will(returnValue(true))
            .then(returnValue(false));
    MOCKER_CPP(&NfsWormReader::IsComplete)
            .stubs()
            .will(returnValue(true));
    EXPECT_NO_THROW(m_nfsWormReader->ThreadFunc());
    EXPECT_NO_THROW(m_nfsWormReader->ThreadFunc());
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(m_backupParams.srcEngine, m_backupParams.dstEngine);
    fileHandle.m_file->m_fileName = "1";
    EXPECT_NO_THROW(m_nfsWormReader->ThreadFunc());
}

/*
 * 用例名称：CheckOpenFile
 * 前置条件：
 * check点：OpenFile返回SUCCESS
 */
TEST_F(NfsWormReaderTest, CheckOpenFile)
{
    FileHandle fileHandle {};
    EXPECT_EQ(m_nfsWormReader->OpenFile(fileHandle), 0);
}

/*
 * 用例名称：CheckReadData
 * 前置条件：
 * check点：ReadData返回SUCCESS
 */
TEST_F(NfsWormReaderTest, CheckReadData)
{
    FileHandle fileHandle {};
    EXPECT_EQ(m_nfsWormReader->ReadData(fileHandle), 0);
}

/*
 * 用例名称：CheckReadMeta
 * 前置条件：
 * check点：ReadMeta返回SUCCESS
 */
TEST_F(NfsWormReaderTest, CheckReadMeta)
{
    FileHandle fileHandle {};
    EXPECT_EQ(m_nfsWormReader->ReadMeta(fileHandle), 0);
}

/*
 * 用例名称：CheckCloseFile
 * 前置条件：
 * check点：CloseFile返回SUCCESS
 */
TEST_F(NfsWormReaderTest, CheckCloseFile)
{
    FileHandle fileHandle {};
    EXPECT_EQ(m_nfsWormReader->CloseFile(fileHandle), 0);
}
