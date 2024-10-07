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
#include "protect_engines/hcs/api/iam/IamClient.h"
#include "common/Structs.h"
#include "common/token_mgr/TokenDetail.h"
#include "protect_engines/hcs/common/HcsHttpStatus.h"
#include "common/httpclient/HttpClient.h"

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
class IamClientTest : public testing::Test {
protected:
    void SetUp() {}
    void TearDown() {}
};

std::string g_tokenStr = "MIIEhwYJKoZIhvcNAQcCoIIEeDCCBHQCAQExDTALBglghkgBZQMEAgEwggLoBgkqhkiG9w0BBwGgggLZBIIC1XsidG9rZW4iOnsiZXhwaXJlc19hdCI6IjIwMjItMDctMjJUMDc6NTk6NTkuODY5MDAwWiIsIm1ldGhvZHMiOlsicGFzc3dvcmQiXSwiY2F0YWxvZyI6W10sInJvbGVzIjpbeyJuYW1lIjoidmRjX2FkbSIsImlkIjoiY2E3MWU3NzFiYWZjNDI5OTkwOThmMDg4YTc4NGM3NTEifSx7Im5hbWUiOiJ0YWdfYWRtIiwiaWQiOiJkNmJiNWRiZjc0YjQ0Yjk1YWFmMmU3MGJmYzcyNTE0ZiJ9LHsibmFtZSI6ImFwcHJvdl9hZG0iLCJpZCI6IjBiZDBmZjlhMWZkNDRiNzc5MzZlNWUzNzExODZhMzI1In0seyJuYW1lIjoidmRjX293bmVyIiwiaWQiOiI0Nzg4ZjYyMzhmZDM0MWNjYmZkOGQwYzQzMzg4YjdlZSJ9LHsibmFtZSI6InRlX2FkbWluIiwiaWQiOiJhYzgyMWRkN2EwZDI0ZDI2OGI4ZGE2MDg0ZmRlNmQ3OCJ9XSwicHJvamVjdCI6eyJkb21haW4iOnsibmFtZSI6Imh1YW5ncm9uZyIsImlkIjoiOTkwNzYzNjFiOTVmNDIyNmIxOGRiMDAwMTU1NWJkMDAifSwibmFtZSI6InNjLWNkLTFfdGVzdCIsImlkIjoiZTM4ZDIyN2VkY2NlNDYzMWJlMjBiZmE1YWFkNzEzMGIifSwiaXNzdWVkX2F0IjoiMjAyMi0wNy0yMVQwNzo1OTo1OS44NjkwMDBaIiwidXNlciI6eyJkb21haW4iOnsibmFtZSI6Imh1YW5ncm9uZyIsImlkIjoiOTkwNzYzNjFiOTVmNDIyNmIxOGRiMDAwMTU1NWJkMDAifSwibmFtZSI6Imh1YW5ncm9uZyIsImlkIjoiZDQyMTZiN2QzYmE2NGE0ZWI2M2RiMzdjMmI5MTIyMmMifX19MYIBcjCCAW4CAQEwSTA9MQswCQYDVQQGEwJDTjEPMA0GA1UEChMGSHVhd2VpMR0wGwYDVQQDExRIdWF3ZWkgSVQgUHJvZHVjdCBDQQIIFamRIbpBmrcwCwYJYIZIAWUDBAIBMA0GCSqGSIb3DQEBAQUABIIBAGkKLMyXHOFwT4nqe4Iue5g59bBMsIAhW-bhq0MIiJklULEo8RDH+hX5e8AQ44K1Dv2KKXSctXqZoIjW+SeRFxSQm8Ifp-mw18gDn6F+DZRE1ZS+CeecSG8BmXutAfhd9YJQ2xRcw4tbOy21OY-WrXXqIkyyAW1kZpv1yejMm6d6QHDanObsrH9aMJkv79l9tpu0lk4kXM4ohAaUSbVJm47iOiRN2BNxnsHa4bymXFOCIkUYLtA+z0-BXjJIiZjem6Uhtqt6P97Z7MzyuTSFMw0fl6BGswajprEqrVvJg7tB2WCstsff2SPedA86-ufA39TrGuu1kWhLJeUWGQTf2PI=";

