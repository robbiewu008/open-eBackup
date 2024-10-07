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
#include "protect_engines\hyperv\resource_discovery\HyperVResourceAccess.h"
#include "protect_engines\hyperv\utils\executor\WinCmdExector.h"
#include <vector>
#include <map>
#include <json/json.h>
#include "common/JsonHelper.h"
#include "thrift_interface/ApplicationProtectBaseDataType_types.h"
#include "thrift_interface/ApplicationProtectPlugin_types.h"
#include "thrift_interface/ApplicationProtectFramework_types.h"
using namespace AppProtect;
using namespace HyperVPlugin;
using namespace HyperVPlugin::Utils;

namespace {
const std::string CMD_TYPE_CHECK_CONNECTION_SCVMM = "CheckSCConnection";
const std::string CMD_TYPE_CHECK_CONNECTION_HOST = "CheckHostConnection";
}  // namespace
class HyperVResourceAccessTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
};

void HyperVResourceAccessTest::SetUp()
{}

void HyperVResourceAccessTest::TearDown()
{}

void HyperVResourceAccessTest::SetUpTestCase()
{}

void HyperVResourceAccessTest::TearDownTestCase()
{}
class ResourceAccessStub {
public:
    int StubExecutecSuccess(const Param &cmdParam, Json::Value &result)
    {
        return SUCCESS;
    }
    int StubExecutecGetClusterListSuccess(const Param &cmdParam, Json::Value &result)
    {
        Json::Value hyperVClusterInfo;
        hyperVClusterInfo["Name"] = "ClusterName";
        hyperVClusterInfo["ID"] = "578896ad-ade9-4822-b413-d558bd7a369b";
        hyperVClusterInfo["IPAddresses"] = "192.168.9.63";
        result["result"].append(hyperVClusterInfo);
        return SUCCESS;
    }
    int StubExecutecGetHostListSuccess(const Param &cmdParam, Json::Value &result)
    {
        Json::Value hyperVInfo;
        hyperVInfo["Name"] = "ClusterName";
        hyperVInfo["ID"] = "578896ad-ade9-4822-b413-d558bd7a369b";
        hyperVInfo["OverallState"] = "0";
        hyperVInfo["VirtualizationPlatform"] = "2";
        hyperVInfo["HyperVVersion"] = "10.0.14393.0";
        result["result"].append(hyperVInfo);
        return SUCCESS;
    }
    int StubExecutecGetVMListSuccess(const Param &cmdParam, Json::Value &result)
    {
        Json::Value hyperVInfo;
        hyperVInfo["Name"] = "ClusterName";
        hyperVInfo["ID"] = "578896ad-ade9-4822-b413-d558bd7a369b";
        hyperVInfo["State"] = "3";
        hyperVInfo["Generation"] = "2";
        hyperVInfo["Version"] = "5.0";
        result["result"].append(hyperVInfo);
        return SUCCESS;
    }
    int StubExecutecGetDiskListSuccess(const Param &cmdParam, Json::Value &result)
    {
        Json::Value hyperVInfo;
        hyperVInfo["DiskIdentifier"] = "2C7278D1-4058-4620-9A18-FFE57F90AE41";
        hyperVInfo["Size"] = "136365211648";
        hyperVInfo["VhdFormat"] = "3";
        hyperVInfo["VhdType"] = "4";
        hyperVInfo["Path"] = "C:\\Users\\Public\\Documents\\Hyper-V\\
              Virtual Hard Disks\\CentOS7_04455719-5F4C-4ED1-AA4B-631F44AB561C.avhdx";
        hyperVInfo["ParentPath"] = "C:\\Users\\Public\\Documents\\Hyper-V\\Virtual Hard Disks\\CentOS7.vhdx";
        result["result"].append(hyperVInfo);
        return SUCCESS;
    }
    int StubExecutecGetDirectoryListSuccess(const Param &cmdParam, Json::Value &result)
    {
        Json::Value hyperVInfo;
        hyperVInfo["Name"] = "ClusterName";
        hyperVInfo["Length"] = "532423";
        hyperVInfo["LastWriteTime"] = "2022-05-25";
        hyperVInfo["Mode"] = "directory";
        result["result"].append(hyperVInfo);
        return SUCCESS;
    }
    int StubExecutecFailed(const Param &cmdParam, Json::Value &result)
    {
        return FAILED;
    }
};
/**
 * 测试用例：测试SCVMM连通性
 * 前置条件：windows命令执行成功
 * Check点：测试SCVMM连通性成功
 */
