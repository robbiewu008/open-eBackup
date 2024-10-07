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
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "stub.h"
#include "volume_handlers/volumeHandlerMock.h"
#include "job_controller/jobs/restore/RestoreIOTask.h"
#include "repository_handlers/mock/FileSystemHandlerMock.h"

using ::testing::_;
using ::testing::An;
using ::testing::AtLeast;
using testing::DoAll;
using testing::InitGoogleMock;
using ::testing::Return;
using ::testing::Sequence;
using ::testing::SetArgReferee;
using ::testing::Throw;
using ::testing::Invoke;;

using namespace VirtPlugin;

namespace HDT_TEST {
class VolumeHandlerMock;

class RestoreIOTaskTest : public testing::Test {
protected:
    void SetUp()
    {
        AppProtect::RestoreJob vmRestoreParam;
        std::shared_ptr<ThriftDataBase> restorePtr = std::make_shared<AppProtect::RestoreJob>(vmRestoreParam);
        std::shared_ptr<JobCommonInfo> jobInfo = std::make_shared<JobCommonInfo>(restorePtr);
        m_jobHandle = std::make_shared<JobHandle>(JobType::RESTORE, jobInfo);
    }
    void TearDown() {}

    std::shared_ptr<JobHandle> m_jobHandle = nullptr;
    VolInfo m_volInfo;