std::string g_tokenBody = "{\"token\": {         \"expires_at\": \"2022-07-22T07:59:59.869000Z\",         \"methods\": [\"password\"],         \"catalog\": [             {                 \"endpoints\": [                     {                         \"region_id\": \"sc-cd-1\",                         \"id\": \"c8476fa3de214d42b99cdc8103b36a5e\",                         \"region\": \"sc-cd-1\",                         \"interface\": \"public\",                         \"url\": \"https://evs.sc-cd-1.demo.com/v2/e38d227edcce4631be20bfa5aad7130b\"                     }                 ],                 \"name\": \"evs\",                 \"id\": \"2cd3de7abbeb40fa859d73ab6a8282f5\",                 \"type\": \"volume\"             },             {                 \"endpoints\": [                     {                         \"region_id\": \"sc-cd-1\",                         \"id\": \"c1bf27b0c6644301971643034e063047\",                         \"region\": \"sc-cd-1\",                         \"interface\": \"internal\",                         \"url\": \"https://iam-cache-proxy.sc-cd-1.demo.com:26335\"                     },                     {                         \"region_id\": \"sc-cd-1\",                         \"id\": \"851c1913e6bc4c90ac3b948c7d20e665\",                         \"region\": \"sc-cd-1\",                         \"interface\": \"public\",                         \"url\": \"https://iam-apigateway-proxy.sc-cd-1.demo.com\"                     }                 ],                 \"name\": \"iam\",                 \"id\": \"4b47e15c6c0c4d02b1b6ff75a003e6c6\",                 \"type\": \"iam\"             },             {                 \"endpoints\": [                     {                         \"region_id\": \"sc-cd-1\",                         \"id\": \"c4e4278a31014b6282d9a3403214b540\",                         \"region\": \"sc-cd-1\",                         \"interface\": \"public\",                         \"url\": \"https://ecs.sc-cd-1.demo.com/v2/e38d227edcce4631be20bfa5aad7130b\"                     }                 ],                 \"name\": \"ecs\",                 \"id\": \"b1efa676dfad47609c7b9a6a6fe2c861\",                 \"type\": \"compute\"             },             {                 \"endpoints\": [                     {                         \"region_id\": \"sc-cd-1\",                         \"id\": \"cd044271e54348c0b4f1cb14211b317b\",                         \"region\": \"sc-cd-1\",                         \"interface\": \"internal\",                         \"url\": \"https://sc.demo.com:26335\"                     },                     {                         \"region_id\": \"sc-cd-1\",\                         \"id\": \"75b86267698a4cb3a34e2a1c555ee77f\",                         \"region\": \"sc-cd-1\",                         \"interface\": \"public\",                         \"url\": \"https://sc.demo.com\"                     }                 ],                 \"name\": \"sc\",                 \"id\": \"b921c288b2314f5fa13bd9342823e6d9\",                 \"type\": \"sc\"             },         ],         \"roles\": [             {                 \"name\": \"vdc_adm\",                 \"id\": \"ca71e771bafc42999098f088a784c751\"             },             {                 \"name\": \"tag_adm\",                 \"id\": \"d6bb5dbf74b44b95aaf2e70bfc72514f\"             },         ],         \"project\": {             \"domain\": {                 \"name\": \"huangrong\",                 \"id\": \"99076361b95f4226b18db0001555bd00\"             },             \"name\": \"sc-cd-1_test\",             \"id\": \"e38d227edcce4631be20bfa5aad7130b\"         },         \"issued_at\": \"2022-07-21T07:59:59.869000Z\",         \"user\": {             \"domain\": {                 \"name\": \"huangrong\",                 \"id\": \"99076361b95f4226b18db0001555bd00\"             },             \"name\": \"huangrong\",             \"id\": \"d4216b7d3ba64a4eb63db37c2b91222c\"         }     } }";

std::string g_adminProjectStr = "{\"projects\":[{\"id\":\"138152f07fc649278939c9eabf43abb2\",\"name\":\"MOS\",\"domain_id\":\"f79ca95d4c744c2498d56e131d332c11\",\"enabled\":true,\"parent_id\":\"f79ca95d4c744c2498d56e131d332c11\",\"is_domain\":false,},{\"id\":\"b0b1712fd6aa4aa1957a3e49ff8260e6\",\"name\":\"mo_bss_project\",\"domain_id\":\"f79ca95d4c744c2498d56e131d332c11\",\"enabled\":true,\"parent_id\":\"f79ca95d4c744c2498d56e131d332c11\",\"is_domain\":false,}]}";

