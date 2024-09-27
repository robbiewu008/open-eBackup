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
#include "log/Log.h"
#include "ApplicationServiceImp.h"
#include "OpenLibMgr.h"
#include "ApplicationProtectFramework_types.h"

using ::testing::_;
using ::testing::AtLeast;
using testing::DoAll;
using testing::InitGoogleMock;
using ::testing::Return;
using ::testing::Sequence;
using ::testing::SetArgReferee;
using ::testing::Throw;

using namespace AppProtect;

namespace {
    const int INNER_ERROR = 200;
}

class ApplicationServiceImpTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    std::unique_ptr<ApplicationServiceImp> m_appPtr {nullptr};
    Stub stub;
};

void ApplicationServiceImpTest::SetUp() {
    m_appPtr = std::make_unique<ApplicationServiceImp>();
}
void ApplicationServiceImpTest::TearDown() {}
void ApplicationServiceImpTest::SetUpTestCase() {}
void ApplicationServiceImpTest::TearDownTestCase() {}


static void FuncStub()
{
    INFOLOG("FuncStub");
}

static void* GetObj_Stub(void *ob, const std::string& name)
{
    return (void*)FuncStub;
}
/*
 * 用例名称: 测试thrift同步接口通过dlopen加载动态库函数
 * 前置条件：未加载动态库
 * check点：测试打开动态库失败后的返回以及抛出异常
 */
TEST_F(ApplicationServiceImpTest, DiscoverApplications)
{
    std::vector<Application> returnValue;
    std::string appType = "type123";
    m_appPtr->DiscoverApplications(returnValue, appType);
    EXPECT_EQ(returnValue.size(), 0);
}

TEST_F(ApplicationServiceImpTest, CheckApplication)
{
    ActionResult returnValue;
    ApplicationEnvironment appEnv;
    Application application;
    m_appPtr->CheckApplication(returnValue, appEnv, application);
    EXPECT_EQ(returnValue.code, INNER_ERROR);
}

TEST_F(ApplicationServiceImpTest, ListApplicationResource)
{
    stub.set(Module::DlibDlsym, GetObj_Stub);
    std::vector<ApplicationResource> returnValue;
    ApplicationResource parentResource;
    Application application;
    ApplicationEnvironment appEnv;

    EXPECT_NO_THROW(m_appPtr->ListApplicationResource(returnValue, appEnv, application, parentResource));

    // test for param_check type CHINESE_ENGLISH_NAME
    appEnv.name = "中文!@#$%z";
    EXPECT_THROW(m_appPtr->ListApplicationResource(returnValue, appEnv, application, parentResource), AppProtect::AppProtectPluginException);
    appEnv.name = "name";
    EXPECT_NO_THROW(m_appPtr->ListApplicationResource(returnValue, appEnv, application, parentResource));

    // test for param_check type COMMON_ID
    ApplicationEnvironment appEnvId;
    appEnvId.id = "123$%";
    EXPECT_THROW(m_appPtr->ListApplicationResource(returnValue, appEnvId, application, parentResource), AppProtect::AppProtectPluginException);
    appEnvId.id = "123";
    EXPECT_NO_THROW(m_appPtr->ListApplicationResource(returnValue, appEnvId, application, parentResource));

    // test for param_check type IPV4_OR_IPV6_STR
    ApplicationEnvironment appEnvEndpoint;
    appEnvEndpoint.endpoint = "123好";
    EXPECT_THROW(m_appPtr->ListApplicationResource(returnValue, appEnvEndpoint, application, parentResource), AppProtect::AppProtectPluginException);
    appEnvEndpoint.endpoint = "123";
    EXPECT_NO_THROW(m_appPtr->ListApplicationResource(returnValue, appEnvEndpoint, application, parentResource));


    // test for param_check type COMMON_PORT
    ApplicationEnvironment appEnvPort;
    appEnvPort.port = 65536;
    EXPECT_THROW(m_appPtr->ListApplicationResource(returnValue, appEnvPort, application, parentResource), AppProtect::AppProtectPluginException);
    appEnvPort.port = 65534;
    EXPECT_NO_THROW(m_appPtr->ListApplicationResource(returnValue, appEnvPort, application, parentResource));

    stub.reset(Module::DlibDlsym);
}

TEST_F(ApplicationServiceImpTest, ListApplicationResourceV2)
{
    ResourceResultByPage page;
    ListResourceRequest request;
    EXPECT_THROW(m_appPtr->ListApplicationResourceV2(page, request), AppProtectFrameworkException);
}

TEST_F(ApplicationServiceImpTest, DiscoverHostCluster)
{
    ApplicationEnvironment returnEnv;
    ApplicationEnvironment appEnv;
    try {
        m_appPtr->DiscoverHostCluster(returnEnv, appEnv);
    }
    catch(...) {
        ERRLOG("Test catch throw");
    }
    EXPECT_EQ(returnEnv.id, "");
}

TEST_F(ApplicationServiceImpTest, DiscoverAppCluster)
{
    stub.set(Module::DlibDlsym, GetObj_Stub);

    ApplicationEnvironment returnEnv;
    ApplicationEnvironment appEnv;
    Application application;

    // test for param_check type COMMON_ENUM_VALUE
    Authentication auth;
    auth.authType = AuthType::type::APP_PASSWORD;
    application.auth = auth;
    EXPECT_NO_THROW(m_appPtr->DiscoverAppCluster(returnEnv, appEnv, application));

    // test for param_check type COMMON_ANY_CHAR
    Authentication auth2;
    auth2.extendInfo = "中文";
    application.auth = auth2;
    EXPECT_NO_THROW(m_appPtr->DiscoverAppCluster(returnEnv, appEnv, application));

    stub.reset(Module::DlibDlsym);
}

TEST_F(ApplicationServiceImpTest, ListApplicationConfig)
{
    std::map<std::string, std::string> resources;
    std::string script;
    try{
        m_appPtr->ListApplicationConfig(resources, script);
    }
    catch(...) {
        ERRLOG("Test catch throw");
    }
    EXPECT_EQ(resources.size(), 0);
}