TEST_F(HyperVResourceAccessTest, CheckSCVMMConnectionSuccess)
{

    Stub stub;
    stub.set(ADDR(WinCmdExector, Execute), ADDR(ResourceAccessStub, StubExecutecSuccess));
    ApplicationEnvironment appEnv;
    Application application;
    HyperVResourceAccess access(appEnv, application);
    ActionResult returnValue;
    int32_t result = access.CheckSCVMMConnection(returnValue);
    EXPECT_EQ(result, SUCCESS);
}
/**
 * 测试用例：测试Host连通性
 * 前置条件：windows命令执行成功
 * Check点：测试Host连通性成功
 */
TEST_F(HyperVResourceAccessTest, CheckHostConnectionSuccess)
{

    Stub stub;
    stub.set(ADDR(WinCmdExector, Execute), ADDR(ResourceAccessStub, StubExecutecSuccess));
    ApplicationEnvironment appEnv;
    Application application;
    HyperVResourceAccess access(appEnv, application);
    ActionResult returnValue;
    int32_t result = access.CheckHostConnection(returnValue);
    EXPECT_EQ(result, SUCCESS);
}
/**
 * 测试用例：获取集群列表
 * 前置条件：windows命令执行成功，返回集群列表
 * Check点：获取集群列表成功
 */
TEST_F(HyperVResourceAccessTest, GetClusterListSuccess)
{

    Stub stub;
    stub.set(ADDR(WinCmdExector, Execute), ADDR(ResourceAccessStub, StubExecutecGetClusterListSuccess));
    ApplicationEnvironment appEnv;
    Application application;
    HyperVResourceAccess access(appEnv, application);
    ResourceResultByPage page;
    int32_t result = access.GetClusterList(page);
    EXPECT_EQ(result, SUCCESS);
}
/**
 * 测试用例：获取主机列表
 * 前置条件：windows命令执行成功，返回主机列表
 * Check点：获取主机列表成功
 */
TEST_F(HyperVResourceAccessTest, GetHostListSuccess)
{

    Stub stub;
    stub.set(ADDR(WinCmdExector, Execute), ADDR(ResourceAccessStub, StubExecutecGetHostListSuccess));
    ApplicationEnvironment appEnv;
    Application application;
    HyperVResourceAccess access(appEnv, application);
    ResourceResultByPage page;
    int32_t result = access.GetHostList(page);
    EXPECT_EQ(result, SUCCESS);
}
/**
 * 测试用例：获取VM列表
 * 前置条件：windows命令执行成功，返回VM列表
 * Check点：获取VM列表成功
 */
TEST_F(HyperVResourceAccessTest, GetVMListSuccess)
{

    Stub stub;
    stub.set(ADDR(WinCmdExector, Execute), ADDR(ResourceAccessStub, StubExecutecGetVMListSuccess));
    ApplicationEnvironment appEnv;
    Application application;
    HyperVResourceAccess access(appEnv, application);
    ResourceResultByPage page;
    int32_t result = access.GetVMList(page);
    EXPECT_EQ(result, SUCCESS);
}
/**
 * 测试用例：获取Disk列表
 * 前置条件：windows命令执行成功，返回Disk列表
 * Check点：获取Disk列表成功
 */
TEST_F(HyperVResourceAccessTest, GetDiskListSuccess)
{

    Stub stub;
    stub.set(ADDR(WinCmdExector, Execute), ADDR(ResourceAccessStub, StubExecutecGetDiskListSuccess));
    ApplicationEnvironment appEnv;
    Application application;
    HyperVResourceAccess access(appEnv, application);
    ResourceResultByPage page;
    int32_t result = access.GetDiskList(page);
    EXPECT_EQ(result, SUCCESS);
}
/**
 * 测试用例：获取Directory列表
 * 前置条件：windows命令执行成功，返回Directory列表
 * Check点：获取Directory列表成功
 */
TEST_F(HyperVResourceAccessTest, GetDirectoryListSuccess)
{

    Stub stub;
    stub.set(ADDR(WinCmdExector, Execute), ADDR(ResourceAccessStub, StubExecutecGetDirectoryListSuccess));
    ApplicationEnvironment appEnv;
    Application application;
    HyperVResourceAccess access(appEnv, application);
    ResourceResultByPage page;
    int32_t result = access.GetDirectoryList(page);
    EXPECT_EQ(result, SUCCESS);
}
