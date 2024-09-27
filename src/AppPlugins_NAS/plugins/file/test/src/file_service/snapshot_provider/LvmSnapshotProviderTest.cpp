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
#include "snapshot_provider/LvmSnapshotProvider.h"
#include <memory>
#include "system/System.hpp"

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

class LvmSnapshotProviderTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    Stub stub;
};


void LvmSnapshotProviderTest::SetUp()
{}

void LvmSnapshotProviderTest::TearDown()
{
}

void LvmSnapshotProviderTest::SetUpTestCase()
{
}

void LvmSnapshotProviderTest::TearDownTestCase()
{
}

static bool Stub_CreateSnapShot_No_Support(const std::string& cmd,
    const std::vector<std::string>& paramList,
    std::vector<std::string>& shellOutput)
{
    return false;
}

static int Stub_DeleteSnapShot_Support(const std::string& cmd,
    const std::vector<std::string>& paramList,
    std::vector<std::string>& shellOutput)
{
    return Module::SUCCESS;
}

static bool g_firstCall = true;
static int Stub_CreateSnapShot_Support(void* obj, const std::string& cmd,
    const std::vector<std::string>& paramList,
    std::vector<std::string>& shellOutput)
{
    if (g_firstCall) {
        shellOutput.push_back("vg/lv");
        g_firstCall = false;
    } else {
        shellOutput.push_back("vg/lv:/mnt/lvm_snapshot/home");
    }
    return Module::SUCCESS;
}
static bool Stub_GetSubVolumes(void* obj,std::string path,
    std::vector<std::shared_ptr<FsDevice>>& outputEntryList)
{
    std::shared_ptr<FsDevice> fsDevice = std::make_shared<FsDevice>(1,"/home","ext4","/dev/mapper/lv");
    outputEntryList.push_back(fsDevice);
    return true;
}

static int Stub_runShellCmdWithOutput_FAILED(const severity_level& severity, const std::string& moduleName,
        const size_t& requestID, const std::string& cmd, const std::vector<std::string> params,
        std::vector<std::string>& cmdoutput, std::vector<std::string>& stderroutput)
{
    stderroutput.push_back("aaa");
    return Module::FAILED;
}

static int Stub_runShellCmdWithOutput_SUCCESS(const severity_level& severity, const std::string& moduleName,
        const size_t& requestID, const std::string& cmd, const std::vector<std::string> params,
        std::vector<std::string>& cmdoutput, std::vector<std::string>& stderroutput)
{
    stderroutput.push_back("aaa");
    return Module::SUCCESS;
}

/*
* 用例名称：ExecShellCmd
* 前置条件：
* check点：执行shell命令
*/
TEST_F(LvmSnapshotProviderTest, ExecShellCmd)
{
    std::string jobId = "121345";
    std::shared_ptr<DeviceMount> deviceMountPtr = std::make_shared<DeviceMount>();
    std::shared_ptr<LvmSnapshotProvider> lvmSnapshotPtr =
        std::make_shared<LvmSnapshotProvider>(deviceMountPtr,  jobId, "/lvm_snapshots");

    std::string cmd = "cmd";
    std::vector<std::string> paramList = {"aaa"};
    std::vector<std::string> shellOutput = {"bbb"};
    stub.set(Module::runShellCmdWithOutput, Stub_runShellCmdWithOutput_FAILED);
    int ret = lvmSnapshotPtr->ExecShellCmd(cmd, paramList, shellOutput);
    EXPECT_EQ(ret, Module::FAILED);
    stub.reset(Module::runShellCmdWithOutput);

    stub.set(Module::runShellCmdWithOutput, Stub_runShellCmdWithOutput_SUCCESS);
    ret = lvmSnapshotPtr->ExecShellCmd(cmd, paramList, shellOutput);
    EXPECT_EQ(ret, Module::SUCCESS);
    stub.reset(Module::runShellCmdWithOutput);
}

static int Stub_ExecShellCmd_Failed()
{
    return Module::FAILED;
}