static Module::IHttpClient* Stub_Get_Token_Success()
{
    std::shared_ptr<IHttpResponseMock> httpRespone = std::make_shared<IHttpResponseMock>();
    EXPECT_CALL(*httpRespone, Success()).WillRepeatedly(Return(true));
    EXPECT_CALL(*httpRespone, GetStatusCode()).WillRepeatedly(Return(201));
    EXPECT_CALL(*httpRespone, GetErrCode()).WillRepeatedly(Return(0));
    EXPECT_CALL(*httpRespone, GetErrString()).WillRepeatedly(Return(""));
    EXPECT_CALL(*httpRespone, Busy()).WillRepeatedly(Return(false));
    EXPECT_CALL(*httpRespone, GetHttpStatusCode()).WillRepeatedly(Return(0));
    EXPECT_CALL(*httpRespone, GetHttpStatusDescribe()).WillRepeatedly(Return(""));
    std::set<std::string> GetHeadByNameReturn = {};
    EXPECT_CALL(*httpRespone, GetHeadByName(_)).WillRepeatedly(Return(GetHeadByNameReturn));
    EXPECT_CALL(*httpRespone, GetBody()).WillRepeatedly(Return(g_tokenBody));
    std::set<std::string> headerValue;
    headerValue.insert(g_tokenStr);
    std::map<std::string, std::set<std::string> > getHeadersReturn = {{"X-Subject-Token", headerValue}};
    EXPECT_CALL(*httpRespone, GetHeaders()).WillRepeatedly(Return(getHeadersReturn));
    std::set<std::string> getCookiesReturn = {};
    EXPECT_CALL(*httpRespone, GetCookies()).WillRepeatedly(Return(getCookiesReturn));

    IHttpClientMock* httpClient = new(std::nothrow) IHttpClientMock();
    EXPECT_CALL(*httpClient, SendRequest(_,_)).WillRepeatedly(Return(httpRespone));
    return httpClient;
}

static Module::IHttpClient* Stub_Get_Token_Failed()
{
    IHttpClientMock* httpClient = new(std::nothrow) IHttpClientMock();
    EXPECT_CALL(*httpClient, SendRequest(_,_)).WillRepeatedly(Return(nullptr));
    EXPECT_CALL(*httpClient, SendRequest(_,_)).WillRepeatedly(Return(nullptr));
    return httpClient;
}

static int32_t Stub_callApi_Failed(VirtPlugin::RequestInfo &requestInfo, std::shared_ptr<ResponseModel> response, ModelBase &model)
{
    return FAILED;
}

/*
 * 测试用例： token查询成功
 * 前置条件： 对返回值进行打桩
 * CHECK点： 判断response中的值是否和返回值一致
 */
TEST_F(IamClientTest, GetTokenSuccess_project)
{
    GetTokenRequest getTokenRequeset;
    getTokenRequeset.SetTokenType(Scope::PROJECT);
    AuthObj auth;
    auth.name = "name";
    auth.passwd = "passwd";
    getTokenRequeset.SetUserInfo(auth);
    getTokenRequeset.SetScopeValue("e38d227edcce4631be20bfa5aad7130b");
    getTokenRequeset.SetDomain("domain");
    getTokenRequeset.SetEndpoint("demo.com");
    getTokenRequeset.SetRegion("sc-cd-1");
    Stub stub;
    stub.set(ADDR(Module::IHttpClient, GetInstance), Stub_Get_Token_Success);
    IamClient iamClient;
    std::shared_ptr<GetTokenResponse> response = iamClient.GetToken(getTokenRequeset);
    EXPECT_TRUE((response != nullptr));
    EXPECT_EQ(response->GetStatusCode(), 201);
    EXPECT_TRUE(response->Serial());
    TokenDetail tokenDetail = response->GetTokenDetail();
    EXPECT_EQ(tokenDetail.m_token.m_expiresAt, "2022-07-22T07:59:59.869000Z");
    auto headers = response->GetHeaders();
    auto it_head = headers.find("X-Subject-Token");
    EXPECT_FALSE((it_head == headers.end()));
    std::string tokenStr = *(it_head->second.begin());
    EXPECT_EQ(tokenStr, g_tokenStr);
    stub.reset(ADDR(Module::IHttpClient, GetInstance));
}

/*
 * 测试用例： token查询失败
 * 前置条件： http请求发送失败
 * CHECK点： 判断response中的值是否和返回值一致
 */
