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
#include <iostream>
#include <list>
#include <cstdio>
#include "gtest/gtest.h"
#include "json/json.h"
#include "gmock/gmock.h"
#include "stub.h"
#include "common/Constants.h"
#include "common/DirtyRanges.h"
#include "repository_handlers/RepositoryHandler.h"
#include "repository_handlers/filesystem/FileSystemHandler.h"

using namespace VirtPlugin;
namespace HDT_TEST {
const std::string DIRTY_RANGES_FILE_DIR = "/tmp/DirtyRangesTest";
const std::string TASK_ID = "id_123";
const uint64_t RANGE_SIZE = 100;
const uint64_t RANGE_NUM = 2;

void PrepareRangesFileTest(std::string path);
void CleanRangesFileTest(std::string path);

class DirtyRangesTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();

public:
    uint64_t m_rangeStart1;
    uint64_t m_rangeSize1;
    uint64_t m_rangeStart2;
    uint64_t m_rangeSize2;
    std::string m_serialize;
    std::string m_dirtyRangesFileRootPath;
    std::string m_taskID;
    std::shared_ptr<RepositoryHandler> m_rangeFileHandler;
};

void DirtyRangesTest::SetUp()
{
    m_rangeStart1 = 0;
    m_rangeSize1 = RANGE_SIZE;
    m_rangeStart2 = DIRTY_RANGE_BLOCK_SIZE * RANGE_NUM;
    m_rangeSize2 = RANGE_SIZE;
    m_serialize = "{\"DirtyRanges\":{\"m_isForDedupe\":false,\"m_isFull\":false,\"m_isRestore\":false,"
        "\"m_ranges\":\"" + std::to_string(m_rangeStart1) + ":" + std::to_string(DIRTY_RANGE_BLOCK_SIZE) + ","
        + std::to_string(m_rangeStart2) + ":" + std::to_string(DIRTY_RANGE_BLOCK_SIZE) + "\"}}\n";
    m_dirtyRangesFileRootPath = DIRTY_RANGES_FILE_DIR;
    m_taskID = TASK_ID;
}

void DirtyRangesTest::TearDown() {}

void DirtyRangesTest::SetUpTestCase()
{
    PrepareRangesFileTest(DIRTY_RANGES_FILE_DIR);
}

void DirtyRangesTest::TearDownTestCase()
{
    CleanRangesFileTest(DIRTY_RANGES_FILE_DIR);
}

void PrepareRangesFileTest(std::string path)
{
    std::string cmd2CreateTestDir = "mkdir -p " + path;
    system(cmd2CreateTestDir.c_str());
}

void CleanRangesFileTest(std::string path)
{
    std::string cmd2RemoveTestDir = "rm -rf " + path;
    system(cmd2RemoveTestDir.c_str());
}

void ModifyRangesFileTest(std::string path, std::string taskID)
{
    std::string rangefile = path + "/" + taskID + ".dirtyRanges";
    std::string cmd2CreateFile = "echo -n FILE CONTENT IN THE TEST FILE. > " + rangefile;
    system(cmd2CreateFile.c_str());
}

/*
 * 用例名称：添加多个Range到DirtyRange中
 * 前置条件：无
 * check点：多个Range位于同一个块区间，DirtyRange不新增
 */
TEST_F(DirtyRangesTest, AddRangeFromDiffBlock)
{
    DirtyRanges dirtyRanges;
    bool ret = dirtyRanges.AddRange(DirtyRange(m_rangeStart1, m_rangeSize1));
    EXPECT_EQ(true, ret);
    m_rangeStart2 = m_rangeStart1 + m_rangeSize1;
    ret = dirtyRanges.AddRange(DirtyRange(m_rangeStart2, m_rangeSize2));
    EXPECT_EQ(true, ret);
    std::shared_ptr<std::list<DirtyRange>> listRanges = dirtyRanges.GetRanges();
    int size = listRanges->size();
    EXPECT_EQ(1, size);
}

/*
 * 用例名称：添加多个Range到DirtyRange中
 * 前置条件：无
 * check点：多个Range位于不同的块区间，DirtyRange新增
 */
TEST_F(DirtyRangesTest, AddRangeFromSameBlock)
{
    DirtyRanges dirtyRanges;
    bool ret = dirtyRanges.AddRange(DirtyRange(m_rangeStart1, m_rangeSize1));
    EXPECT_EQ(true, ret);
    ret = dirtyRanges.AddRange(DirtyRange(m_rangeStart2, m_rangeSize2));
    EXPECT_EQ(true, ret);
    std::shared_ptr<std::list<DirtyRange>> listRanges = dirtyRanges.GetRanges();
    int size = listRanges->size();
    EXPECT_EQ(RANGE_NUM, size);
}