/*
* 用例名称：DeleteSnapshotByTag
* 前置条件：
* check点：删除快照
*/
TEST_F(LvmSnapshotProviderTest, DeleteSnapshotByTag)
{

    std::string jobId = "121345";
    std::shared_ptr<DeviceMount> deviceMountPtr = std::make_shared<DeviceMount>();
    std::shared_ptr<LvmSnapshotProvider> lvmSnapshotPtr =
        std::make_shared<LvmSnapshotProvider>(deviceMountPtr,  jobId, "/lvm_snapshots");

    stub.set(ADDR(LvmSnapshotProvider, ExecShellCmd), Stub_ExecShellCmd_Failed);

    std::string snapTag = "aaa";
    bool ret = lvmSnapshotPtr->DeleteSnapshotByTag(snapTag);
    EXPECT_EQ(ret, false);

    ret = lvmSnapshotPtr->DeleteSnapshotByVolume(snapTag);
    EXPECT_EQ(ret, false);
    stub.reset(ADDR(LvmSnapshotProvider, ExecShellCmd));
}

/*
* 用例名称：CreateSnapshotByVolume
* 前置条件：
* check点：创建快照
*/
TEST_F(LvmSnapshotProviderTest, CreateSnapshotByVolume)
{

    std::string jobId = "121345";
    std::shared_ptr<DeviceMount> deviceMountPtr = std::make_shared<DeviceMount>();
    std::shared_ptr<LvmSnapshotProvider> lvmSnapshotPtr =
        std::make_shared<LvmSnapshotProvider>(deviceMountPtr,  jobId, "/lvm_snapshots");

    std::string str = "xxx";
    // std::shared_ptr<LvmSnapshot> ptr {nullptr};
    // lvmSnapshotPtr->deviceVolumeSnapMap[str] = ptr;
    std::string volumeName = "xxx";
    std::string volumePath = "/a/b";
    int ret;
    std::shared_ptr<LvmSnapshot> ptr = lvmSnapshotPtr->CreateSnapshotByVolume(volumeName, volumePath, ret);
    EXPECT_EQ(ptr, nullptr);

    stub.set(ADDR(LvmSnapshotProvider, ExecShellCmd), Stub_ExecShellCmd_Failed);
    ptr = lvmSnapshotPtr->CreateSnapshotByVolume(volumeName, volumePath, ret);
    EXPECT_EQ(ptr, nullptr);
    stub.reset(ADDR(LvmSnapshotProvider, ExecShellCmd));
}

/*
* 用例名称：GetLogicalVolume
* 前置条件：
* check点：获得逻辑卷
*/
TEST_F(LvmSnapshotProviderTest, GetLogicalVolume)
{

    std::string jobId = "121345";
    std::shared_ptr<DeviceMount> deviceMountPtr = std::make_shared<DeviceMount>();
    std::shared_ptr<LvmSnapshotProvider> lvmSnapshotPtr =
        std::make_shared<LvmSnapshotProvider>(deviceMountPtr,  jobId, "/lvm_snapshots");

    stub.set(ADDR(LvmSnapshotProvider, ExecShellCmd), Stub_ExecShellCmd_Failed);
    std::string path = "/a/b";
    bool isCrossVolume = false;
    std::vector<LvmSnapshot> volumeInfo;
    bool ret = lvmSnapshotPtr->GetLogicalVolume(path, isCrossVolume, volumeInfo);
    EXPECT_EQ(ret, false);
    stub.reset(ADDR(LvmSnapshotProvider, ExecShellCmd));
}

/*
* 用例名称：创建快照
* 前置条件：初始化当前系统所有的设备信息
* check点：创建快照成功
*/
TEST_F(LvmSnapshotProviderTest, CreateSnapshot_EmptyFile_Success)
{
    std::string jobId = "121345";
    std::shared_ptr<DeviceMount> deviceMountPtr = std::make_shared<DeviceMount>();
    std::shared_ptr<LvmSnapshotProvider> lvmSnapshotPtr =
        std::make_shared<LvmSnapshotProvider>(deviceMountPtr, jobId, "/lvm_snapshots");
    SnapshotResult snapResult = lvmSnapshotPtr->CreateSnapshot("", false);
    EXPECT_EQ(snapResult.snapShotStatus == SNAPSHOT_STATUS::FAILED, true);
}

static bool Function_False()
{
    return false;
}
TEST_F(LvmSnapshotProviderTest, CreateSnapshot_Not_IsDir)
{
    stub.set(ADDR(LvmSnapshotProvider, IsDir), Function_False);
    std::string jobId = "121345";
    std::shared_ptr<DeviceMount> deviceMountPtr = std::make_shared<DeviceMount>();
    std::shared_ptr<LvmSnapshotProvider> lvmSnapshotPtr =
        std::make_shared<LvmSnapshotProvider>(deviceMountPtr, jobId, "/lvm_snapshots");
    SnapshotResult snapResult = lvmSnapshotPtr->CreateSnapshot("", false);
    EXPECT_EQ(snapResult.snapShotStatus == SNAPSHOT_STATUS::FAILED, true);

    stub.reset(ADDR(LvmSnapshotProvider, IsDir));
}