TEST_F(IamClientTest, GetTokenFailed)
{
    GetTokenRequest getTokenRequeset;
    getTokenRequeset.SetTokenType(Scope::PROJECT);
    AuthObj auth;
    auth.name = "name";
    auth.passwd = "passwd";
    getTokenRequeset.SetUserInfo(auth);
    getTokenRequeset.SetScopeValue("e38d227edcce4631be20bfa5aad7130b");
    getTokenRequeset.SetDomain("domain");
    getTokenRequeset.SetEndpoint("demo.com");
    getTokenRequeset.SetRegion("sc-cd-1");
    Stub stub;
    stub.set(ADDR(RestClient, CallApi), Stub_callApi_Failed);
    IamClient iamClient;
    std::shared_ptr<GetTokenResponse> response = iamClient.GetToken(getTokenRequeset);
    EXPECT_FALSE(response->Success());
    stub.reset(ADDR(Module::IHttpClient, GetInstance));
}

/*
 * 测试用例： token查询成功
 * 前置条件： 对返回值进行打桩
 * CHECK点： 判断response中的值是否和返回值一致
 */
TEST_F(IamClientTest, GetTokenSuccess_domain)
{
    GetTokenRequest getTokenRequeset;
    getTokenRequeset.SetTokenType(Scope::USER_DOMAIN);
    AuthObj auth;
    auth.name = "name";
    auth.passwd = "passwd";
    getTokenRequeset.SetUserInfo(auth);
    getTokenRequeset.SetScopeValue("domain");
    getTokenRequeset.SetDomain("domain");
    getTokenRequeset.SetEndpoint("demo.com");
    getTokenRequeset.SetRegion("sc-cd-1");
    Stub stub;
    stub.set(ADDR(Module::IHttpClient, GetInstance), Stub_Get_Token_Success);
    IamClient iamClient;
    std::shared_ptr<GetTokenResponse> response = iamClient.GetToken(getTokenRequeset);
    EXPECT_TRUE((response != nullptr));
    stub.reset(ADDR(Module::IHttpClient, GetInstance));
}

/*
 * 测试用例： token查询失败
 * 前置条件： 不对request设置参数
 * CHECK点： reponse返回值为nullptr
 */
TEST_F(IamClientTest, GetTokenFailedCheckParamFailed)
{
    GetTokenRequest getTokenRequeset;
    Stub stub;
    stub.set(ADDR(Module::IHttpClient, GetInstance), Stub_Get_Token_Failed);
    IamClient iamClient;
    EXPECT_TRUE((iamClient.GetToken(getTokenRequeset) == nullptr));
    
    AuthObj auth;
    getTokenRequeset.SetUserInfo(auth);
    EXPECT_TRUE((iamClient.GetToken(getTokenRequeset) == nullptr));

    getTokenRequeset.SetDomain("domainName");
    EXPECT_TRUE((iamClient.GetToken(getTokenRequeset) == nullptr));

    getTokenRequeset.SetEndpoint("dome.com");
    EXPECT_TRUE((iamClient.GetToken(getTokenRequeset) == nullptr));

    stub.reset(ADDR(Module::IHttpClient, GetInstance));
}

int IamSendCount = 0;
 
int32_t Stub_SendRequestToGetAdminTokenOfProjectSuccess(
    void *obj, Module::HttpRequest &httpParam, std::shared_ptr<ResponseModel> response)
{
    if (IamSendCount == 0) {
        response->SetSuccess(true);
        response->SetGetBody(g_tokenBody);
        std::set<std::string> headerValue;
        headerValue.insert(g_tokenStr);
        std::map<std::string, std::set<std::string> > getHeadersReturn = {{"X-Subject-Token", headerValue}};
        response->SetHeaders(getHeadersReturn);
        response->SetStatusCode(static_cast<uint32_t>(HcsExternalStatusCode::CREATEED));
        IamSendCount++;
        return SUCCESS;
    }
    if (IamSendCount == 1) {
        response->SetSuccess(true);
        response->SetGetBody(g_adminProjectStr);
        response->SetStatusCode(static_cast<uint32_t>(HcsExternalStatusCode::OK));
        IamSendCount++;
        return SUCCESS;
    }
    if (IamSendCount == 2) {
        response->SetSuccess(true);
        response->SetGetBody(g_tokenBody);
        std::set<std::string> headerValue;
        headerValue.insert(g_tokenStr);
        std::map<std::string, std::set<std::string> > getHeadersReturn = {{"X-Subject-Token", headerValue}};
        response->SetHeaders(getHeadersReturn);
        response->SetStatusCode(static_cast<uint32_t>(HcsExternalStatusCode::CREATEED));
        IamSendCount++;
        return SUCCESS;
    }
}