/*
 * 用例名称：添加多个Range到DirtyRange中
 * 前置条件：无
 * check点：后续Range未按顺序排列，添加失败
 */
TEST_F(DirtyRangesTest, AddRangeFailedWhenRangeNoOrder)
{
    DirtyRanges dirtyRanges;
    bool ret = dirtyRanges.AddRange(DirtyRange(m_rangeStart2, m_rangeSize2));
    EXPECT_EQ(true, ret);
    ret = dirtyRanges.AddRange(DirtyRange(m_rangeStart1, m_rangeSize1));
    EXPECT_EQ(false, ret);
    std::shared_ptr<std::list<DirtyRange>> listRanges = dirtyRanges.GetRanges();
    int size = listRanges->size();
    EXPECT_EQ(0, size);
}

/*
 * 用例名称：DirtyRange序列化测试
 * 前置条件：无
 * check点：序列化字符串是否和预期一致
 */
TEST_F(DirtyRangesTest, Serialize)
{
    DirtyRanges dirtyRanges;
    dirtyRanges.AddRange(DirtyRange(m_rangeStart1, m_rangeSize1));
    dirtyRanges.AddRange(DirtyRange(m_rangeStart2, m_rangeSize2));
    std::string serialize = dirtyRanges.Serialize();
    EXPECT_EQ(m_serialize, serialize);
}

/*
 * 用例名称：DirtyRange反序列化测试
 * 前置条件：无
 * check点：输入字符串正确，解析结果返回成功
 */
TEST_F(DirtyRangesTest, DeserializeSuccess)
{
    DirtyRanges dirtyRanges;
    dirtyRanges.clear();
    Json::Value val;
    val["DirtyRanges"]["m_isFull"] = false;
    val["DirtyRanges"]["m_isRestore"] = false;
    val["DirtyRanges"]["m_isForDedupe"] = false;
    val["DirtyRanges"]["m_ranges"] = std::to_string(m_rangeStart1) + ":" + std::to_string(DIRTY_RANGE_BLOCK_SIZE) + ","
        + std::to_string(m_rangeStart2) + ":" + std::to_string(DIRTY_RANGE_BLOCK_SIZE);
    bool ret = dirtyRanges.Deserialize(val);
    EXPECT_EQ(true, ret);
    std::shared_ptr<std::list<DirtyRange>> listRanges = dirtyRanges.GetRanges();
    int size = listRanges->size();
    EXPECT_EQ(RANGE_NUM, size);
    std::list<DirtyRange>::const_iterator it = listRanges->begin();
    EXPECT_EQ(it->start, m_rangeStart1);
    EXPECT_EQ(it->size, DIRTY_RANGE_BLOCK_SIZE);
    ++it;
    EXPECT_EQ(it->start, m_rangeStart2);
    EXPECT_EQ(it->size, DIRTY_RANGE_BLOCK_SIZE);
}

/*
 * 用例名称：DirtyRange反序列化测试
 * 前置条件：无
 * check点：输入字符串格式错误，解析结果返回失败
 */
TEST_F(DirtyRangesTest, DeserializeFailed)
{
    DirtyRanges dirtyRanges;
    dirtyRanges.clear();
    Json::Value val;
    val["DirtyRanges"]["m_isFull"] = false;
    val["DirtyRanges"]["m_isRestore"] = false;
    val["DirtyRanges"]["m_isForDedupe"] = false;
    val["DirtyRanges"]["m_ranges"] = "error_string";
    bool ret = dirtyRanges.Deserialize(val);
    EXPECT_EQ(false, ret);
}

/*
 * 用例名称：DirtyRange反序列化测试
 * 前置条件：无
 * check点：输入空字符串，解析结果返回成功
 */
TEST_F(DirtyRangesTest, DeserializeSuccessWhenNullInput)
{
    DirtyRanges dirtyRanges;
    dirtyRanges.clear();
    Json::Value val;
    val["DirtyRanges"]["m_isFull"] = false;
    val["DirtyRanges"]["m_isRestore"] = false;
    val["DirtyRanges"]["m_isForDedupe"] = false;
    bool ret = dirtyRanges.Deserialize(val);
    EXPECT_EQ(true, ret);
}

/*
 * 用例名称：将DirtyRange保存到文件中
 * 前置条件：无
 * check点：写入成功
 */
