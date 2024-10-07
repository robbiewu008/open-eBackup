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
#include "file_resource/AppService.h"
#include "file_resource/PosixHandler.h"
#include "application/ApplicationServiceDataType.h"

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
using namespace AppServiceExport;

class AppServiceTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    Stub stub;
};


void AppServiceTest::SetUp()
{
}

void AppServiceTest::TearDown()
{
}

void AppServiceTest::SetUpTestCase()
{
}

void AppServiceTest::TearDownTestCase()
{
}

static bool Stub_JsonStringToStruct_FALSE(void* obj)
{
    return false;
}

static bool Stub_JsonStringToStruct_TRUE(void* obj)
{
    return true;
}

static std::string Stub_NormalizeDirPath(std::string path)
{
    std::string str = "/home/NotExist";
    return str;
}

static bool Funciton_Failed()
{
    return false;
}
TEST_F(AppServiceTest, ListApplicationResourceTest)
{
    std::vector<AppProtect::ApplicationResource> returnValue;
    const AppProtect::ApplicationEnvironment appEnv;
    AppProtect::Application application;
    const AppProtect::ApplicationResource parentResource;
    AppProtect::ListResourceRequest request;
    request.condition.pageSize = 1;
    AppServiceExport::ListApplicationResource(returnValue, appEnv, application, parentResource);
    EXPECT_EQ(1, request.condition.pageSize);

}

static void Function_void_Stub(void* obj,std::vector<Application>& returnValue, const std::string& appType)
{
    return;
}

TEST_F(AppServiceTest, DiscoverApplicationsTest)
{
    stub.set(ADDR(AppServiceExport,DiscoverApplications), Function_void_Stub);
    AppProtect::ListResourceRequest request;
    request.condition.pageNo = 0;

    EXPECT_EQ(0, request.condition.pageNo);
    stub.reset(ADDR(AppServiceExport,DiscoverApplications));
    
}

TEST_F(AppServiceTest, ConstructResourceParamTest)
{
    AppProtect::ListResourceRequest request;
    FilePlugin::ListResourceParam listResourceParam;
    listResourceParam.nasShareAuthInfo.authType = "5";
    AppProtect::Application application;
    application.name = "netapp_mdd_cifs";
    application.id = "a6014064-9fec-3dea-bade-de98a3336f13";

    request.applications.push_back(application);
    request.condition.pageNo = 0;
    request.condition.pageSize = 1;
    application.auth.authType = AuthType::type::APP_PASSWORD;
    application.auth.authkey = "test_admin";
    application.auth.authPwd = "Huawei@123";
    application.auth.extendInfo =  "{\"krb5Conf\":\"krb5ConfValue\",\"password\":\"123456\""
                                ",\"keytab\":\"keytab_value\", \"configMode\":\"0\",\"shareMode\":\"0\"}";
    application.extendInfo =  "{\"protocol\":\"Linux\",\"filters\":\"\",\"ip\":\"8.40.145.46\""
                                " ,\"fileType\":\"native\", \"domainName\":\"keber.host.com\",\"encryption\": \"0\""
                                        ",\"kerberosId\":\"0\",\"shareMode\":\"0\",\"directory\":\"/home\"}";
    bool ret = AppServiceExport::ConstructResourceParam(request, listResourceParam);
    EXPECT_EQ(ret, false);

    stub.set((bool(*)(const std::string& , ResourceExendInfo&))
    Module::JsonHelper::JsonStringToStruct, Stub_JsonStringToStruct_TRUE);
    stub.set((bool(*)(const std::string& , NasAuthExtentInfo&))
    Module::JsonHelper::JsonStringToStruct, Stub_JsonStringToStruct_FALSE);
    ret = AppServiceExport::ConstructResourceParam(request, listResourceParam);
    EXPECT_EQ(ret, true);

}

// TEST_F(AppServiceTest, List_Native_Posix_Success)
// {
    // AppProtect::ListResourceRequest request;
    // AppProtect::Application application;
    // application.subType = "Fileset";
    // application.name = "netapp_mdd_cifs";
    // application.id = "a6014064-9fec-3dea-bade-de98a3336f13";
    // application.extendInfo =  "{\"protocol\":\"Linux\",\"filters\":\"\",\"ip\":\"8.40.145.46\""
    //                             " ,\"fileType\":\"native\", \"domainName\":\"keber.host.com\",\"encryption\": \"0\""
    //                                     ",\"kerberosId\":\"0\",\"shareMode\":\"0\",\"directory\":\"/home\"}";
    // application.auth.authType = AuthType::type::APP_PASSWORD;
    // application.auth.authkey = "test_admin";
    // application.auth.authPwd = "Huawei@123";
    // application.auth.extendInfo =  "{\"krb5Conf\":\"krb5ConfValue\",\"password\":\"123456\""
    //                             ",\"keytab\":\"keytab_value\", \"configMode\":\"0\",\"shareMode\":\"0\"}";

    // request.applications.push_back(application);
    // request.condition.pageNo = 0;
    // request.condition.pageSize = 1;
    // AppProtect::ResourceResultByPage returnValue;
    // AppServiceExport::ListApplicationResourceV2(returnValue, request);
    // EXPECT_EQ(1, returnValue.items.size());

    // stub.set(ADDR(AppServiceExport,ConstructResourceParam), Funciton_Failed);
    // AppServiceExport::ListApplicationResourceV2(returnValue, request);
    // EXPECT_EQ(1, returnValue.items.size());