/*
 * 测试用例： AdmintokenOfProject查询成功
 * 前置条件： 对返回值进行打桩
 * CHECK点： 判断response中的值是否和返回值一致
 */
TEST_F(IamClientTest, GetAdminTokenOfProjectSuccess_project)
{
    GetTokenRequest getTokenRequeset;
    getTokenRequeset.SetTokenType(Scope::USER_DOMAIN);
    AuthObj auth;
    auth.name = "name";
    auth.passwd = "passwd";
    getTokenRequeset.SetUserInfo(auth);
    getTokenRequeset.SetScopeValue("domain");
    getTokenRequeset.SetDomain("domain");
    getTokenRequeset.SetEndpoint("demo.com");
    getTokenRequeset.SetRegion("sc-cd-1");
    Stub stub;
    stub.set(ADDR(HttpClient, Send), Stub_SendRequestToGetAdminTokenOfProjectSuccess);
    IamClient iamClient;
    GetTokenRequest modelBase;
    std::shared_ptr<GetTokenResponse> response = iamClient.GetAdminTokenOfProject(getTokenRequeset, modelBase);
    EXPECT_TRUE((response != nullptr));
    IamSendCount = 0;
}
 
/*
 * 测试用例： AdmintokenOfProject查询失败
 * 前置条件： 对返回值进行打桩，参数校验失败
 * CHECK点： 判断response中的值是否和返回值一致
 */
TEST_F(IamClientTest, GetAdminTokenOfProjectFailed_CheckParamsFailed)
{
    GetTokenRequest getTokenRequeset;
    getTokenRequeset.SetTokenType(Scope::USER_DOMAIN);
    getTokenRequeset.SetScopeValue("domain");
    getTokenRequeset.SetDomain("domain");
    getTokenRequeset.SetEndpoint("demo.com");
    getTokenRequeset.SetRegion("sc-cd-1");
    Stub stub;
    stub.set(ADDR(HttpClient, Send), Stub_SendRequestToGetAdminTokenOfProjectSuccess);
    IamClient iamClient;
    GetTokenRequest modelBase;
    std::shared_ptr<GetTokenResponse> response = iamClient.GetAdminTokenOfProject(getTokenRequeset, modelBase);
    EXPECT_TRUE((response == nullptr));
}
 
int32_t Stub_SendRequestGetTokenFailed(
    void *obj, Module::HttpRequest &httpParam, std::shared_ptr<ResponseModel> response)
{
    response->SetSuccess(true);
    response->SetStatusCode(static_cast<uint32_t>(HcsExternalStatusCode::BAD_REQUEST));
    return SUCCESS;
}
 
/*
 * 测试用例： AdmintokenOfProject查询失败
 * 前置条件： 对返回值进行打桩，获取adminTokenFailed
 * CHECK点： 判断response中的值是否和返回值一致
 */
TEST_F(IamClientTest, GetAdminTokenOfProjectFailed_GetTokenFailed)
{
    GetTokenRequest getTokenRequeset;
    getTokenRequeset.SetTokenType(Scope::USER_DOMAIN);
    AuthObj auth;
    auth.name = "name";
    auth.passwd = "passwd";
    getTokenRequeset.SetUserInfo(auth);
    getTokenRequeset.SetScopeValue("domain");
    getTokenRequeset.SetDomain("domain");
    getTokenRequeset.SetEndpoint("demo.com");
    getTokenRequeset.SetRegion("sc-cd-1");
    Stub stub;
    stub.set(ADDR(HttpClient, Send), Stub_SendRequestGetTokenFailed);
    IamClient iamClient;
    GetTokenRequest modelBase;
    std::shared_ptr<GetTokenResponse> response = iamClient.GetAdminTokenOfProject(getTokenRequeset, modelBase);
    EXPECT_TRUE((response == nullptr));
}
 
