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
#include <stdio.h>
#include <iostream>
#include "stub.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "gmock/gmock-actions.h"
#include "secodeFuzz.h"
#include "file_resource/AppService.h"
#include "file_resource/PosixHandler.h"
using namespace FilePlugin;
using namespace AppProtect;
class FuzzListApplicationResource : public testing::Test{
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    Stub m_stub;
};

void FuzzListApplicationResource::SetUp() {}

void FuzzListApplicationResource::TearDown() {}

void FuzzListApplicationResource::SetUpTestCase() {}

void FuzzListApplicationResource::TearDownTestCase() {}

TEST_F(FuzzListApplicationResource, listPosixResource_OK)
{
    DT_FUZZ_START(0, 10,(char*)"ListApplicationResourceV2DtFuzzTEST" ,0)
    {
        AppProtect::ListResourceRequest request;
        AppProtect::Application application;
        application.subType = "Fileset";
        application.name = "netapp_mdd_cifs";
        application.id = "a6014064-9fec-3dea-bade-de98a3336f13";
        application.extendInfo =  "{\"protocol\":\"Linux\",\"filters\":\"\",\"ip\":\"8.40.145.46\""
                                    " ,\"fileType\":\"native\", \"domainName\":\"keber.host.com\",\"encryption\": \"0\""
                                            ",\"kerberosId\":\"0\",\"shareMode\":\"0\",\"directory\":\"/home\"}";
        application.auth.authType = AuthType::type::APP_PASSWORD;
        char *authkey = DT_SetGetString(&g_Element[0], 11, 20,(char *)"test_admin");
        application.auth.authkey = authkey;
        application.auth.authPwd = "Huawei@123";
        application.auth.extendInfo =  "{\"krb5Conf\":\"krb5ConfValue\",\"password\":\"123456\""
                                    ",\"keytab\":\"keytab_value\", \"configMode\":\"0\",\"shareMode\":\"0\"}";

        request.applications.push_back(application);
        request.condition.pageNo = 0;
        request.condition.pageSize = 1;
        AppProtect::ResourceResultByPage returnValue;
        AppServiceExport::ListApplicationResourceV2(returnValue, request);
        EXPECT_EQ(1, returnValue.items.size());
    }
    DT_FUZZ_END()
    return;
}

TEST_F(FuzzListApplicationResource, listPosix_protocol_OK)
{
    DT_FUZZ_START(0, 10,(char*)"ListApplicationProtocolDtFuzzTEST" ,0)
    {
        AppProtect::ListResourceRequest request;
        AppProtect::Application application;
        application.subType = "Fileset";
        application.name = "netapp_mdd_cifs";

        application.id = "a6014064-9fec-3dea-bade-de98a3336f13";
        char *protocol = DT_SetGetString(&g_Element[0], 6, 10,(char *)"12x45");
        std::string protocolValue = protocol;
        std::replace(protocolValue.begin(), protocolValue.end(), '\\', '/');
        application.extendInfo.append("{\"protocol\":")
              .append("\"").append(protocolValue).append("\"")
              .append(", \"filters\":\"\", \"ip\":\"8.40.145.46\" ")
              .append(" ,\"fileType\":\"native\", \"domainName\":\"keber.host.com\",\"encryption\": \"0\"")
              .append(",\"kerberosId\":\"0\",\"shareMode\":\"0\",\"directory\":\"/home\"}");
        application.auth.authPwd = "Huawei@123";
        application.auth.extendInfo =  "{\"krb5Conf\":\"krb5ConfValue\",\"password\":\"123456\""
                                    ",\"keytab\":\"keytab_value\", \"configMode\":\"0\",\"shareMode\":\"0\"}";
        request.applications.push_back(application);
        request.condition.pageNo = 0;
        request.condition.pageSize = 1;
        AppProtect::ResourceResultByPage returnValue;
        EXPECT_THROW(AppServiceExport::ListApplicationResourceV2(returnValue, request), AppProtect::AppProtectPluginException);
    }
    DT_FUZZ_END()
    return;
}
TEST_F(FuzzListApplicationResource, DiscoverApplications_OK)
{
    DT_FUZZ_START(0, 10,(char*)"DiscoverApplicationsDtFuzzTEST" ,0)
    {
        char *appTypeKey = DT_SetGetString(&g_Element[0], 8, 20,(char *)"Fileset");
        std::string appType = appTypeKey;
        std::vector<Application> returnValue;
        AppServiceExport::DiscoverApplications(returnValue, appType);
    }
    DT_FUZZ_END()
    return;
}

TEST_F(FuzzListApplicationResource, CheckApplication_OK)
{
    DT_FUZZ_START(0, 10,(char*)"CheckApplicationDtFuzzTEST" ,0)
    {
        ActionResult returnValue;
        ApplicationEnvironment appEnv;
        Application application;
        AppServiceExport::CheckApplication(returnValue, appEnv, application);
    }
    DT_FUZZ_END()
    return;
}

TEST_F(FuzzListApplicationResource, ListApplication_OK)
{
    DT_FUZZ_START(0, 1,(char*)"ListApplicationDtFuzzTEST" ,0)
    {
        std::vector<ApplicationResource> returnValue;
        ApplicationEnvironment appEnv;
        Application application;
        ApplicationResource parentResource;
        AppServiceExport::ListApplicationResource(returnValue, appEnv, application, parentResource);
    }
    DT_FUZZ_END()
    return;
}
