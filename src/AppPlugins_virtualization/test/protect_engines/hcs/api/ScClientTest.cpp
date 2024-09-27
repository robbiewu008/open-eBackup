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
#include <common/JsonHelper.h>
#include "protect_engines/hcs/common/HcsCommonInfo.h"
#include "common/model/ResponseModelMock.h"
#include "common/model/ModelBaseMock.h"
#include "protect_engines/hcs/utils/IHttpResponseMock.h"
#include "protect_engines/hcs/utils/IHttpClientMock.h"
#include "protect_engines/hcs/api/sc/ScClient.h"
#include "protect_engines/hcs/utils/HCSTokenMgr.h"
#include "common/Structs.h"

using ::testing::_;
using ::testing::An;
using ::testing::AtLeast;
using testing::DoAll;
using testing::InitGoogleMock;
using ::testing::Return;
using ::testing::Sequence;
using ::testing::SetArgReferee;
using ::testing::Throw;
using ::testing::Invoke;;

using namespace VirtPlugin;
using namespace HcsPlugin;

namespace HDT_TEST {
class ScClientTest : public testing::Test {
protected:
    void SetUp() {}
    void TearDown() {}
};

std::string g_tokenStr_sc = "MIIEhwYJKoZIhvcNAQcCoIIEeDCCBHQCAQExDTALBglghkgBZQMEAgEwggLoBgkqhkiG9w0BBwGgggLZBIIC1XsidG9rZW4iOnsiZXhwaXJlc19hdCI6IjIwMjItMDctMjJUMDc6NTk6NTkuODY5MDAwWiIsIm1ldGhvZHMiOlsicGFzc3dvcmQiXSwiY2F0YWxvZyI6W10sInJvbGVzIjpbeyJuYW1lIjoidmRjX2FkbSIsImlkIjoiY2E3MWU3NzFiYWZjNDI5OTkwOThmMDg4YTc4NGM3NTEifSx7Im5hbWUiOiJ0YWdfYWRtIiwiaWQiOiJkNmJiNWRiZjc0YjQ0Yjk1YWFmMmU3MGJmYzcyNTE0ZiJ9LHsibmFtZSI6ImFwcHJvdl9hZG0iLCJpZCI6IjBiZDBmZjlhMWZkNDRiNzc5MzZlNWUzNzExODZhMzI1In0seyJuYW1lIjoidmRjX293bmVyIiwiaWQiOiI0Nzg4ZjYyMzhmZDM0MWNjYmZkOGQwYzQzMzg4YjdlZSJ9LHsibmFtZSI6InRlX2FkbWluIiwiaWQiOiJhYzgyMWRkN2EwZDI0ZDI2OGI4ZGE2MDg0ZmRlNmQ3OCJ9XSwicHJvamVjdCI6eyJkb21haW4iOnsibmFtZSI6Imh1YW5ncm9uZyIsImlkIjoiOTkwNzYzNjFiOTVmNDIyNmIxOGRiMDAwMTU1NWJkMDAifSwibmFtZSI6InNjLWNkLTFfdGVzdCIsImlkIjoiZTM4ZDIyN2VkY2NlNDYzMWJlMjBiZmE1YWFkNzEzMGIifSwiaXNzdWVkX2F0IjoiMjAyMi0wNy0yMVQwNzo1OTo1OS44NjkwMDBaIiwidXNlciI6eyJkb21haW4iOnsibmFtZSI6Imh1YW5ncm9uZyIsImlkIjoiOTkwNzYzNjFiOTVmNDIyNmIxOGRiMDAwMTU1NWJkMDAifSwibmFtZSI6Imh1YW5ncm9uZyIsImlkIjoiZDQyMTZiN2QzYmE2NGE0ZWI2M2RiMzdjMmI5MTIyMmMifX19MYIBcjCCAW4CAQEwSTA9MQswCQYDVQQGEwJDTjEPMA0GA1UEChMGSHVhd2VpMR0wGwYDVQQDExRIdWF3ZWkgSVQgUHJvZHVjdCBDQQIIFamRIbpBmrcwCwYJYIZIAWUDBAIBMA0GCSqGSIb3DQEBAQUABIIBAGkKLMyXHOFwT4nqe4Iue5g59bBMsIAhW-bhq0MIiJklULEo8RDH+hX5e8AQ44K1Dv2KKXSctXqZoIjW+SeRFxSQm8Ifp-mw18gDn6F+DZRE1ZS+CeecSG8BmXutAfhd9YJQ2xRcw4tbOy21OY-WrXXqIkyyAW1kZpv1yejMm6d6QHDanObsrH9aMJkv79l9tpu0lk4kXM4ohAaUSbVJm47iOiRN2BNxnsHa4bymXFOCIkUYLtA+z0-BXjJIiZjem6Uhtqt6P97Z7MzyuTSFMw0fl6BGswajprEqrVvJg7tB2WCstsff2SPedA86-ufA39TrGuu1kWhLJeUWGQTf2PI=";

std::string g_vdcListBody ="{\"total\": 3,     \"vdcs\": [         {             \"id\": \"99076361b95f4226b18db0001555bd00\",             \"name\": \"huangrong\",             \"tag\": \"vdc\",             \"description\": \"\",             \"upper_vdc_id\": \"-1\",             \"upper_vdc_name\": null,             \"top_vdc_id\": \"a8df1682-3c71-4391-9ad9-04cc6dca13bf\",             \"extra\": \"{\\\"manager\\\":\\\"\\\",\\\"phone\\\":\\\"\\\",\\\"email\\\":\\\"\\\"}\",             \"ecs_used\": 0.0,             \"evs_used\": 0.0,             \"project_count\": 0,             \"enabled\": true,             \"domain_id\": \"99076361b95f4226b18db0001555bd00\",             \"level\": 0,             \"create_user_id\": \"da5b034745aa4db0a14fe57a86fb11f6\",             \"create_user_name\": \"bss_admin\",             \"create_at\": 1652175583000,             \"utc_create_at\": \"2022-05-10 09:39:43.0\",             \"domain_name\": \"huangrong\",             \"ldap_id\": null,             \"third_id\": null,             \"idp_name\": null,             \"third_type\": \"0\",             \"region_id\": null,             \"enterprise_id\": null,             \"az_id\": null,             \"enterprise_project_id\": null         },         {             \"id\": \"e283a3f9e0a64487934941674b4acd3a\",             \"name\": \"oceanprotect\",             \"tag\": \"vdc\",             \"description\": \"\",             \"upper_vdc_id\": \"-1\",             \"upper_vdc_name\": null,             \"top_vdc_id\": \"63a67934-909f-42b0-8363-82aba8c58ceb\",             \"extra\": \"{\\\"manager\\\":\\\"\\\",\\\"phone\\\":\\\"\\\",\\\"email\\\":\\\"\\\"}\",             \"ecs_used\": 0.0,             \"evs_used\": 0.0,             \"project_count\": 0,             \"enabled\": true,             \"domain_id\": \"e283a3f9e0a64487934941674b4acd3a\",             \"level\": 0,             \"create_user_id\": \"da5b034745aa4db0a14fe57a86fb11f6\",             \"create_user_name\": \"bss_admin\",             \"create_at\": 1654678668000,             \"utc_create_at\": \"2022-06-08 08:57:48.0\",             \"domain_name\": \"oceanprotect\",             \"ldap_id\": null,             \"third_id\": null,             \"idp_name\": null,             \"third_type\": \"0\",             \"region_id\": null,             \"enterprise_id\": null,             \"az_id\": null,             \"enterprise_project_id\": null         },         {             \"id\": \"d28507a101364296b2e04ad44ff19777\",             \"name\": \"test001\",             \"tag\": \"vdc\",             \"description\": \"\",             \"upper_vdc_id\": \"-1\",             \"upper_vdc_name\": null,             \"top_vdc_id\": \"0ca22258-dbd9-4fdd-b482-37ca21330289\",             \"extra\": \"{\\\"manager\\\":\\\"\\\",\\\"phone\\\":\\\"\\\",\\\"email\\\":\\\"\\\"}\",             \"ecs_used\": 0.0,             \"evs_used\": 0.0,             \"project_count\": 0,             \"enabled\": true,             \"domain_id\": \"d28507a101364296b2e04ad44ff19777\",             \"level\": 0,             \"create_user_id\": \"da5b034745aa4db0a14fe57a86fb11f6\",             \"create_user_name\": \"bss_admin\",             \"create_at\": 1654761100000,             \"utc_create_at\": \"2022-06-09 07:51:40.0\",             \"domain_name\": \"test001\",             \"ldap_id\": null,             \"third_id\": null,             \"idp_name\": null,             \"third_type\": \"0\",             \"region_id\": null,             \"enterprise_id\": null,             \"az_id\": null,             \"enterprise_project_id\": null         }     ] }";
std::string g_projectListBody = "{\"total\": 1,     \"projects\": [         {             \"id\": \"e38d227edcce4631be20bfa5aad7130b\",             \"name\": \"sc-cd-1_test\",             \"description\": \"\",             \"domain_id\": \"99076361b95f4226b18db0001555bd00\",             \"enabled\": true,             \"tenant_id\": \"a8df1682-3c71-4391-9ad9-04cc6dca13bf\",             \"is_shared\": false,             \"tenant_name\": \"huangrong\",             \"create_user_id\": \"d4216b7d3ba64a4eb63db37c2b91222c\",             \"create_user_name\": \"huangrong\",             \"regions\": [                 {                     \"region_id\": \"sc-cd-1\",                     \"region_name\": {                         \"zh_cn\": \"西南\",                         \"en_us\": \"西南\"                     },                     \"region_type\": null,                     \"region_status\": \"normal\"                 }             ]         }     ] }";
std::string g_projectDetailBody = "{\"project\": {         \"tenant_id\": \"a8df1682-3c71-4391-9ad9-04cc6dca13bf\",         \"create_user_id\": \"d4216b7d3ba64a4eb63db37c2b91222c\",         \"tenant_name\": \"huangrong\",         \"create_user_name\": \"huangrong\",         \"description\": \"\",         \"tenant_type\": \"vdc\",         \"enabled\": true,         \"domain_id\": \"99076361b95f4226b18db0001555bd00\",         \"contract_number\": null,         \"is_shared\": false,         \"name\": \"sc-cd-1_test\",         \"iam_project_name\": \"sc-cd-1_test\",         \"display_name\": \"sc-cd-1_test\",         \"id\": \"e38d227edcce4631be20bfa5aad7130b\",         \"owner_id\": null,         \"owner_name\": null,         \"region_name\": null,         \"regions\": [             {                 \"region_id\": \"sc-cd-1\",                 \"region_name\": {                     \"zh_cn\": \"西南\",                     \"en_us\": \"西南\"                 },                 \"region_type\": null,                 \"region_status\": \"normal\",                 \"cloud_infras\": [                     {                         \"cloud_infra_id\": \"FUSION_CLOUD_sc-cd-1\",                         \"cloud_infra_name\": \"OpenStack_sc-cd-1\",                         \"cloud_infra_status\": \"normal\",                         \"cloud_infra_type\": \"FUSION_CLOUD\",                         \"azs\": [                             {                                 \"az_id\": \"az0.dc0\",                                 \"az_name\": \"某某公司\",                                 \"az_status\": \"normal\"                             }                         ],                         \"quotas\": []                     }                 ]             }         ],         \"attachment_id\": null,         \"attachment_name\": null,         \"attachment_size\": 0,         \"is_support_hws_service\": true     } }";

bool Stub_GetToken_SC_Success(void *obj, ModelBase &model, std::string &tokenStr, std::string &endPoint)
{
    tokenStr = g_tokenStr_sc;
    endPoint = "";
    return true;
}

static std::shared_ptr<IHttpResponseMock> Stub_Common(std::string body)
{
    std::shared_ptr<IHttpResponseMock> httpRespone = std::make_shared<IHttpResponseMock>();
    EXPECT_CALL(*httpRespone, Success()).WillRepeatedly(Return(true));
    EXPECT_CALL(*httpRespone, GetStatusCode()).WillRepeatedly(Return(200));
    EXPECT_CALL(*httpRespone, GetErrCode()).WillRepeatedly(Return(0));
    EXPECT_CALL(*httpRespone, GetErrString()).WillRepeatedly(Return(""));
    EXPECT_CALL(*httpRespone, Busy()).WillRepeatedly(Return(false));
    EXPECT_CALL(*httpRespone, GetHttpStatusCode()).WillRepeatedly(Return(0));
    EXPECT_CALL(*httpRespone, GetHttpStatusDescribe()).WillRepeatedly(Return(""));
    std::set<std::string> GetHeadByNameReturn = {};
    EXPECT_CALL(*httpRespone, GetHeadByName(_)).WillRepeatedly(Return(GetHeadByNameReturn));
    EXPECT_CALL(*httpRespone, GetBody()).WillRepeatedly(Return(body));
    std::set<std::string> headerValue;
    headerValue.insert(g_tokenStr_sc);
    std::map<std::string, std::set<std::string> > getHeadersReturn = {{"X-Subject-Token", headerValue}};
    EXPECT_CALL(*httpRespone, GetHeaders()).WillRepeatedly(Return(getHeadersReturn));
    std::set<std::string> getCookiesReturn = {};
    EXPECT_CALL(*httpRespone, GetCookies()).WillRepeatedly(Return(getCookiesReturn));
    return httpRespone;
}

static Module::IHttpClient* Stub_Get_vdc_list_Success()
{
    IHttpClientMock* httpClient = new(std::nothrow) IHttpClientMock();
    EXPECT_CALL(*httpClient, SendRequest(_,_)).WillRepeatedly(Return(Stub_Common(g_vdcListBody)));
    return httpClient;
}

static Module::IHttpClient* Stub_Get_project_list_Success()
{
    IHttpClientMock* httpClient = new(std::nothrow) IHttpClientMock();
    EXPECT_CALL(*httpClient, SendRequest(_,_)).WillRepeatedly(Return(Stub_Common(g_projectListBody)));
    return httpClient;
}

static Module::IHttpClient* Stub_Get_project_detail_Success()
{
    IHttpClientMock* httpClient = new(std::nothrow) IHttpClientMock();
    EXPECT_CALL(*httpClient, SendRequest(_,_)).WillRepeatedly(Return(Stub_Common(g_projectDetailBody)));
    return httpClient;
}

/*
 * 测试用例： 查询vdc用户
 * 前置条件： http能够成功返回参数
 * CHECK点： http返回值和打桩值一致
 */
TEST_F(ScClientTest, GetVdcListSuccess)
{
    QueryVdcListRequest request;
    AuthObj auth;
    auth.name = "name";
    auth.passwd = "passwd";
    request.SetUserInfo(auth);
    request.SetScopeValue("domian");
    request.SetDomain("domain");
    request.SetEndpoint("demo.com");
    Stub stub;
    stub.set(ADDR(HCSTokenMgr, GetToken), Stub_GetToken_SC_Success);
    stub.set(ADDR(Module::IHttpClient, GetInstance), Stub_Get_vdc_list_Success);
    ScClient scClient;
    std::shared_ptr<QueryVdcListResponse> response = scClient.QueryVdcList(request);
    EXPECT_TRUE((response != nullptr));
    EXPECT_EQ(response->GetStatusCode(), 200);
    EXPECT_TRUE(response->Serial());
    VdcListDetail vdcListDetail = response->GetVdcListDetail();
    std::vector<std::string> vdcs = {"huangrong", "oceanprotect", "test001"};
    std::vector<std::string> vdcs_query;
    for (auto vdc : vdcListDetail.m_vdcs) {
        vdcs_query.push_back(vdc.m_name);
    }
    EXPECT_EQ(3, vdcListDetail.m_total);
    EXPECT_EQ(vdcs, vdcs_query);
    stub.reset(ADDR(Module::IHttpClient, GetInstance));
}

/*
 * 测试用例： 查询vdc用户，参数校验失败
 * 前置条件： http能够成功返回参数
 * CHECK点： 查询接口返回nullptr
 */
TEST_F(ScClientTest, GetVdcListFailed)
{
    QueryVdcListRequest request;
    Stub stub;
    stub.set(ADDR(Module::IHttpClient, GetInstance), Stub_Get_vdc_list_Success);
    ScClient scClient;
    std::shared_ptr<QueryVdcListResponse> response = scClient.QueryVdcList(request);
    EXPECT_EQ(response, nullptr);
    stub.reset(ADDR(Module::IHttpClient, GetInstance));
}

/*
 * 测试用例： 查询工程列表
 * 前置条件： http能够成功返回参数
 * CHECK点： http返回值和打桩值一致
 */
TEST_F(ScClientTest, GetProjectListSuccess)
{
    QueryResourceListRequest request;
    AuthObj auth;
    auth.name = "name";
    auth.passwd = "passwd";
    request.SetUserInfo(auth);
    request.SetScopeValue("domian_1");
    request.SetDomain("domain_1");
    request.SetEndpoint("demo.com");
    request.SetVdcManagerId("vdc_id");
    Stub stub;
    stub.set(ADDR(HCSTokenMgr, GetToken), Stub_GetToken_SC_Success);
    stub.set(ADDR(Module::IHttpClient, GetInstance), Stub_Get_project_list_Success);
    ScClient scClient;
    std::shared_ptr<QueryResourceListResponse> response = scClient.QueryResourceList(request);
    EXPECT_TRUE((response != nullptr));
    EXPECT_EQ(response->GetStatusCode(), 200);
    EXPECT_TRUE(response->Serial());
    ResourceListDetail resourceListDetail = response->GetResourceListDetail();
    EXPECT_EQ(1, resourceListDetail.m_total);
    EXPECT_EQ("sc-cd-1_test", resourceListDetail.m_projects[0].m_name);
    stub.reset(ADDR(Module::IHttpClient, GetInstance));
}

/*
 * 测试用例： 查询工程列表，参数校验失败
 * 前置条件： http能够成功返回参数
 * CHECK点： 查询接口返回nullptr
 */
TEST_F(ScClientTest, GetProjectListFailed)
{
    QueryResourceListRequest request;
    AuthObj auth;
    auth.name = "name";
    auth.passwd = "passwd";
    request.SetUserInfo(auth);
    request.SetScopeValue("domian_1");
    request.SetDomain("domain_1");
    request.SetEndpoint("demo.com");
    request.m_vdcManagerIdIsSet = false;
    Stub stub;
    stub.set(ADDR(Module::IHttpClient, GetInstance), Stub_Get_project_list_Success);
    ScClient scClient;
    std::shared_ptr<QueryResourceListResponse> response = scClient.QueryResourceList(request);
    EXPECT_TRUE((response == nullptr));
    stub.reset(ADDR(Module::IHttpClient, GetInstance));
}

TEST_F(ScClientTest, GetProjectDetailSuccess)
{
    QueryProjectDetailRequest request;
    AuthObj auth;
    auth.name = "name";
    auth.passwd = "passwd";
    request.SetUserInfo(auth);
    request.SetScopeValue("domian_2");
    request.SetDomain("domain_2");
    request.SetEndpoint("demo.com");
    request.SetProjectId("projectId");
    Stub stub;
    stub.set(ADDR(HCSTokenMgr, GetToken), Stub_GetToken_SC_Success);
    stub.set(ADDR(Module::IHttpClient, GetInstance), Stub_Get_project_detail_Success);
    ScClient scClient;
    std::shared_ptr<QueryProjectDetailResponse> response = scClient.QueryProjectDetail(request);
    EXPECT_TRUE((response != nullptr));
    EXPECT_EQ(response->GetStatusCode(), 200);
    EXPECT_TRUE(response->Serial());
    ProjectDetail projectDetail = response->GetProjectDetail();
    EXPECT_EQ("sc-cd-1_test", projectDetail.m_projectDetailInfo.m_name);
    stub.reset(ADDR(Module::IHttpClient, GetInstance));
}

/*
 * 测试用例： 查询工程详情，参数校验失败
 * 前置条件： http能够成功返回参数
 * CHECK点： 查询接口返回nullptr
 */
TEST_F(ScClientTest, GetProjectDetailFailed)
{
    QueryProjectDetailRequest request;
    AuthObj auth;
    auth.name = "name";
    auth.passwd = "passwd";
    request.SetUserInfo(auth);
    request.SetScopeValue("domian_2");
    request.SetDomain("domain_2");
    request.SetEndpoint("demo.com");
    request.m_projectIdIsSet = false;
    Stub stub;
    stub.set(ADDR(Module::IHttpClient, GetInstance), Stub_Get_project_detail_Success);
    ScClient scClient;
    std::shared_ptr<QueryProjectDetailResponse> response = scClient.QueryProjectDetail(request);
    EXPECT_TRUE((response == nullptr));
    stub.reset(ADDR(Module::IHttpClient, GetInstance));
}
}