int32_t Stub_SendRequestToGetAdminProjectFailed(
    void *obj, Module::HttpRequest &httpParam, std::shared_ptr<ResponseModel> response)
{
    if (IamSendCount == 0) {
        response->SetSuccess(true);
        response->SetGetBody(g_tokenBody);
        std::set<std::string> headerValue;
        headerValue.insert(g_tokenStr);
        std::map<std::string, std::set<std::string> > getHeadersReturn = {{"X-Subject-Token", headerValue}};
        response->SetHeaders(getHeadersReturn);
        response->SetStatusCode(static_cast<uint32_t>(HcsExternalStatusCode::CREATEED));
        IamSendCount++;
        return SUCCESS;
    }
    if (IamSendCount == 1) {
        response->SetSuccess(true);
        response->SetStatusCode(static_cast<uint32_t>(HcsExternalStatusCode::BAD_REQUEST));
        IamSendCount++;
        return SUCCESS;
    }
}
 
/*
 * 测试用例： AdmintokenOfProject查询失败
 * 前置条件： 对返回值进行打桩，获取adminProjectFailed
 * CHECK点： 判断response中的值是否和返回值一致
 */
TEST_F(IamClientTest, GetAdminTokenOfProjectFailed_GetProjectFailed)
{
    GetTokenRequest getTokenRequeset;
    getTokenRequeset.SetTokenType(Scope::USER_DOMAIN);
    AuthObj auth;
    auth.name = "name";
    auth.passwd = "passwd";
    getTokenRequeset.SetUserInfo(auth);
    getTokenRequeset.SetScopeValue("domain");
    getTokenRequeset.SetDomain("domain");
    getTokenRequeset.SetEndpoint("demo.com");
    getTokenRequeset.SetRegion("sc-cd-1");
    Stub stub;
    stub.set(ADDR(HttpClient, Send), Stub_SendRequestToGetAdminProjectFailed);
    IamClient iamClient;
    GetTokenRequest modelBase;
    std::shared_ptr<GetTokenResponse> response = iamClient.GetAdminTokenOfProject(getTokenRequeset, modelBase);
    EXPECT_TRUE((response == nullptr));
    IamSendCount = 0;
}
 
int32_t Stub_SendRequestToGetAdminTokenOfProjectFailed(
    void *obj, Module::HttpRequest &httpParam, std::shared_ptr<ResponseModel> response)
{
    if (IamSendCount == 0) {
        response->SetSuccess(true);
        response->SetGetBody(g_tokenBody);
        std::set<std::string> headerValue;
        headerValue.insert(g_tokenStr);
        std::map<std::string, std::set<std::string> > getHeadersReturn = {{"X-Subject-Token", headerValue}};
        response->SetHeaders(getHeadersReturn);
        response->SetStatusCode(static_cast<uint32_t>(HcsExternalStatusCode::CREATEED));
        IamSendCount++;
        return SUCCESS;
    }
    if (IamSendCount == 1) {
        response->SetSuccess(true);
        response->SetGetBody(g_adminProjectStr);
        response->SetStatusCode(static_cast<uint32_t>(HcsExternalStatusCode::OK));
        IamSendCount++;
        return SUCCESS;
    }
    if (IamSendCount == 2) {
        response->SetSuccess(true);
        response->SetStatusCode(static_cast<uint32_t>(HcsExternalStatusCode::BAD_REQUEST));
        IamSendCount++;
        return SUCCESS;
    }
}
 
/*
 * 测试用例： AdmintokenOfProject查询失败
 * 前置条件： 对返回值进行打桩，获取adminProjectTokenFailed
 * CHECK点： 判断response中的值是否和返回值一致
 */
TEST_F(IamClientTest, GetAdminTokenOfProjectFailed_GetProjectTokenFailed)
{
    GetTokenRequest getTokenRequeset;
    getTokenRequeset.SetTokenType(Scope::USER_DOMAIN);
    AuthObj auth;
    auth.name = "name";
    auth.passwd = "passwd";
    getTokenRequeset.SetUserInfo(auth);
    getTokenRequeset.SetScopeValue("domain");
    getTokenRequeset.SetDomain("domain");
    getTokenRequeset.SetEndpoint("demo.com");
    getTokenRequeset.SetRegion("sc-cd-1");
    Stub stub;
    stub.set(ADDR(HttpClient, Send), Stub_SendRequestToGetAdminTokenOfProjectFailed);
    IamClient iamClient;
    GetTokenRequest modelBase;
    std::shared_ptr<GetTokenResponse> response = iamClient.GetAdminTokenOfProject(getTokenRequeset, modelBase);
    EXPECT_EQ(response->GetStatusCode(), 400);
    IamSendCount = 0;
}

/*
 * 测试用例： GetAdminOfProjects失败
 * 前置条件： 对返回值进行打桩
 * CHECK点： 判断response中的值是否和返回值一致
 */
