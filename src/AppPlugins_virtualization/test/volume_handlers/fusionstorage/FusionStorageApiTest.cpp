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
#include "volume_handlers/VolumeHandler.h"
#include "volume_handlers/fusionstorage/FusionStorageApi.h"

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

typedef int32_t (*fptr)(const severity_level &severity, const std::string &moduleName, const std::size_t &requestID,
    const std::string &cmd, const std::vector<std::string> params, std::vector<std::string> &cmdoutput,
    std::string &stderroutput);

class FusionStorageApiTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
};

void FusionStorageApiTest::SetUp()
{
    std::string m_fusionStorMgrIp = "127.0.0.1";
}

void FusionStorageApiTest::TearDown()
{}

void FusionStorageApiTest::SetUpTestCase()
{}

void FusionStorageApiTest::TearDownTestCase()
{}

static int32_t ExecStubSuccess()
{
    return SUCCESS;
}

static int32_t ExecStubFailed()
{
    return FAILED;
}

static void InitBitmapVolumeInfo(BitmapVolumeInfo &info)
{
    info.snapNameFrom = "14";
    info.snapNameTo = "20";
    info.volName = "volume_1";
    info.usedInRestore = false;
}

static int32_t RunShellCmdWithOutputSucc(const severity_level &severity, const std::string &moduleName,
    const std::size_t &requestID, const std::string &cmd, const std::vector<std::string> params,
    std::vector<std::string> &cmdoutput, std::string &stderroutput)
{
    cmdoutput.push_back("ret_code=0");
    cmdoutput.push_back("ret_desc=succ");
    cmdoutput.push_back("dev_addr=/tmp/virtual/");
    cmdoutput.push_back("ResultCode=0");
    cmdoutput.push_back("snapSize=123");
    cmdoutput.push_back("blockSize=123");
    cmdoutput.push_back("volSize=123");
    cmdoutput.push_back("status=0");
    return SUCCESS;
}

/*
 * 用例名称：创建差量位图卷
 * 前置条件：无
 * check点：1、执行成功，创建成功
 */
TEST_F(FusionStorageApiTest, CreateBitmapVolumeTestSuccess)
{
    Stub stub;
    BitmapVolumeInfo info;
    std::string errDes;
    InitBitmapVolumeInfo(info);

    FusionStorageApi storageApiHandler;
    stub.set(ADDR(FusionStorageApi, DoCreateBitmapVol), ExecStubSuccess);
    stub.set(ADDR(FusionStorageApi, QueryBitmapVol), ExecStubSuccess);
    EXPECT_NE(storageApiHandler.CreateBitmapVolume(info, errDes), SUCCESS);
}

/*
 * 用例名称：创建差量位图卷
 * 前置条件：无
 * check点：1、创建位图卷失败
 */
TEST_F(FusionStorageApiTest, CreateBitmapVolumeTestFailed1)
{
    Stub stub;
    BitmapVolumeInfo info;
    std::string errDes;
    InitBitmapVolumeInfo(info);

    FusionStorageApi storageApiHandler;
    stub.set(ADDR(FusionStorageApi, DoCreateBitmapVol), ExecStubFailed);
    EXPECT_NE(storageApiHandler.CreateBitmapVolume(info, errDes), SUCCESS);
}

/*
 * 用例名称：创建差量位图卷
 * 前置条件：无
 * check点：1、失败：创建位图卷成功，查询位图卷失败
 */
TEST_F(FusionStorageApiTest, CreateBitmapVolumeTestFailed2)
{
    Stub stub;
    BitmapVolumeInfo info;
    std::string errDes;
    InitBitmapVolumeInfo(info);

    FusionStorageApi storageApiHandler;
    stub.set(ADDR(FusionStorageApi, DoCreateBitmapVol), ExecStubSuccess);
    stub.set(ADDR(FusionStorageApi, QueryBitmapVol), ExecStubFailed);
    EXPECT_NE(storageApiHandler.CreateBitmapVolume(info, errDes), SUCCESS);
}

