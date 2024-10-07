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
#include <openssl/sha.h>
#include "securec.h"
#include "common/sha256/Sha256.h"
#include "job_controller/jobs/verify/BlockCheckTask.h"
#include "repository_handlers/mock/FileSystemHandlerMock.h"

using ::testing::_;
using ::testing::An;
using ::testing::AtLeast;
using testing::DoAll;
using testing::InitGoogleMock;
using ::testing::Invoke;
using ::testing::Return;
using ::testing::Sequence;
using ::testing::SetArgReferee;
using ::testing::Throw;
;

using namespace VirtPlugin;

namespace HDT_TEST {

class BlockCheckTaskTest : public testing::Test {
protected:
    void SetUp()
    {}
    void TearDown()
    {}
};

/**
 * 用例名称：执行块校验任务成功失败
 * 前置条件：repoHander为空
 * check点：执行失败
 */
TEST_F(BlockCheckTaskTest, ExecBlockCheckTaskFailedWhenRepoNull)
{
    uint64_t bufSize = 256;
    std::shared_ptr<uint8_t[]> readBuf = std::make_unique<uint8_t[]>(bufSize);
    memset_s(readBuf.get(), bufSize, 1, bufSize);
    std::shared_ptr<uint8_t[]> shaBuf = std::make_unique<uint8_t[]>(SHA256_DIGEST_LENGTH);
    CalculateSha256::CalculateSha256Value(readBuf, bufSize, shaBuf);
    std::shared_ptr<FileSystemHandlerMock> repoHandlerMock = nullptr;

    BlockCheckTask checkTask(0, bufSize, shaBuf, repoHandlerMock);
    checkTask.SetBuffer(readBuf);
    int ret = checkTask.Exec();
    EXPECT_EQ(ret, FAILED);
}

/**
 * 用例名称：执行块校验任务成功
 * 前置条件：校验文件操作（Seek、Read）成功
 * check点：SHA256对比成功，返回SUCCESS
 */
TEST_F(BlockCheckTaskTest, ExecBlockCheckTaskSuccess)
{
    uint64_t bufSize = 256;
    std::shared_ptr<uint8_t[]> readBuf = std::make_unique<uint8_t[]>(bufSize);
    memset_s(readBuf.get(), bufSize, 1, bufSize);

    std::shared_ptr<uint8_t[]> shaBuf = std::make_unique<uint8_t[]>(SHA256_DIGEST_LENGTH);
    CalculateSha256::CalculateSha256Value(readBuf, bufSize, shaBuf);

    std::shared_ptr<FileSystemHandlerMock> repoHandlerMock = std::make_shared<FileSystemHandlerMock>();
    EXPECT_CALL(*repoHandlerMock, Read(An<std::shared_ptr<uint8_t[]>>(), _)).WillRepeatedly(Return(bufSize));
    EXPECT_CALL(*repoHandlerMock, Seek(_, _)).WillRepeatedly(Return(0));

    BlockCheckTask checkTask(0, bufSize, shaBuf, repoHandlerMock);
    checkTask.SetBuffer(readBuf);
    int ret = checkTask.Exec();
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * 用例名称：读校验文件失败，执行块校验任务成功失败
 * 前置条件：读校验文件失败（Read返回0）
 * check点：校验结果为FAILED
 */
TEST_F(BlockCheckTaskTest, ExecBlockCheckTaskFailedWhenReadFail)
{
    uint64_t bufSize = 256;
    std::shared_ptr<uint8_t[]> readBuf = std::make_unique<uint8_t[]>(bufSize);
    memset_s(readBuf.get(), bufSize, 1, bufSize);

    std::shared_ptr<FileSystemHandlerMock> repoHandlerMock = std::make_shared<FileSystemHandlerMock>();
    EXPECT_CALL(*repoHandlerMock, Read(An<std::shared_ptr<uint8_t[]>>(), _)).WillRepeatedly(Return(0));
    EXPECT_CALL(*repoHandlerMock, Seek(_, _)).WillRepeatedly(Return(0));

    BlockCheckTask checkTask(0, bufSize, readBuf, repoHandlerMock);
    checkTask.SetBuffer(readBuf);
    int ret = checkTask.Exec();
    EXPECT_EQ(ret, FAILED);
}

/**
 * 用例名称：移动校验文件失败，执行块校验任务成功失败
 * 前置条件：移动校验文件失败（Seek返回-1）
 * check点：校验结果为FAILED
 */
TEST_F(BlockCheckTaskTest, ExecBlockCheckTaskFailedWhenSeekFail)
{
    uint64_t bufSize = 256;
    std::shared_ptr<uint8_t[]> readBuf = std::make_unique<uint8_t[]>(bufSize);
    memset_s(readBuf.get(), bufSize, 1, bufSize);

    std::shared_ptr<FileSystemHandlerMock> repoHandlerMock = std::make_shared<FileSystemHandlerMock>();
    EXPECT_CALL(*repoHandlerMock, Read(An<std::shared_ptr<uint8_t[]>>(), _)).WillRepeatedly(Return(bufSize));
    EXPECT_CALL(*repoHandlerMock, Seek(_, _)).WillRepeatedly(Return(-1));

    BlockCheckTask checkTask(0, bufSize, readBuf, repoHandlerMock);
    checkTask.SetBuffer(readBuf);
    int ret = checkTask.Exec();
    EXPECT_EQ(ret, FAILED);
}

/**
 * 用例名称：SHA256值不匹配，执行块校验任务成功失败
 * 前置条件：SHA256值不匹配
 * check点：校验结果为FAILED
 */
TEST_F(BlockCheckTaskTest, ExecBlockCheckTaskFailedWhenShaMismatchFail)
{
    uint64_t bufSize = 256;
    std::shared_ptr<uint8_t[]> readBuf = std::make_unique<uint8_t[]>(bufSize);
    memset_s(readBuf.get(), bufSize, 1, bufSize);

    std::shared_ptr<uint8_t[]> shaBuf = std::make_unique<uint8_t[]>(SHA256_DIGEST_LENGTH);
    CalculateSha256::CalculateSha256Value(readBuf, bufSize, shaBuf);

    std::shared_ptr<FileSystemHandlerMock> repoHandlerMock = std::make_shared<FileSystemHandlerMock>();
    EXPECT_CALL(*repoHandlerMock, Read(An<std::shared_ptr<uint8_t[]>>(), _)).WillRepeatedly(Return(bufSize));
    EXPECT_CALL(*repoHandlerMock, Seek(_, _)).WillRepeatedly(Return(0));

    BlockCheckTask checkTask(0, bufSize, shaBuf, repoHandlerMock);
    readBuf[0] = 0;
    checkTask.SetBuffer(readBuf);
    int ret = checkTask.Exec();
    EXPECT_EQ(ret, DAMAGED);
}
}  // namespace HDT_TEST
