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
#include <list>
#include <cstdio>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "log/Log.h"
#include "stub.h"
#include "ArchiveServiceTask.h"

using ::testing::_;
using ::testing::AtLeast;
using testing::DoAll;
using testing::InitGoogleMock;
using ::testing::Return;
using ::testing::Sequence;
using ::testing::SetArgReferee;
using ::testing::Throw;

namespace {
const int SUCCESS = 0;
const int FAILED = -1;

class ArchiveClientTest : public ArchiveClientBase {
public:
    ArchiveClientTest() {}
    ~ArchiveClientTest() {}

    virtual int GetFileData(const ArchiveRequest& req, ArchiveResponse& rsp) override { return SUCCESS; }
};

}
class ArchiveServiceTaskTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();

    ArchiveServiceParams m_params;
    FileHandle m_fileHandle;
    std::shared_ptr<BlockBufferMap> m_blockBufferMap;
    std::shared_ptr<ArchiveClientBase> m_archiveClient = std::make_shared<ArchiveClientTest>();
};

void ArchiveServiceTaskTest::SetUp()
{
    m_params.srcRootPath = "m_srcAdvParams->rootPath";
    m_params.dstRootPath = "m_dstAdvParams->rootPath";
    m_params.backupDataFormat = BackupDataFormat::UNKNOWN_FORMAT;
    m_params.restoreReplacePolicy = RestoreReplacePolicy::NONE;
    m_params.backupType = BackupType::UNKNOWN_TYPE;

    m_fileHandle.m_file = std::make_shared<FileDesc>(BackupIOEngine::POSIX, BackupIOEngine::ARCHIVE_CLIENT);

    m_blockBufferMap = std::make_shared<BlockBufferMap>();
}

void ArchiveServiceTaskTest::TearDown()
{}

void ArchiveServiceTaskTest::SetUpTestCase()
{}

void ArchiveServiceTaskTest::TearDownTestCase()
{}

/*
* 用例名称：测试执行入口
* 前置条件：未知类型
* check点：未知类型
*/
TEST_F(ArchiveServiceTaskTest, Exec)
{
    auto task = std::make_shared<ArchiveServiceTask>(
        ArchiveEvent::OPEN_SRC, m_blockBufferMap, m_fileHandle, m_params, m_archiveClient);
    task->m_bufferMapPtr = nullptr;
    EXPECT_NO_THROW(task->Exec());

    task->m_event = ArchiveEvent::INVALID_EVENT;
    EXPECT_NO_THROW(task->Exec());
}

/*
* 用例名称：打开文件服务
* 前置条件：无
* check点：打开文件成功
*/
TEST_F(ArchiveServiceTaskTest, HandleOpenFile)
{
    auto task = std::make_shared<ArchiveServiceTask>(
        ArchiveEvent::OPEN_SRC, m_blockBufferMap, m_fileHandle, m_params, m_archiveClient);
    task->Exec();

    EXPECT_EQ(task->m_result, SUCCESS);
}

/*
* 用例名称：关闭文件服务
* 前置条件：无
* check点：关闭文件成功
*/
TEST_F(ArchiveServiceTaskTest, HandleCloseSrc)
{
    auto task = std::make_shared<ArchiveServiceTask>(
        ArchiveEvent::CLOSE_SRC, m_blockBufferMap, m_fileHandle, m_params, m_archiveClient);
    task->Exec();

    EXPECT_EQ(task->m_result, SUCCESS);
}

/*
* 用例名称：打开文件服务
* 前置条件：空文件/非空文件
* check点：打开文件成功
*/
TEST_F(ArchiveServiceTaskTest, HandleReadData)
{
    m_fileHandle.m_block.m_size = 0;
    auto task = std::make_shared<ArchiveServiceTask>(
        ArchiveEvent::READ_DATA, m_blockBufferMap, m_fileHandle, m_params, m_archiveClient);
    task->Exec();
    EXPECT_EQ(task->m_result, SUCCESS);

    m_fileHandle.m_block.m_size = 1024;
    task = std::make_shared<ArchiveServiceTask>(
        ArchiveEvent::READ_DATA, m_blockBufferMap, m_fileHandle, m_params, m_archiveClient);
    task->Exec();
    EXPECT_EQ(task->m_result, SUCCESS);
}

/*
* 用例名称：处理软连接
* 前置条件：读取软连接成功
* check点：处理软链接成功
*/
TEST_F(ArchiveServiceTaskTest, ProcessReadSoftLinkData)
{
    m_fileHandle.m_block.m_buffer = new uint8_t[1024];
    auto task = std::make_shared<ArchiveServiceTask>(
        ArchiveEvent::READ_DATA, m_blockBufferMap, m_fileHandle, m_params, m_archiveClient);
    int ret = task->ProcessReadSoftLinkData();
    EXPECT_EQ(ret, Module::SUCCESS);
}

/*
* 用例名称：处理特殊文件
* 前置条件：
* check点：处理软链接成功
*/
TEST_F(ArchiveServiceTaskTest, ProcessReadSpecialFileData)
{
    auto task = std::make_shared<ArchiveServiceTask>(
        ArchiveEvent::READ_DATA, m_blockBufferMap, m_fileHandle, m_params, m_archiveClient);
    int ret = task->ProcessReadSpecialFileData();
    EXPECT_EQ(ret, Module::SUCCESS);
}

/*
* 用例名称：测试删除文件名前缀 、
* 前置条件：文件名带有多个 ///
* check点：去除文件名前缀的 ///
*/
TEST_F(ArchiveServiceTaskTest, CutPrefixSlash)
{
    auto task = std::make_shared<ArchiveServiceTask>(
        ArchiveEvent::READ_DATA, m_blockBufferMap, m_fileHandle, m_params, m_archiveClient);
    std::string input = "///123.txt";
    std::string output = task->CutPrefixSlash(input);
    EXPECT_EQ(output, "123.txt");

    input = "///dir/123";
    output = task->CutPrefixSlash(input);
    EXPECT_EQ(output, "dir/123");

    input = "./dir/123";
    output = task->CutPrefixSlash(input);
    EXPECT_EQ(output, "dir/123");

    input = ".dir/123";
    output = task->CutPrefixSlash(input);
    EXPECT_EQ(output, "dir/123");

    input = ".//dir/123";
    output = task->CutPrefixSlash(input);
    EXPECT_EQ(output, "dir/123");
}