TEST_F(DirtyRangesTest, FlushToStorageSuccess)
{
    DirtyRanges dirtyRanges;
    dirtyRanges.AddRange(DirtyRange(m_rangeStart1, m_rangeSize1));
    dirtyRanges.AddRange(DirtyRange(m_rangeStart2, m_rangeSize2));
    m_rangeFileHandler = std::make_shared<FileSystemHandler>();
    dirtyRanges.Initialize(m_dirtyRangesFileRootPath, m_taskID, m_rangeFileHandler);
    bool ret = dirtyRanges.FlushToStorage();
    EXPECT_EQ(true, ret);
}

/*
 * 用例名称：从文件中加载DirtyRange
 * 前置条件：文件存在，内容正确
 * check点：加载成功
 */
TEST_F(DirtyRangesTest, LoadFromStorageSuccess)
{
    DirtyRanges dirtyRanges;
    m_rangeFileHandler = std::make_shared<FileSystemHandler>();
    dirtyRanges.Initialize(m_dirtyRangesFileRootPath, m_taskID, m_rangeFileHandler);
    bool ret = dirtyRanges.LoadFromStorage();
    EXPECT_EQ(true, ret);
    uint64_t blockNum = dirtyRanges.GetBlockNum(DIRTY_RANGE_BLOCK_SIZE);
    EXPECT_EQ(RANGE_NUM, blockNum);
    DirtyRanges::iterator it = dirtyRanges.begin(DIRTY_RANGE_BLOCK_SIZE);
    EXPECT_EQ(it->Offset(), m_rangeStart1);
    EXPECT_EQ(it->size, DIRTY_RANGE_BLOCK_SIZE);
    ++it;
    EXPECT_EQ(it->Offset(), m_rangeStart2);
    EXPECT_EQ(it->size, DIRTY_RANGE_BLOCK_SIZE);
}

/*
 * 用例名称：从文件中加载DirtyRange
 * 前置条件：文件存在，内容格式不正确
 * check点：格式解析错误，加载失败
 */
TEST_F(DirtyRangesTest, LoadFromStorageFailedWhenFileContentErr)
{
    ModifyRangesFileTest(DIRTY_RANGES_FILE_DIR, TASK_ID);
    DirtyRanges dirtyRanges;
    m_rangeFileHandler = std::make_shared<FileSystemHandler>();
    dirtyRanges.Initialize(m_dirtyRangesFileRootPath, m_taskID, m_rangeFileHandler);
    bool ret = dirtyRanges.LoadFromStorage();
    EXPECT_EQ(false, ret);
}

/*
 * 用例名称：从文件中加载DirtyRange
 * 前置条件：文件不存在
 * check点：文件不存在，加载失败
 */
TEST_F(DirtyRangesTest, LoadFromStorageFailedWhenFileNoExist)
{
    CleanRangesFileTest(DIRTY_RANGES_FILE_DIR);
    DirtyRanges dirtyRanges;
    m_rangeFileHandler = std::make_shared<FileSystemHandler>();
    dirtyRanges.Initialize(m_dirtyRangesFileRootPath, m_taskID, m_rangeFileHandler);
    bool ret = dirtyRanges.LoadFromStorage();
    EXPECT_EQ(false, ret);
}

/*
 * 用例名称：清理DirtyRange文件
 * 前置条件：文件存在
 * check点：清理成功
 */
TEST_F(DirtyRangesTest, CleanDirtyRangesFileSuccess)
{
    PrepareRangesFileTest(DIRTY_RANGES_FILE_DIR);
    ModifyRangesFileTest(DIRTY_RANGES_FILE_DIR, TASK_ID);
    DirtyRanges dirtyRanges;
    m_rangeFileHandler = std::make_shared<FileSystemHandler>();
    dirtyRanges.Initialize(m_dirtyRangesFileRootPath, m_taskID, m_rangeFileHandler);
    bool ret = dirtyRanges.CleanDirtyRangesFile();
    EXPECT_EQ(true, ret);
}

/*
 * 用例名称：清理DirtyRange文件
 * 前置条件：文件不存在
 * check点：文件不存在，返回成功
 */
TEST_F(DirtyRangesTest, CleanDirtyRangesFileSuccessWhenNoExist)
{
    CleanRangesFileTest(DIRTY_RANGES_FILE_DIR);
    DirtyRanges dirtyRanges;
    m_rangeFileHandler = std::make_shared<FileSystemHandler>();
    dirtyRanges.Initialize(m_dirtyRangesFileRootPath, m_taskID, m_rangeFileHandler);
    bool ret = dirtyRanges.CleanDirtyRangesFile();
    EXPECT_EQ(true, ret);
}
}
