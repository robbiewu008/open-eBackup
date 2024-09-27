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
#include "stub.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "gmock/gmock-actions.h"
#include "snapshot_provider/DeviceMount.h"
#include <memory>

using ::testing::_;
using ::testing::Invoke;
using ::testing::AtLeast;
using testing::DoAll;
using testing::InitGoogleMock;
using ::testing::Return;
using ::testing::Sequence;
using ::testing::SetArgReferee;
using ::testing::Throw;
using ::testing::Mock;
using namespace FilePlugin;

namespace {
    const uint64_t COMMANDJOBTYPE_TEST = 999;
}

class DeviceMountTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
};


void DeviceMountTest::SetUp()
{
}

void DeviceMountTest::TearDown()
{
}

void DeviceMountTest::SetUpTestCase()
{
}

void DeviceMountTest::TearDownTestCase()
{
}

/*
* 用例名称：加载设备列表
* 前置条件：初始化当前系统所有的设备信息
* check点：加载设备列表成功
*/
TEST_F(DeviceMountTest, LoadDevice_Success)
{
    std::shared_ptr<DeviceMount> deviceMountPtr = std::make_shared<DeviceMount>();
    bool result = deviceMountPtr->LoadDevice();
    EXPECT_EQ(result, true);
    result = deviceMountPtr->LoadDevice();
    EXPECT_EQ(result, true);
}

/*
* 用例名称：根据文件获取文件系统类型
* 前置条件：初始化当前系统所有的设备信息
* check点：获取到的文件系统类型不为空
*/
TEST_F(DeviceMountTest, GetFsType_Success)
{
    std::shared_ptr<DeviceMount> deviceMountPtr = std::make_shared<DeviceMount>();
    deviceMountPtr->LoadDevice();
    std::string fsType = deviceMountPtr->GetFsType("/home");
    bool result = !fsType.empty();
    EXPECT_EQ(result, true);
}

/*
* 用例名称：获取指定文件的设备信息
* 前置条件：无
* check点：获取到对应的设备类型
*/
TEST_F(DeviceMountTest, GetFsDevice_Success)
{
    std::shared_ptr<DeviceMount> deviceMountPtr = std::make_shared<DeviceMount>();
    deviceMountPtr->LoadDevice();
    std::shared_ptr<FsDevice> fsDevice = deviceMountPtr->FindDevice("/home");
    bool result = fsDevice != nullptr;
    EXPECT_EQ(result, true);
}

/*
* 用例名称：获取不存在的文件设备类型
* 前置条件：无
* check点：获取不到对应的设备类型
*/
TEST_F(DeviceMountTest, GetFsDevice_Not_Exist)
{
    std::shared_ptr<DeviceMount> deviceMountPtr = std::make_shared<DeviceMount>();
    deviceMountPtr->LoadDevice();
    std::shared_ptr<FsDevice> fsDevice = deviceMountPtr->FindDevice("/home/not");
    bool result = fsDevice == nullptr;
    EXPECT_EQ(result, true);
}

/*
* 用例名称：获取子卷内容
* 前置条件：无
* check点：成功获取到子卷信息
*/
TEST_F(DeviceMountTest, GetSubVolumes_Success)
{
    std::shared_ptr<DeviceMount> deviceMountPtr = std::make_shared<DeviceMount>();
    deviceMountPtr->LoadDevice();
    std::vector<std::shared_ptr<FsDevice>> outputEntryList;
    bool result = deviceMountPtr->GetSubVolumes("/", outputEntryList);
    EXPECT_EQ(result, true);
}

/*
* 用例名称：检查路径是否是挂载点
* 前置条件：无
* check点：是挂载点
*/
TEST_F(DeviceMountTest, CheckWhetherMountPoint_True)
{
    std::shared_ptr<DeviceMount> deviceMountPtr = std::make_shared<DeviceMount>();
    deviceMountPtr->LoadDevice();
    std::string path = "/";
    bool result = deviceMountPtr->CheckWhetherMountPoint(path);
    EXPECT_EQ(result, true);
}

/*
* 用例名称：检查路径是否是挂载点
* 前置条件：无
* check点：不是挂载点
*/
TEST_F(DeviceMountTest, CheckWhetherMountPoint_False)
{
    std::shared_ptr<DeviceMount> deviceMountPtr = std::make_shared<DeviceMount>();
    deviceMountPtr->LoadDevice();
    std::string path = "/root";
    bool result = deviceMountPtr->CheckWhetherMountPoint(path);
    EXPECT_EQ(result, false);
}