TEST_F(LvmSnapshotProviderTest, CreateSnapshot_File_Not_Exist_Success)
{
    std::string jobId = "123456";
    std::shared_ptr<DeviceMount> deviceMountPtr = std::make_shared<DeviceMount>();
    std::shared_ptr<LvmSnapshotProvider> lvmSnapshotPtr =
        std::make_shared<LvmSnapshotProvider>(deviceMountPtr, jobId, "/lvm_snapshots");
    SnapshotResult snapResult = lvmSnapshotPtr->CreateSnapshot("/home/noExist", false);
    EXPECT_EQ(snapResult.snapShotStatus == SNAPSHOT_STATUS::FAILED, true);
}

TEST_F(LvmSnapshotProviderTest, CreateSnapshot_Not_Support_Success)
{
    stub.set(ADDR(LvmSnapshotProvider, ExecShellCmd), Stub_CreateSnapShot_No_Support);
    std::string jobId = "123456";
    std::shared_ptr<DeviceMount> deviceMountPtr = std::make_shared<DeviceMount>();
    deviceMountPtr->LoadDevice();
    std::shared_ptr<LvmSnapshotProvider> lvmSnapshotPtr =
        std::make_shared<LvmSnapshotProvider>(deviceMountPtr, jobId, "/lvm_snapshots");
    SnapshotResult snapResult = lvmSnapshotPtr->CreateSnapshot("/home", false);
    stub.reset(ADDR(LvmSnapshotProvider, ExecShellCmd));
    EXPECT_EQ(snapResult.snapShotStatus == SNAPSHOT_STATUS::UNSUPPORTED, true);
}

// TEST_F(LvmSnapshotProviderTest, CreateSnapshot_Support_Success)
// {
    // stub.set(ADDR(LvmSnapshotProvider, ExecShellCmd), Stub_CreateSnapShot_Support);
    // std::string jobId = "123456";
    // std::shared_ptr<DeviceMount> deviceMountPtr = std::make_shared<DeviceMount>();
    // deviceMountPtr->LoadDevice();
    // std::shared_ptr<LvmSnapshotProvider> lvmSnapshotPtr = std::make_shared<LvmSnapshotProvider>(deviceMountPtr,jobId);
    // SnapshotResult snapResult = lvmSnapshotPtr->CreateSnapshot("/home", false);
    // EXPECT_EQ(snapResult.snapShotStatus == SNAPSHOT_STATUS::SUCCESS, true);
    // stub.reset(ADDR(LvmSnapshotProvider, ExecShellCmd));
// }

// TEST_F(LvmSnapshotProviderTest, CreateSnapshot_GetSubVolumes_Success)
// {
    // g_firstCall = true;
    // stub.set(ADDR(LvmSnapshotProvider, ExecShellCmd), Stub_CreateSnapShot_Support);
    // stub.set(ADDR(DeviceMount, GetSubVolumes), Stub_GetSubVolumes);
    // std::string jobId = "123456";
    // std::shared_ptr<DeviceMount> deviceMountPtr = std::make_shared<DeviceMount>();
    // deviceMountPtr->LoadDevice();
    // std::shared_ptr<LvmSnapshotProvider> lvmSnapshotPtr = std::make_shared<LvmSnapshotProvider>(deviceMountPtr,jobId);
    // std::vector<LvmSnapshot> volumeInfo;
    // lvmSnapshotPtr->GetSubVolumes("/home",volumeInfo);
    // bool result = volumeInfo.size() > 0;
    // EXPECT_EQ(result, true);
    // stub.reset(ADDR(LvmSnapshotProvider, ExecShellCmd));
    // stub.reset(ADDR(DeviceMount, GetSubVolumes));
// }

