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
#include <vector>
#include <string>
#include <system/System.hpp>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "stub.h"
#include "common/Macros.h"
#include "volume_handlers/common/DiskDeviceFile.h"

using ::testing::_;
using ::testing::AtLeast;
using testing::DoAll;
using testing::InitGoogleMock;
using ::testing::Return;
using ::testing::Sequence;
using ::testing::SetArgReferee;
using ::testing::Throw;

using namespace VirtPlugin;

namespace HDT_TEST {
const std::string g_fileContent = "FILE CONTENT IN THE TEST FILE.";
const std::string g_fileToOpen = "/tmp/volume_test_files/volume_test.txt";
const uint64_t volumeSizeInBytes = 32;
uint64_t offsetInBytes = 0;
uint64_t bufferSizeInBytes = 32;
static void PrepareFileTest();
static void CleanFileTest();

class DiskDeviceFileTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
};

void DiskDeviceFileTest::SetUp()
{
    PrepareFileTest();
}

void DiskDeviceFileTest::TearDown()
{
    CleanFileTest();
}

void DiskDeviceFileTest::SetUpTestCase()
{}

void DiskDeviceFileTest::TearDownTestCase()
{}

static int32_t ExecStubSuccess()
{
    return Module::SUCCESS;
}

static void PrepareFileTest()
{
    std::string cmd2CreateTestDir = "mkdir -p /tmp/volume_test_files/";
    system(cmd2CreateTestDir.c_str());

    std::string g_fileToOpen = "/tmp/volume_test_files/volume_test.txt";
    std::string cmd2CreateFile = "echo -n " + g_fileContent + ">" + g_fileToOpen;
    system(cmd2CreateFile.c_str());
}

static void CleanFileTest()
{
    std::string cmd2RemoveTestDir = "rm -rf /tmp/volume_test_files/";
    system(cmd2RemoveTestDir.c_str());
}

/*
 * 测试用例： 打开和关闭文件
 * 前置条件： 文件存在
 * CHECK点： 打开文件成功；关闭文件成功
 */
TEST_F(DiskDeviceFileTest, OpenAndCloseTest)
{
    DiskDeviceFile diskDeviceFile;
    EXPECT_EQ(diskDeviceFile.Open(g_fileToOpen, O_WRONLY, volumeSizeInBytes), DISK_DEVICE_OK);
    EXPECT_EQ(diskDeviceFile.Close(), DISK_DEVICE_OK);
}

/*
 * 测试用例： 打开文件失败
 * 前置条件： 文件不存在
 * CHECK点： 打开文件返回非SUCCESS
 */
TEST_F(DiskDeviceFileTest, Open_Failed)
{
    DiskDeviceFile diskDeviceFile;
    EXPECT_NE(
        diskDeviceFile.Open("/tmp/volume_test_files/not_exist_file.txt", O_WRONLY, volumeSizeInBytes), DISK_DEVICE_OK);
}

/*
 * 测试用例： 从文件读数据
 * 前置条件： 打开文件成功
 * CHECK点： 读取数据长度符合预期；读取到的数据内容符合预期；
 */
TEST_F(DiskDeviceFileTest, Read)
{
    Stub stub;
    DiskDeviceFile diskDeviceFile;
    EXPECT_EQ(diskDeviceFile.Open(g_fileToOpen, O_WRONLY, volumeSizeInBytes), DISK_DEVICE_OK);

    const size_t sizeToRead = 32;
    std::shared_ptr<uint8_t[]> buf = std::make_unique<uint8_t[]>(sizeToRead);
    stub.set(ADDR(DiskDeviceFile, DoRead), ExecStubSuccess);
    EXPECT_EQ(diskDeviceFile.Read(offsetInBytes, bufferSizeInBytes, buf), DISK_DEVICE_OK);

    EXPECT_EQ(diskDeviceFile.Close(), DISK_DEVICE_OK);
}
}  // namespace HDT_TEST