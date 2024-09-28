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
#include "common/DirtyRanges.h"
#include "volume_handlers/fusionstorage/FusionStorageBitmapHandle.h"

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

class FusionStorageBitmapHandleTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();

private:
    std::shared_ptr<DiskDataPersistence> diskDataPersistence;
    std::shared_ptr<FusionStorageApi> m_spDSWareApi;
};

void FusionStorageBitmapHandleTest::SetUp()
{
    std::string m_fusionStorMgrIp = "127.0.0.1";
    std::string poolID = "123";
    diskDataPersistence = std::make_shared<DiskDataPersistence>();
    m_spDSWareApi = std::make_shared<FusionStorageApi>(m_fusionStorMgrIp, poolID);
}

void FusionStorageBitmapHandleTest::TearDown()
{}

void FusionStorageBitmapHandleTest::SetUpTestCase()
{}

void FusionStorageBitmapHandleTest::TearDownTestCase()
{}

static int32_t ExecStubSuccess()
{
    return SUCCESS;
}

static int32_t ExecStubFailed()
{
    return FAILED;
}

static std::string GetBitmapVolNameStub()
{
    return "12_5_124543536_BITMAP";
}

static bool DirtyRangeIsRestoreStub()
{
    return false;
}

static void InitBitmapVolumeInfo(BitmapVolumeInfo &info)
{
    info.snapNameFrom = "14";
    info.snapNameTo = "20";
    info.volName = "volume_1";
    info.usedInRestore = false;
    info.snapNameFrom = "12";
    info.snapNameTo = "20";
    info.blockSize = 8 * 1024;
}

static void InitDirtyRangesParams(DirtyRangesParams &params)
{
    params.offset = 0;
    params.diskSize = 4 * 1024 * 1024;
    params.parentVolumeID = "12";
    params.volumeID = "20";
}

static void DeleteAndDetachapVolumeStub(std::string &volumeName, std::string &errMsg)
{
    errMsg = "0";
}

/*
 * 用例名称：获取DirtyRange
 * 前置条件：无
 * check点：1、执行成功，获取DirtyRange成功
 */
TEST_F(FusionStorageBitmapHandleTest, GetDirtyRangesTestSuccess)
{
    Stub stub;
    DirtyRangesParams info;
    DirtyRanges dirtyRanges;
    std::string errDes;
    InitDirtyRangesParams(info);

    FusionStorageBitmapHandle fusionStorageBitmapHandle("127.0.0.1", "123", true, "12", diskDataPersistence);
    stub.set(ADDR(FusionStorageBitmapHandle, GetBitmapVolName), GetBitmapVolNameStub);
    stub.set(ADDR(DirtyRanges, IsRestore), DirtyRangeIsRestoreStub);
    stub.set(ADDR(FusionStorageBitmapHandle, CreateAndAttachBitmapVolume), ExecStubSuccess);
    stub.set(ADDR(FusionStorageBitmapHandle, CalculateDirtyRange), ExecStubSuccess);
    stub.set(ADDR(FusionStorageBitmapHandle, DeleteAndDetachapVolume), ExecStubSuccess);
    EXPECT_EQ(fusionStorageBitmapHandle.GetDirtyRanges(info, dirtyRanges, errDes), SUCCESS);
}

/*
 * 用例名称：获取DirtyRange
 * 前置条件：无
 * check点：1、获取DirtyRanges失败, 创建和挂载位图卷失败
 */
TEST_F(FusionStorageBitmapHandleTest, GetDirtyRangesTestFailed1)
{
    Stub stub;
    DirtyRangesParams info;
    DirtyRanges dirtyRanges;
    std::string errDes;
    InitDirtyRangesParams(info);

    FusionStorageBitmapHandle fusionStorageBitmapHandle("127.0.0.1", "123", true, "12", diskDataPersistence);
    stub.set(ADDR(FusionStorageBitmapHandle, GetBitmapVolName), GetBitmapVolNameStub);
    stub.set(ADDR(DirtyRanges, IsRestore), DirtyRangeIsRestoreStub);
    stub.set(ADDR(FusionStorageBitmapHandle, CreateAndAttachBitmapVolume), ExecStubFailed);
    stub.set(ADDR(FusionStorageBitmapHandle, DeleteAndDetachapVolume), ExecStubSuccess);
    EXPECT_NE(fusionStorageBitmapHandle.GetDirtyRanges(info, dirtyRanges, errDes), SUCCESS);
}

/*
 * 用例名称：获取DirtyRange
 * 前置条件：无
 * check点：1、获取DirtyRanges失败, 计算DirtyRange失败
 */