TEST_F(LvmSnapshotProviderTest, CreateSnapshot_GetLogicalVolume_Success)
{
    g_firstCall = false;
    stub.set(ADDR(LvmSnapshotProvider, ExecShellCmd), Stub_CreateSnapShot_Support);
    std::string jobId = "123456";
    std::shared_ptr<DeviceMount> deviceMountPtr = std::make_shared<DeviceMount>();
    deviceMountPtr->LoadDevice();
    std::shared_ptr<LvmSnapshotProvider> lvmSnapshotPtr =
        std::make_shared<LvmSnapshotProvider>(deviceMountPtr, jobId, "/lvm_snapshots");
    std::vector<LvmSnapshot> volumeInfo;
    bool result = lvmSnapshotPtr->GetLogicalVolume("/home", "false", volumeInfo);
    EXPECT_EQ(result, true);
    stub.reset(ADDR(LvmSnapshotProvider, ExecShellCmd));
}

TEST_F(LvmSnapshotProviderTest, DeleteSnapshot_ByTag_Success)
{
    g_firstCall = false;
    stub.set(ADDR(LvmSnapshotProvider, ExecShellCmd), Stub_DeleteSnapShot_Support);
    std::string jobId = "123456";
    std::shared_ptr<DeviceMount> deviceMountPtr = std::make_shared<DeviceMount>();
    std::shared_ptr<LvmSnapshotProvider> lvmSnapshotPtr
        = std::make_shared<LvmSnapshotProvider>(deviceMountPtr, jobId, "/lvm_snapshots");
    bool result = lvmSnapshotPtr->DeleteSnapshotByTag(jobId);
    EXPECT_EQ(result, true);
    stub.reset(ADDR(LvmSnapshotProvider, ExecShellCmd));
}

TEST_F(LvmSnapshotProviderTest, DeleteSnapshot_ByVolume_Success)
{
    g_firstCall = false;
    stub.set(Module::runShellCmdWithOutput, Stub_runShellCmdWithOutput_SUCCESS);
    std::string volumeName = "vg/lv";
    std::shared_ptr<DeviceMount> deviceMountPtr = std::make_shared<DeviceMount>();
    std::shared_ptr<LvmSnapshotProvider> lvmSnapshotPtr
        = std::make_shared<LvmSnapshotProvider>(deviceMountPtr, "jobId", "/lvm_snapshots");
    bool result = lvmSnapshotPtr->DeleteSnapshotByVolume(volumeName);
    EXPECT_EQ(result, true);
    stub.reset(ADDR(LvmSnapshotProvider, ExecShellCmd));
}

TEST_F(LvmSnapshotProviderTest, DeleteAllSnapshot_Success)
{
    g_firstCall = false;
    stub.set(ADDR(LvmSnapshotProvider, ExecShellCmd), Stub_DeleteSnapShot_Support);
    std::string volumeName = "vg/lv";
    std::shared_ptr<DeviceMount> deviceMountPtr = std::make_shared<DeviceMount>();
    std::shared_ptr<LvmSnapshotProvider> lvmSnapshotPtr
        = std::make_shared<LvmSnapshotProvider>(deviceMountPtr, "jobId", "/lvm_snapshots");
    SnapshotDeleteResult snapDeleteResult = lvmSnapshotPtr->DeleteAllSnapshots();
    EXPECT_EQ(snapDeleteResult.status, true);
    auto snapShot = std::make_shared<LvmSnapshot>(volumeName, "/home/xxx",
       "/mnt/lvm_snap", "/mnt/lvm_snap", "");
    lvmSnapshotPtr->deviceVolumeSnapMap.emplace(snapShot->m_oriDeviceVolume, snapShot);
    snapDeleteResult = lvmSnapshotPtr->DeleteAllSnapshots();
    EXPECT_EQ(snapDeleteResult.status, true);
    stub.reset(ADDR(LvmSnapshotProvider, ExecShellCmd));
}

// TEST_F(LvmSnapshotProviderTest, QuerySnapshot_Success)
// {
    // g_firstCall = true;
    // stub.set(ADDR(LvmSnapshotProvider, ExecShellCmd), Stub_CreateSnapShot_Support);
    // std::string jobId = "123456";
    // std::shared_ptr<DeviceMount> deviceMountPtr = std::make_shared<DeviceMount>();
    // deviceMountPtr->LoadDevice();
    // std::shared_ptr<LvmSnapshotProvider> lvmSnapshotPtr = std::make_shared<LvmSnapshotProvider>(deviceMountPtr,jobId);
    // auto snapShot = std::make_shared<LvmSnapshot>("vg/lv", "/home/xxx",
    //    "/mnt/lvm_snap", "/mnt/lvm_snap");
    // lvmSnapshotPtr->deviceVolumeSnapMap.emplace(snapShot->m_oriDeviceVolume, snapShot);
    // SnapshotResult snapResult = lvmSnapshotPtr->QuerySnapshot("/home");
    // EXPECT_EQ(snapResult.snapShotStatus == SNAPSHOT_STATUS::SUCCESS, true);
    // stub.reset(ADDR(LvmSnapshotProvider, ExecShellCmd));