/*
 * 用例名称：删除差量位图卷
 * 前置条件：无
 * check点：1、删除差量位图卷是否正常
 */
TEST_F(FusionStorageApiTest, DeleteBitmapVolumeTest)
{
    Stub stub;
    const std::string volumeName = "volume_1";
    std::string errDes;

    FusionStorageApi storageApiHandler;
    stub.set(ADDR(FusionStorageApi, GetDswareAgentIp), ExecStubSuccess);
    stub.set(ADDR(FusionStorageApi, DeleteBitmapVolumeNoRetry), ExecStubSuccess);
    EXPECT_EQ(storageApiHandler.DeleteBitmapVolume(volumeName, errDes), SUCCESS);

    stub.set(ADDR(FusionStorageApi, GetDswareAgentIp), ExecStubSuccess);
    stub.set(ADDR(FusionStorageApi, DeleteBitmapVolumeNoRetry), ExecStubFailed);
    stub.set(ADDR(FusionStorageApi, GetErrString), ExecStubSuccess);
    EXPECT_NE(storageApiHandler.DeleteBitmapVolume(volumeName, errDes), SUCCESS);
}

/*
 * 用例名称：挂载差量位图卷
 * 前置条件：无
 * check点：1、挂载差量位图卷是否正常
 */
TEST_F(FusionStorageApiTest, AttachVolumeTest)
{
    Stub stub;
    std::string volumeName = "volume_1";
    std::string diskDevicePath = "/tmp/virtual";
    std::string errDes;

    FusionStorageApi storageApiHandler;

    fptr runShellCmdPtr = (fptr)(&Module::runShellCmdWithOutput);
    stub.set(runShellCmdPtr, RunShellCmdWithOutputSucc);
    EXPECT_EQ(storageApiHandler.AttachVolume(volumeName, diskDevicePath, errDes), SUCCESS);
}

/*
 * 用例名称：卸载差量位图卷
 * 前置条件：无
 * check点：1、卸载差量位图卷是否正常
 */
TEST_F(FusionStorageApiTest, DetachVolumeTest)
{
    Stub stub;
    std::string volumeName = "volume_1";
    std::string errDes;

    FusionStorageApi storageApiHandler;

    fptr runShellCmdPtr = (fptr)(&Module::runShellCmdWithOutput);
    stub.set(runShellCmdPtr, RunShellCmdWithOutputSucc);
    EXPECT_EQ(storageApiHandler.DetachVolume(volumeName, errDes), SUCCESS);
}

/*
 * 用例名称：删除差量位图卷
 * 前置条件：无
 * check点：1、删除差量位图卷是否正常
 */
TEST_F(FusionStorageApiTest, DeleteBitmapVolumeNoRetryTest)
{
    Stub stub;
    std::string volumeName = "volume_1";
    std::string errDes;

    FusionStorageApi storageApiHandler;

    fptr runShellCmdPtr = (fptr)(&Module::runShellCmdWithOutput);
    stub.set(runShellCmdPtr, RunShellCmdWithOutputSucc);
    EXPECT_EQ(storageApiHandler.DeleteBitmapVolumeNoRetry(volumeName, errDes), SUCCESS);
}

/*
 * 用例名称：查询差量位图卷
 * 前置条件：无
 * check点：1、查询差量位图卷是否正常
 */
TEST_F(FusionStorageApiTest, QueryBitmapVolTest)
{
    Stub stub;
    BitmapVolumeInfo info;
    std::string errDes;
    InitBitmapVolumeInfo(info);

    FusionStorageApi storageApiHandler;

    fptr runShellCmdPtr = (fptr)(&Module::runShellCmdWithOutput);
    stub.set(runShellCmdPtr, RunShellCmdWithOutputSucc);
    EXPECT_EQ(storageApiHandler.QueryBitmapVol(info, errDes), SUCCESS);
}
}  // namespace HDT_TEST