    int m_bufSize = 10;
};


/**
 * 用例名称：执行iotask子任务成功
 * 前置条件：repo不为空
 * check点：返回SUCCESS
 */
TEST_F(RestoreIOTaskTest, ExecRestoreIOTaskSuccess)
{
    // mock repoHandler
    std::shared_ptr<FileSystemHandlerMock> repoHandlerMock = std::make_shared<FileSystemHandlerMock>();
    EXPECT_CALL(*repoHandlerMock, Read(An<std::shared_ptr<uint8_t[]>>(),_)).WillRepeatedly(Return(m_bufSize));
    EXPECT_CALL(*repoHandlerMock, Seek(_,_)).WillRepeatedly(Return(SUCCESS));
    // mock volhandler
    std::shared_ptr<VolumeHandlerMock> volHandlerMock = std::make_shared<VolumeHandlerMock>(m_jobHandle, m_volInfo);
    EXPECT_CALL(*volHandlerMock, WriteBlocks(_,_,_)).WillRepeatedly(Return(SUCCESS));
    std::shared_ptr<unsigned char[]> buffer(
            new unsigned char[m_bufSize], std::default_delete<unsigned char[]>());
    RestoreIOTask ioTask(0, m_bufSize, volHandlerMock, repoHandlerMock);
    ioTask.SetIgnoreBadBlockFlag(false);
    ioTask.SetBuffer(buffer);
    int ret = ioTask.Exec();
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * 用例名称：执行iotask子任务失败
 * 前置条件：volHandler为null
 * check点：返回FAILED
 */
TEST_F(RestoreIOTaskTest, ExecRestoreIOTaskFailed_when_volHandlerIsNull)
{
    // mock repoHandler
    std::shared_ptr<FileSystemHandlerMock> repoHandlerMock = std::make_shared<FileSystemHandlerMock>();
    EXPECT_CALL(*repoHandlerMock, Read(An<std::shared_ptr<uint8_t[]>>(),_)).WillRepeatedly(Return(m_bufSize));
    // mock volhandler
    std::shared_ptr<VolumeHandlerMock> volHandlerMock = nullptr;
    std::shared_ptr<unsigned char[]> buffer(
            new unsigned char[m_bufSize], std::default_delete<unsigned char[]>());
    RestoreIOTask ioTask(0, m_bufSize, volHandlerMock, repoHandlerMock);
    ioTask.SetIgnoreBadBlockFlag(false);
    ioTask.SetBuffer(buffer);
    int ret = ioTask.Exec();
    EXPECT_EQ(ret, FAILED);
}


/**
 * 用例名称：执行iotask子任务失败
 * 前置条件：repoHandler为null
 * check点：返回FAILED
 */
TEST_F(RestoreIOTaskTest, ExecRestoreIOTaskFailed_when_repoHandlerIsNull)
{
    // mock repoHandler
    std::shared_ptr<FileSystemHandlerMock> repoHandlerMock = nullptr;
    // mock volhandler
    std::shared_ptr<VolumeHandlerMock> volHandlerMock = std::make_shared<VolumeHandlerMock>(m_jobHandle, m_volInfo);
    EXPECT_CALL(*volHandlerMock, WriteBlocks(_,_,_)).WillRepeatedly(Return(SUCCESS));
    std::shared_ptr<unsigned char[]> buffer(
            new unsigned char[m_bufSize], std::default_delete<unsigned char[]>());
    RestoreIOTask ioTask(0, m_bufSize, volHandlerMock, repoHandlerMock);
    ioTask.SetIgnoreBadBlockFlag(false);
    ioTask.SetBuffer(buffer);
    int ret = ioTask.Exec();
    EXPECT_EQ(ret, FAILED);
}

/**
 * 用例名称：执行iotask子任务失败
 * 前置条件：bufsizse为null
 * check点：返回FAILED
 */
TEST_F(RestoreIOTaskTest, ExecRestoreIOTaskFailed_when_bufferIsNull)
{
    // mock repoHandler
    std::shared_ptr<FileSystemHandlerMock> repoHandlerMock = std::make_shared<FileSystemHandlerMock>();
    EXPECT_CALL(*repoHandlerMock, Read(An<std::shared_ptr<uint8_t[]>>(),_)).WillRepeatedly(Return(FAILED));
    // mock volhandler
    std::shared_ptr<VolumeHandlerMock> volHandlerMock = std::make_shared<VolumeHandlerMock>(m_jobHandle, m_volInfo);
    EXPECT_CALL(*volHandlerMock, WriteBlocks(_,_,_)).WillRepeatedly(Return(SUCCESS));
    std::shared_ptr<unsigned char[]> buffer = nullptr;
    RestoreIOTask ioTask(0, m_bufSize, volHandlerMock, repoHandlerMock);
    ioTask.SetIgnoreBadBlockFlag(false);
    ioTask.SetBuffer(buffer);
    int ret = ioTask.Exec();
    EXPECT_EQ(ret, FAILED);
}

/**
 * 用例名称：执行iotask子任务失败
 * 前置条件：读取数据失败
 * check点：返回FAILED
 */
TEST_F(RestoreIOTaskTest, ExecRestoreIOTaskFailed_when_ReadFailed)
{
    // mock repoHandler
    std::shared_ptr<FileSystemHandlerMock> repoHandlerMock = std::make_shared<FileSystemHandlerMock>();
    EXPECT_CALL(*repoHandlerMock, Read(An<std::shared_ptr<uint8_t[]>>(),_)).WillRepeatedly(Return(FAILED));
    // mock volhandler
    std::shared_ptr<VolumeHandlerMock> volHandlerMock = std::make_shared<VolumeHandlerMock>(m_jobHandle, m_volInfo);
    EXPECT_CALL(*volHandlerMock, WriteBlocks(_,_,_)).WillRepeatedly(Return(SUCCESS));
    std::shared_ptr<unsigned char[]> buffer = nullptr;
    RestoreIOTask ioTask(0, m_bufSize, volHandlerMock, repoHandlerMock);
    ioTask.SetIgnoreBadBlockFlag(false);
    ioTask.SetBuffer(buffer);
    int ret = ioTask.Exec();
    EXPECT_EQ(ret, FAILED);
}

/**
 * 用例名称：执行iotask子任务失败
 * 前置条件：WriteBlocks写入数据块失败
 * check点：返回FAILED
 */
TEST_F(RestoreIOTaskTest, ExecRestoreIOTaskFailed_when_WriteBlockFailed)
{
    // mock repoHandler
    std::shared_ptr<FileSystemHandlerMock> repoHandlerMock = std::make_shared<FileSystemHandlerMock>();
    EXPECT_CALL(*repoHandlerMock, Read(An<std::shared_ptr<uint8_t[]>>(),_)).WillRepeatedly(Return(m_bufSize));
    EXPECT_CALL(*repoHandlerMock, Seek(_,_)).WillRepeatedly(Return(SUCCESS));
    std::shared_ptr<VolumeHandlerMock> volHandlerMock = std::make_shared<VolumeHandlerMock>(m_jobHandle, m_volInfo);
    EXPECT_CALL(*volHandlerMock, WriteBlocks(_,_,_)).WillRepeatedly(Return(FAILED));
    uint32_t blockSize = 4194304;  // Byte
    std::shared_ptr<unsigned char[]> buffer(
            new unsigned char[blockSize], std::default_delete<unsigned char[]>());
    memset_s(buffer.get(), blockSize, 0, blockSize);
    RestoreIOTask ioTask(0, m_bufSize, volHandlerMock, repoHandlerMock);
    ioTask.SetIgnoreBadBlockFlag(false);
    ioTask.SetBuffer(buffer);
    int ret = ioTask.Exec();
    EXPECT_EQ(ret, FAILED);
}

/**
 * 用例名称：忽略坏块恢复成功
 * 前置条件：ReadBlocks读数据块失败，忽略失败，IOTask返回成功
 * check点：返回SUCCESS
 */
TEST_F(RestoreIOTaskTest, ExecRestoreIOTaskSuccess_IgnoreReadBlockFailed)
{
    std::shared_ptr<FileSystemHandlerMock> repoHandlerMock = std::make_shared<FileSystemHandlerMock>();
    EXPECT_CALL(*repoHandlerMock, Read(An<std::shared_ptr<uint8_t[]>>(),_)).WillRepeatedly(Return(m_bufSize - 1));
    EXPECT_CALL(*repoHandlerMock, Seek(_,_)).WillRepeatedly(Return(SUCCESS));
    std::shared_ptr<VolumeHandlerMock> volHandlerMock = std::make_shared<VolumeHandlerMock>(m_jobHandle, m_volInfo);
    uint32_t blockSize = 4194304;  // Byte
    std::shared_ptr<unsigned char[]> buffer(
            new unsigned char[blockSize], std::default_delete<unsigned char[]>());
    memset_s(buffer.get(), blockSize, 0, blockSize);
    RestoreIOTask ioTask(0, m_bufSize, volHandlerMock, repoHandlerMock);
    ioTask.SetIgnoreBadBlockFlag(true);
    ioTask.SetBuffer(buffer);
    int ret = ioTask.Exec();
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(ioTask.Result(), SUCCESS);
}
}