// }

static std::string Stub_Empty_Str()
{
    return "";
}
TEST_F(LvmSnapshotProviderTest, QuerySnapshot_realPath_Empty)
{
    stub.set(ADDR(LvmSnapshotProvider, ExecShellCmd), Stub_CreateSnapShot_Support);
    stub.set(ADDR(LvmSnapshotProvider, GetRealPath), Stub_Empty_Str);
    std::string jobId = "123456";
    std::shared_ptr<DeviceMount> deviceMountPtr = std::make_shared<DeviceMount>();
    deviceMountPtr->LoadDevice();
    std::shared_ptr<LvmSnapshotProvider> lvmSnapshotPtr
        = std::make_shared<LvmSnapshotProvider>(deviceMountPtr, jobId, "/lvm_snapshots");
    auto snapShot = std::make_shared<LvmSnapshot>("vg/lv", "/home/xxx",
       "/mnt/lvm_snap", "/mnt/lvm_snap", "");
    lvmSnapshotPtr->deviceVolumeSnapMap.emplace(snapShot->m_oriDeviceVolume, snapShot);
    SnapshotResult snapResult = lvmSnapshotPtr->QuerySnapshot("/home");
    EXPECT_EQ(snapResult.snapShotStatus == SNAPSHOT_STATUS::SUCCESS, false);
    stub.reset(ADDR(LvmSnapshotProvider, ExecShellCmd));
    stub.reset(ADDR(LvmSnapshotProvider, GetRealPath));
}

TEST_F(LvmSnapshotProviderTest, GetDirName_Success)
{
    std::string jobId = "123456";
    std::shared_ptr<DeviceMount> deviceMountPtr = std::make_shared<DeviceMount>();
    std::shared_ptr<LvmSnapshotProvider> lvmSnapshotPtr =
        std::make_shared<LvmSnapshotProvider>(deviceMountPtr, jobId, "/lvm_snapshots");
    std::string dirStr = lvmSnapshotPtr->GetDirName("/home");
    bool result = dirStr.empty();
    EXPECT_EQ(result, false);
}

TEST_F(LvmSnapshotProviderTest, MonutSnapshot_EmptyDir_Success)
{
    std::string jobId = "123456";
    std::shared_ptr<DeviceMount> deviceMountPtr = std::make_shared<DeviceMount>();
    std::shared_ptr<LvmSnapshotProvider> lvmSnapshotPtr
        = std::make_shared<LvmSnapshotProvider>(deviceMountPtr, jobId, "/lvm_snapshots");
    bool result = lvmSnapshotPtr->MountSnapshot("/dev/vg/lv", "");
    EXPECT_EQ(result, false);
}

TEST_F(LvmSnapshotProviderTest, MonutSnapshot_DeviceInvalid_Success)
{
    std::string jobId = "45678";
    std::shared_ptr<DeviceMount> deviceMountPtr = std::make_shared<DeviceMount>();
    std::shared_ptr<LvmSnapshotProvider> lvmSnapshotPtr =
        std::make_shared<LvmSnapshotProvider>(deviceMountPtr, jobId, "/lvm_snapshots");
    bool result = lvmSnapshotPtr->MountSnapshot("vg/lv", "/opt/lvm-snapshots/home");
    EXPECT_EQ(result, false);
}

TEST_F(LvmSnapshotProviderTest, ConcatPath)
{
    std::string jobId = "45678";
    std::shared_ptr<DeviceMount> deviceMountPtr = std::make_shared<DeviceMount>();
    std::shared_ptr<LvmSnapshotProvider> lvmSnapshotPtr =
        std::make_shared<LvmSnapshotProvider>(deviceMountPtr, jobId, "/lvm_snapshots");

    std::string first = "a/";
    std::string second = "";
    std::string result = lvmSnapshotPtr->ConcatPath(first, second);
    EXPECT_EQ(result, first);

    first = "a/";
    second = "/b";
    result = lvmSnapshotPtr->ConcatPath(first, second);
    EXPECT_EQ(result, "a/b");
}