// }

TEST_F(AppServiceTest, List_Native_Not_Exist_Success)
{
    AppProtect::ListResourceRequest request;
    AppProtect::Application application;
    application.subType = "Fileset";
    application.id = "a6014064-9fec-3dea-bade-de98a3336f13";
    application.extendInfo =  "{\"protocol\":\"Linux\""" ,\"fileType\":\"native\", \"domainName\":\"keber.host.com\",\"encryption\": \"0\""
                                        ",\"kerberosId\":\"0\",\"shareMode\":\"0\",\"directory\":\"/home/NotEixtst\"}";
    application.auth.authType = AuthType::type::APP_PASSWORD;
    application.auth.extendInfo =  "{\"krb5Conf\":\"krb5ConfValue\",\"password\":\"123456\""
                                ",\"keytab\":\"keytab_value\", \"configMode\":\"0\",\"shareMode\":\"0\"}";

    request.applications.push_back(application);
    request.condition.pageNo = 0;
    request.condition.pageSize = 100;
    AppProtect::ResourceResultByPage returnValue;
    EXPECT_THROW(AppServiceExport::ListApplicationResourceV2(returnValue, request), AppProtect::AppProtectPluginException);
    request.applications.clear();
    application.extendInfo =  "{\"protocol\":\"UnLinux\""" ,\"fileType\":\"native\", \"domainName\":\"keber.host.com\",\"encryption\": \"0\""
                                        ",\"kerberosId\":\"0\",\"shareMode\":\"0\",\"directory\":\"/home/NotEixtst\"}";
    request.applications.push_back(application);
    EXPECT_THROW(AppServiceExport::ListApplicationResourceV2(returnValue, request), AppProtect::AppProtectPluginException);
}

TEST_F(AppServiceTest, List_Native_NotExist_File_OpenDir_Success)
{
    stub.set(ADDR(PosixHandler, NormalizeDirectoryPath), Stub_NormalizeDirPath);
    AppProtect::ListResourceRequest request;
    AppProtect::Application application;
    application.subType = "Fileset";
    application.id = "a6014064-9fec-3dea-bade-de98a3336f13";
    application.extendInfo =  "{\"protocol\":\"Linux\""" ,\"fileType\":\"native\", \"domainName\":\"keber.host.com\",\"encryption\": \"0\""
                                        ",\"kerberosId\":\"0\",\"shareMode\":\"0\",\"directory\":\"/home\"}";
    application.auth.authType = AuthType::type::APP_PASSWORD;
    application.auth.extendInfo =  "{\"krb5Conf\":\"krb5ConfValue\",\"password\":\"123456\""
                                ",\"keytab\":\"keytab_value\", \"configMode\":\"0\",\"shareMode\":\"0\"}";

    request.applications.push_back(application);
    request.condition.pageNo = 0;
    request.condition.pageSize = 100;
    AppProtect::ResourceResultByPage returnValue;
    EXPECT_THROW(AppServiceExport::ListApplicationResourceV2(returnValue, request), AppProtect::AppProtectPluginException);
    stub.reset(ADDR(PosixHandler, NormalizeDirectoryPath));
}


TEST_F(AppServiceTest, CheckApplication)
{
    ActionResult returnValue;
    ApplicationEnvironment appEnv;
    Application application;

    EXPECT_NO_THROW(CheckApplication(returnValue, appEnv, application));
}


TEST_F(AppServiceTest, DiscoverApplications)
{
    std::vector<Application> returnValue;
    std::string appType;

    EXPECT_NO_THROW(DiscoverApplications(returnValue, appType));
}

TEST_F(AppServiceTest, ListVolumeResource)
{
    FilePlugin::ListResourceParam listResourceParam {};
    FilePlugin::FileResourceInfo resourceInfo {};
    PosixHandler handler;
    EXPECT_NO_THROW(handler.ListVolumeResource(resourceInfo, listResourceParam));
}
