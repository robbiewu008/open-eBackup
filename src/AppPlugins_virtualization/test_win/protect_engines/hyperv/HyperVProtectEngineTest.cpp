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
#include "addr_pri.h"
#include <tchar.h>
#include <iostream>
#include <map>
#include <cerrno>
#include <memory>
#include <fstream>
#include "protect_engines/hyperv/utils/executor/WinCmdExector.h"
#include "protect_engines/hyperv/resource_discovery/HyperVResourceAccess.h"
#include "protect_engines/hyperv/HyperVProtectEngine.h"
#include "log/Log.h"
#include "common/uuid/Uuid.h"
#include "securec.h"
#include "common/Types.h"
#include "common/File.h"
#include "common/Constants.h"
#include <vector>
#include <winsock2.h>
#include <windows.h>
#include <WinBase.h>
#include "common/JsonHelper.h"
#include <map>
#include "cstdint"
#include "protect_engines/ProtectEngine.h"
#include "protect_engines/hyperv/api/wmi_client/WMIClient.h"
#include "protect_engines/hyperv/api/powershell/PSClient.h"
using namespace HyperVPlugin;
using namespace HyperVPlugin::Utils;

namespace {
const std::string TARGET_TYPE_SCVMM = "SCVMM";
const std::string TARGET_TYPE_CLUSTER = "Cluster";
const std::string TARGET_TYPE_HOST = "Host";
const std::string TARGET_TYPE_VM = "VM";
const std::string TARGET_TYPE_DISK = "Disk";
const std::string TARGET_TYPE_DIRECTORY = "Directory";
const std::string TARGET_TYPE_KEY = "targetType";
const std::string FAILED_CODE = "-1";
}  // namespace
class HyperVProtectEngineTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
};

void HyperVProtectEngineTest::SetUp()
{}

void HyperVProtectEngineTest::TearDown()
{}

void HyperVProtectEngineTest::SetUpTestCase()
{}

void HyperVProtectEngineTest::TearDownTestCase()
{}
class HyperVProtectEngineStub {
public:
    int StubExecutecSuccess(const Param &cmdParam, Json::Value &result)
    {
        return HyperVPlugin::SUCCESS;
    }
    int StubExecutecFailed(const Param &cmdParam, Json::Value &result)
    {
        return HyperVPlugin::FAILED;
    }
    int32_t StubCheckSCVMMConnectionSuccess(ActionResult &returnValue)
    {
        returnValue.__set_code(1);
        return HyperVPlugin::SUCCESS;
    }
    int StubGetClusterListSuccess(const Param &cmdParam, Json::Value &result)
    {
        return HyperVPlugin::SUCCESS;
    }
    int StubExecutecGetHostListSuccess(const Param &cmdParam, Json::Value &result)
    {
        Json::Value hyperVClusterInfo;
        hyperVClusterInfo["Name"] = "ClusterName";
        hyperVClusterInfo["ID"] = "578896ad-ade9-4822-b413-d558bd7a369b";
        hyperVClusterInfo["OverallState"] = "0";
        hyperVClusterInfo["VirtualizationPlatform"] = "2";
        hyperVClusterInfo["HyperVVersion"] = "10.0.14393.0";
        result["result"].append(hyperVClusterInfo);
        return HyperVPlugin::SUCCESS;
    }

};
/**
 * 测试用例：检查主机连通性测试
 * 前置条件：目标主机为空
 * Check点：检查主机连通性失败
 */
TEST_F(HyperVProtectEngineTest, TestCheckApplicationFailed)
{
    Stub stub;
    stub.set(ADDR(HyperVResourceAccess, CheckSCVMMConnection),
        ADDR(HyperVProtectEngineStub, StubCheckSCVMMConnectionSuccess));
    ApplicationEnvironment appEnv;
    Application application;
    ActionResult returnValue;
    HyperVProtectEngine hyperVProtectEngine;
    hyperVProtectEngine.CheckApplication(returnValue, appEnv, application);
    EXPECT_EQ(returnValue.code, 0);
}
/**
 * 测试用例：检查主机连通性测试
 * 前置条件：目标主机为SCVMM
 * Check点：检查主机连通性成功
 */
TEST_F(HyperVProtectEngineTest, TestCheckApplicationSuccess)
{
    Stub stub;
    stub.set(ADDR(HyperVResourceAccess, CheckSCVMMConnection),
        ADDR(HyperVProtectEngineStub, StubCheckSCVMMConnectionSuccess));

    ApplicationEnvironment appEnv;
    Application application;
    ActionResult returnValue;
    HyperVProtectEngine hyperVProtectEngine;
    appEnv.__set_extendInfo("{\"targetType\": \"SCVMM\"}");
    hyperVProtectEngine.CheckApplication(returnValue, appEnv, application);
    EXPECT_EQ(returnValue.code, 1);
}
/**
 * 测试用例：获取主机资源列表
 * 前置条件：目标主机为Cluster
 * Check点：获取集群列表成功
 */
TEST_F(HyperVProtectEngineTest, TestListApplicationResourceV2Success)
{
    Stub stub;
    stub.set(
        ADDR(HyperVResourceAccess, GetClusterList), ADDR(HyperVProtectEngineStub, StubGetClusterListSuccess));
    ListResourceRequest req;
    ApplicationEnvironment appEnv;
    Application application;
    ActionResult returnValue;
    QueryByPage queryByPage;
    queryByPage.__set_pageNo(1);
    queryByPage.__set_pageSize(10);
    queryByPage.__set_conditions("{\"targetType\": \"Cluster\"}");
    req.__set_condition(queryByPage);
    req.__set_appEnv(appEnv);
    std::vector<Application> val;
    val.push_back(application);
    req.__set_applications(val);
    ResourceResultByPage result;
    HyperVProtectEngine hyperVProtectEngine;
    HyperVProtectEngine *hy = &hyperVProtectEngine;
    hy->ListApplicationResourceV2(result, req);

}