TEST_F(IamClientTest, GetAdminOfProjects_failed)
{
    GetAdminProjectsRequest reqInfo;
    IamClient iamClient;
    std::shared_ptr<GetAdminProjectsResponse> response = iamClient.GetAdminOfProjects(reqInfo);
    EXPECT_TRUE((response == nullptr));
}

int32_t Stub_get_auth_failed(
    void *obj, Module::HttpRequest &httpParam, std::shared_ptr<ResponseModel> response)
{
    response->SetSuccess(true);
    response->SetGetBody(g_tokenBody);
    std::set<std::string> headerValue;
    headerValue.insert(g_tokenStr);
    std::map<std::string, std::set<std::string> > getHeadersReturn = {{"X-Subject", headerValue}};
    response->SetHeaders(getHeadersReturn);
    response->SetStatusCode(static_cast<uint32_t>(HcsExternalStatusCode::CREATEED));
    return SUCCESS;
}

/*
 * 测试用例： GetAdminTokenOfProject时Auth获取失败
 * 前置条件： 对返回值进行打桩
 * CHECK点： 判断response中的值是否和返回值一致
 */
TEST_F(IamClientTest, GetAdminTokenOfProjectAuth_failed)
{
    GetTokenRequest getTokenRequeset;
    getTokenRequeset.SetTokenType(Scope::USER_DOMAIN);
    AuthObj auth;
    auth.name = "name";
    auth.passwd = "passwd";
    getTokenRequeset.SetUserInfo(auth);
    getTokenRequeset.SetScopeValue("domain");
    getTokenRequeset.SetDomain("domain");
    getTokenRequeset.SetEndpoint("demo.com");
    getTokenRequeset.SetRegion("sc-cd-1");
    Stub stub;
    stub.set(ADDR(HttpClient, Send), Stub_get_auth_failed);
    IamClient iamClient;
    GetTokenRequest modelBase;
    std::shared_ptr<GetTokenResponse> response = iamClient.GetAdminTokenOfProject(getTokenRequeset, modelBase);
    EXPECT_TRUE((response == nullptr));
}

int32_t Stub_firstcallapi_failed(
    void *obj, Module::HttpRequest &httpParam, std::shared_ptr<ResponseModel> response)
{
    if (IamSendCount == 0){
        response->SetSuccess(true);
        response->SetGetBody(g_tokenBody);
        std::set<std::string> headerValue;
        headerValue.insert(g_tokenStr);
        std::map<std::string, std::set<std::string> > getHeadersReturn = {{"X-Subject-Token", headerValue}};
        response->SetHeaders(getHeadersReturn);
        response->SetStatusCode(static_cast<uint32_t>(HcsExternalStatusCode::CREATEED));
        IamSendCount++;
        return SUCCESS;
    } else{
        response->SetSuccess(false);
        return FAILED;
    }
}

/*
 * 测试用例： GetAdminTokenOfProject时Auth获取失败
 * 前置条件： 对返回值进行打桩
 * CHECK点： 判断response中的值是否和返回值一致
 */
TEST_F(IamClientTest, GetAdminTokenOfProject_firstcallapi_failed)
{
    GetTokenRequest getTokenRequeset;
    getTokenRequeset.SetTokenType(Scope::USER_DOMAIN);
    AuthObj auth;
    auth.name = "name";
    auth.passwd = "passwd";
    getTokenRequeset.SetUserInfo(auth);
    getTokenRequeset.SetScopeValue("domain");
    getTokenRequeset.SetDomain("domain");
    getTokenRequeset.SetEndpoint("demo.com");
    getTokenRequeset.SetRegion("sc-cd-1");
    Stub stub;
    stub.set(ADDR(HttpClient, Send), Stub_firstcallapi_failed);
    IamClient iamClient;
    GetTokenRequest modelBase;
    std::shared_ptr<GetTokenResponse> response = iamClient.GetAdminTokenOfProject(getTokenRequeset, modelBase);
    EXPECT_TRUE((response == nullptr));
    IamSendCount = 0;
}