TEST_F(FusionStorageBitmapHandleTest, GetDirtyRangesTestFailed2)
{
    Stub stub;
    DirtyRangesParams info;
    DirtyRanges dirtyRanges;
    std::string errDes;
    InitDirtyRangesParams(info);

    FusionStorageBitmapHandle fusionStorageBitmapHandle("127.0.0.1", "123", true, "12", diskDataPersistence);
    stub.set(ADDR(FusionStorageBitmapHandle, GetBitmapVolName), GetBitmapVolNameStub);
    stub.set(ADDR(DirtyRanges, IsRestore), DirtyRangeIsRestoreStub);
    stub.set(ADDR(FusionStorageBitmapHandle, CreateAndAttachBitmapVolume), ExecStubSuccess);
    stub.set(ADDR(FusionStorageBitmapHandle, CalculateDirtyRange), ExecStubFailed);
    stub.set(ADDR(FusionStorageBitmapHandle, DeleteAndDetachapVolume), ExecStubSuccess);
    EXPECT_NE(fusionStorageBitmapHandle.GetDirtyRanges(info, dirtyRanges, errDes), SUCCESS);
}

/*
 * 用例名称：获取DirtyRange
 * 前置条件：无
 * check点：1、获取DirtyRanges失败, 删除和去挂载卷失败
 */
TEST_F(FusionStorageBitmapHandleTest, GetDirtyRangesTestFailed3)
{
    Stub stub;
    DirtyRangesParams info;
    DirtyRanges dirtyRanges;
    std::string errDes;
    InitDirtyRangesParams(info);

    FusionStorageBitmapHandle fusionStorageBitmapHandle("127.0.0.1", "123", true, "12", diskDataPersistence);
    stub.set(ADDR(FusionStorageBitmapHandle, GetBitmapVolName), GetBitmapVolNameStub);
    stub.set(ADDR(DirtyRanges, IsRestore), DirtyRangeIsRestoreStub);
    stub.set(ADDR(FusionStorageBitmapHandle, CreateAndAttachBitmapVolume), ExecStubSuccess);
    stub.set(ADDR(FusionStorageBitmapHandle, CalculateDirtyRange), ExecStubSuccess);
    stub.set(ADDR(FusionStorageBitmapHandle, DeleteAndDetachapVolume), DeleteAndDetachapVolumeStub);
    EXPECT_EQ(fusionStorageBitmapHandle.GetDirtyRanges(info, dirtyRanges, errDes), SUCCESS);
}

/*
 * 用例名称：创建和挂载位图卷
 * 前置条件：无
 * check点：1、创建挂载位图卷成功
 */
TEST_F(FusionStorageBitmapHandleTest, CreateAndAttachBitmapVolumeSuccess)
{
    Stub stub;
    BitmapVolumeInfo info;
    std::string errDes;
    InitBitmapVolumeInfo(info);

    FusionStorageBitmapHandle fusionStorageBitmapHandle("127.0.0.1", "123", true, "12", diskDataPersistence);
    stub.set(ADDR(DiskDataPersistence, AddObject), ExecStubSuccess);
    stub.set(ADDR(DiskDataPersistence, AppendArrayElement), ExecStubSuccess);
    stub.set(ADDR(FusionStorageBitmapHandle, RetryOp), ExecStubSuccess);
    EXPECT_EQ(fusionStorageBitmapHandle.CreateAndAttachBitmapVolume(info, errDes), SUCCESS);
}

/*
 * 用例名称：删除和去挂载卷
 * 前置条件：无
 * check点：1、删除和去挂载卷成功
 */
TEST_F(FusionStorageBitmapHandleTest, DeleteAndDetachapVolumeSuccess)
{
    Stub stub;
    std::string volumeName = "volume_123";
    std::string errDes;

    FusionStorageBitmapHandle fusionStorageBitmapHandle("127.0.0.1", "123", true, "12", diskDataPersistence);
    stub.set(ADDR(FusionStorageBitmapHandle, RetryOp), ExecStubSuccess);
    stub.set(ADDR(DiskDataPersistence, AppendArrayElement), ExecStubSuccess);
}

/*
 * 用例名称：计算DirtyRanges
 * 前置条件：无
 * check点：1、计算DirtyRanges成功
 */
// TEST_F(FusionStorageBitmapHandleTest, CalculateDirtyRangeSuccess)
// {
//     Stub stub;
//     uint64_t startOffset = 0;
//     DirtyRanges dirtyRanges;

//     uint64_t m_trunkSize = 8 * 1024;
//     uint64_t m_diskSize = 10 * 1024 * 1024 * 1024;
//     uint64_t m_bitmapVolumeSizeInBytes = 10 * 1024 * 1024 * 1024;

//     FusionStorageBitmapHandle fusionStorageBitmapHandle("127.0.0.1", "123", true, "12", diskDataPersistence);
//     stub.set(ADDR(DiskDeviceFile, Open), ExecStubSuccess);
//     stub.set(ADDR(DiskDeviceFile, Read), ExecStubSuccess);
//     stub.set(ADDR(FusionStorageBitmapHandle, AddDitryRange), ExecStubSuccess);
//     stub.set(ADDR(DiskDeviceFile, Close), ExecStubSuccess);
//     EXPECT_EQ(fusionStorageBitmapHandle.CalculateDirtyRange(startOffset, dirtyRanges), SUCCESS);
// }
}  // namespace HDT_TEST