int32_t Stub_Secondcallapi_failed(
    void *obj, Module::HttpRequest &httpParam, std::shared_ptr<ResponseModel> response)
{
    if (IamSendCount == 0) {
        response->SetSuccess(true);
        response->SetGetBody(g_tokenBody);
        std::set<std::string> headerValue;
        headerValue.insert(g_tokenStr);
        std::map<std::string, std::set<std::string> > getHeadersReturn = {{"X-Subject-Token", headerValue}};
        response->SetHeaders(getHeadersReturn);
        response->SetStatusCode(static_cast<uint32_t>(HcsExternalStatusCode::CREATEED));
        IamSendCount++;
        return SUCCESS;
    }
    if (IamSendCount == 1) {
        response->SetSuccess(true);
        response->SetGetBody(g_adminProjectStr);
        response->SetStatusCode(static_cast<uint32_t>(HcsExternalStatusCode::OK));
        IamSendCount++;
        return SUCCESS;
    }
    if (IamSendCount == 2) {
        response->SetSuccess(false);
        return FAILED;
    }
}

/*
 * 测试用例： GetAdminTokenOfProject时Auth获取失败
 * 前置条件： 对返回值进行打桩
 * CHECK点： 判断response中的值是否和返回值一致
 */
TEST_F(IamClientTest, GetAdminTokenOfProject_Secondcallapi_failed)
{
    GetTokenRequest getTokenRequeset;
    getTokenRequeset.SetTokenType(Scope::USER_DOMAIN);
    AuthObj auth;
    auth.name = "name";
    auth.passwd = "passwd";
    getTokenRequeset.SetUserInfo(auth);
    getTokenRequeset.SetScopeValue("domain");
    getTokenRequeset.SetDomain("domain");
    getTokenRequeset.SetEndpoint("demo.com");
    getTokenRequeset.SetRegion("sc-cd-1");
    Stub stub;
    stub.set(ADDR(HttpClient, Send), Stub_Secondcallapi_failed);
    IamClient iamClient;
    GetTokenRequest modelBase;
    std::shared_ptr<GetTokenResponse> response = iamClient.GetAdminTokenOfProject(getTokenRequeset, modelBase);
    EXPECT_TRUE((response != nullptr));
    IamSendCount = 0;
}

std::string g_emptyProjectStr = "{\"projects\":[{\"id\":\"138152f07fc649278939c9eabf43abb2\",\"name\":\"MOS\",\"domain_id\":\"f79ca95d4c744c2498d56e131d332c11\",\"enabled\":true,\"parent_id\":\"f79ca95d4c744c2498d56e131d332c11\",\"is_domain\":false,}]}";


int32_t Stub_bssProjectNameempty_failed(
    void *obj, Module::HttpRequest &httpParam, std::shared_ptr<ResponseModel> response)
{
    if (IamSendCount == 0) {
        response->SetSuccess(true);
        response->SetGetBody(g_tokenBody);
        std::set<std::string> headerValue;
        headerValue.insert(g_tokenStr);
        std::map<std::string, std::set<std::string> > getHeadersReturn = {{"X-Subject-Token", headerValue}};
        response->SetHeaders(getHeadersReturn);
        response->SetStatusCode(static_cast<uint32_t>(HcsExternalStatusCode::CREATEED));
        IamSendCount++;
        return SUCCESS;
    }
    if (IamSendCount == 1) {
        response->SetSuccess(true);
        response->SetGetBody(g_emptyProjectStr);
        response->SetStatusCode(static_cast<uint32_t>(HcsExternalStatusCode::OK));
        IamSendCount++;
        return SUCCESS;
    }
}

/*
 * 测试用例： GetAdminTokenOfProject时Auth获取失败
 * 前置条件： 对返回值进行打桩
 * CHECK点： 判断response中的值是否和返回值一致
 */
TEST_F(IamClientTest, GetAdminTokenOfProject_bssProjectNameempty_failed)
{
    GetTokenRequest getTokenRequeset;
    getTokenRequeset.SetTokenType(Scope::USER_DOMAIN);
    AuthObj auth;
    auth.name = "name";
    auth.passwd = "passwd";
    getTokenRequeset.SetUserInfo(auth);
    getTokenRequeset.SetScopeValue("domain");
    getTokenRequeset.SetDomain("domain");
    getTokenRequeset.SetEndpoint("demo.com");
    getTokenRequeset.SetRegion("sc-cd-1");
    Stub stub;
    stub.set(ADDR(HttpClient, Send), Stub_bssProjectNameempty_failed);
    IamClient iamClient;
    GetTokenRequest modelBase;
    std::shared_ptr<GetTokenResponse> response = iamClient.GetAdminTokenOfProject(getTokenRequeset, modelBase);
    EXPECT_TRUE((response == nullptr));
    IamSendCount = 0;
}
}
