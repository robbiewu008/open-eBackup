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
#include "IHttpResponseMock.h"
#include "IHttpClientMock.h"
#include "common/model/ResponseModelMock.h"
#include "common/model/ModelBaseMock.h"
#include "protect_engines/hcs/utils/HCSTokenMgr.h"
#include "common/Structs.h"
#include "common/token_mgr/TokenDetail.h"

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
class TokenMgrTest : public testing::Test {
public:
    void InitLogger()
    {
        std::string logFileName = "token_mgr_test.log";
        std::string logFilePath = "/tmp/log/";
        int logLevel = DEBUG;
        int logFileCount = 10;
        int logFileSize = 30;
        Module::CLogger::GetInstance().Init(
            logFileName.c_str(), logFilePath, logLevel, logFileCount, logFileSize);
    }

protected:
    void SetUp() {
        InitLogger();
    }
    void TearDown() {}
};

std::string g_tokenStr_1 = "MIIEhwYJKoZIhvcNAQcCoIIEeDCCBHQCAQExDTALBglghkgBZQMEAgEwggLoBgkqhkiG9w0BBwGgggLZBIIC1XsidG9rZW4iOnsiZXhwaXJlc19hdCI6IjIwMjItMDctMjJUMDc6NTk6NTkuODY5MDAwWiIsIm1ldGhvZHMiOlsicGFzc3dvcmQiXSwiY2F0YWxvZyI6W10sInJvbGVzIjpbeyJuYW1lIjoidmRjX2FkbSIsImlkIjoiY2E3MWU3NzFiYWZjNDI5OTkwOThmMDg4YTc4NGM3NTEifSx7Im5hbWUiOiJ0YWdfYWRtIiwiaWQiOiJkNmJiNWRiZjc0YjQ0Yjk1YWFmMmU3MGJmYzcyNTE0ZiJ9LHsibmFtZSI6ImFwcHJvdl9hZG0iLCJpZCI6IjBiZDBmZjlhMWZkNDRiNzc5MzZlNWUzNzExODZhMzI1In0seyJuYW1lIjoidmRjX293bmVyIiwiaWQiOiI0Nzg4ZjYyMzhmZDM0MWNjYmZkOGQwYzQzMzg4YjdlZSJ9LHsibmFtZSI6InRlX2FkbWluIiwiaWQiOiJhYzgyMWRkN2EwZDI0ZDI2OGI4ZGE2MDg0ZmRlNmQ3OCJ9XSwicHJvamVjdCI6eyJkb21haW4iOnsibmFtZSI6Imh1YW5ncm9uZyIsImlkIjoiOTkwNzYzNjFiOTVmNDIyNmIxOGRiMDAwMTU1NWJkMDAifSwibmFtZSI6InNjLWNkLTFfdGVzdCIsImlkIjoiZTM4ZDIyN2VkY2NlNDYzMWJlMjBiZmE1YWFkNzEzMGIifSwiaXNzdWVkX2F0IjoiMjAyMi0wNy0yMVQwNzo1OTo1OS44NjkwMDBaIiwidXNlciI6eyJkb21haW4iOnsibmFtZSI6Imh1YW5ncm9uZyIsImlkIjoiOTkwNzYzNjFiOTVmNDIyNmIxOGRiMDAwMTU1NWJkMDAifSwibmFtZSI6Imh1YW5ncm9uZyIsImlkIjoiZDQyMTZiN2QzYmE2NGE0ZWI2M2RiMzdjMmI5MTIyMmMifX19MYIBcjCCAW4CAQEwSTA9MQswCQYDVQQGEwJDTjEPMA0GA1UEChMGSHVhd2VpMR0wGwYDVQQDExRIdWF3ZWkgSVQgUHJvZHVjdCBDQQIIFamRIbpBmrcwCwYJYIZIAWUDBAIBMA0GCSqGSIb3DQEBAQUABIIBAGkKLMyXHOFwT4nqe4Iue5g59bBMsIAhW-bhq0MIiJklULEo8RDH+hX5e8AQ44K1Dv2KKXSctXqZoIjW+SeRFxSQm8Ifp-mw18gDn6F+DZRE1ZS+CeecSG8BmXutAfhd9YJQ2xRcw4tbOy21OY-WrXXqIkyyAW1kZpv1yejMm6d6QHDanObsrH9aMJkv79l9tpu0lk4kXM4ohAaUSbVJm47iOiRN2BNxnsHa4bymXFOCIkUYLtA+z0-BXjJIiZjem6Uhtqt6P97Z7MzyuTSFMw0fl6BGswajprEqrVvJg7tB2WCstsff2SPedA86-ufA39TrGuu1kWhLJeUWGQTf2PI=";

std::string g_tokenBody_1 = "{\"token\": {         \"expires_at\": \"2022-07-22T07:59:59.869000Z\",         \"methods\": [\"password\"],         \"catalog\": [             {                 \"endpoints\": [                     {                         \"region_id\": \"sc-cd-1\",                         \"id\": \"c8476fa3de214d42b99cdc8103b36a5e\",                         \"region\": \"sc-cd-1\",                         \"interface\": \"public\",                         \"url\": \"https://evs.sc-cd-1.demo.com/v2/e38d227edcce4631be20bfa5aad7130b\"                     }                 ],                 \"name\": \"evs\",                 \"id\": \"2cd3de7abbeb40fa859d73ab6a8282f5\",                 \"type\": \"volume\"             },             {                 \"endpoints\": [                     {                         \"region_id\": \"sc-cd-1\",                         \"id\": \"c1bf27b0c6644301971643034e063047\",                         \"region\": \"sc-cd-1\",                         \"interface\": \"internal\",                         \"url\": \"https://iam-cache-proxy.sc-cd-1.demo.com:26335\"                     },                     {                         \"region_id\": \"sc-cd-1\",                         \"id\": \"851c1913e6bc4c90ac3b948c7d20e665\",                         \"region\": \"sc-cd-1\",                         \"interface\": \"public\",                         \"url\": \"https://iam-apigateway-proxy.sc-cd-1.demo.com\"                     }                 ],                 \"name\": \"iam\",                 \"id\": \"4b47e15c6c0c4d02b1b6ff75a003e6c6\",                 \"type\": \"iam\"             },             {                 \"endpoints\": [                     {                         \"region_id\": \"sc-cd-1\",                         \"id\": \"c4e4278a31014b6282d9a3403214b540\",                         \"region\": \"sc-cd-1\",                         \"interface\": \"public\",                         \"url\": \"https://ecs.sc-cd-1.demo.com/v2/e38d227edcce4631be20bfa5aad7130b\"                     }                 ],                 \"name\": \"ecs\",                 \"id\": \"b1efa676dfad47609c7b9a6a6fe2c861\",                 \"type\": \"compute\"             },             {                 \"endpoints\": [                     {                         \"region_id\": \"sc-cd-1\",                         \"id\": \"cd044271e54348c0b4f1cb14211b317b\",                         \"region\": \"sc-cd-1\",                         \"interface\": \"internal\",                         \"url\": \"https://sc.demo.com:26335\"                     },                     {                         \"region_id\": \"sc-cd-1\",\                         \"id\": \"75b86267698a4cb3a34e2a1c555ee77f\",                         \"region\": \"sc-cd-1\",                         \"interface\": \"public\",                         \"url\": \"https://sc.demo.com\"                     }                 ],                 \"name\": \"sc\",                 \"id\": \"b921c288b2314f5fa13bd9342823e6d9\",                 \"type\": \"sc\"             },         ],         \"roles\": [             {                 \"name\": \"vdc_adm\",                 \"id\": \"ca71e771bafc42999098f088a784c751\"             },             {                 \"name\": \"tag_adm\",                 \"id\": \"d6bb5dbf74b44b95aaf2e70bfc72514f\"             },         ],         \"project\": {             \"domain\": {                 \"name\": \"huangrong\",                 \"id\": \"99076361b95f4226b18db0001555bd00\"             },             \"name\": \"sc-cd-1_test\",             \"id\": \"e38d227edcce4631be20bfa5aad7130b\"         },         \"issued_at\": \"2022-07-21T07:59:59.869000Z\",         \"user\": {             \"domain\": {                 \"name\": \"huangrong\",                 \"id\": \"99076361b95f4226b18db0001555bd00\"             },             \"name\": \"huangrong\",             \"id\": \"d4216b7d3ba64a4eb63db37c2b91222c\"         }     } }";
std::string g_tokenBody_catalogIsEmpty = "{\"token\":{\"expires_at\":\"2022-07-22T07:59:59.869000Z\",\"methods\":[\"password\"],\"catalog\":[],\"roles\":[{\"name\":\"vdc_adm\",\"id\":\"ca71e771bafc42999098f088a784c751\"},{\"name\":\"tag_adm\",\"id\":\"d6bb5dbf74b44b95aaf2e70bfc72514f\"},],\"project\":{\"domain\":{\"name\":\"huangrong\",\"id\":\"99076361b95f4226b18db0001555bd00\"},\"name\":\"sc-cd-1_test\",\"id\":\"e38d227edcce4631be20bfa5aad7130b\"},\"issued_at\":\"2022-07-21T07:59:59.869000Z\",\"user\":{\"domain\":{\"name\":\"huangrong\",\"id\":\"99076361b95f4226b18db0001555bd00\"},\"name\":\"huangrong\",\"id\":\"d4216b7d3ba64a4eb63db37c2b91222c\"}}}";
bool g_use_project_tokenBody = true;
bool g_XSubjectToken_exsitInHead = true;

int gGetTokenStatusCode = 201;

static Module::IHttpClient* Stub_CreateClient_SendRequest()
{
    std::shared_ptr<IHttpResponseMock> httpRespone = std::make_shared<IHttpResponseMock>();
    EXPECT_CALL(*httpRespone, Success()).WillRepeatedly(Return(true));
    EXPECT_CALL(*httpRespone, GetStatusCode()).WillRepeatedly(Return(gGetTokenStatusCode));
    EXPECT_CALL(*httpRespone, GetErrCode()).WillRepeatedly(Return(0));
    EXPECT_CALL(*httpRespone, GetErrString()).WillRepeatedly(Return(""));
    EXPECT_CALL(*httpRespone, Busy()).WillRepeatedly(Return(false));
    EXPECT_CALL(*httpRespone, GetHttpStatusCode()).WillRepeatedly(Return(0));
    EXPECT_CALL(*httpRespone, GetHttpStatusDescribe()).WillRepeatedly(Return(""));
    std::set<std::string> GetHeadByNameReturn = {};
    EXPECT_CALL(*httpRespone, GetHeadByName(_)).WillRepeatedly(Return(GetHeadByNameReturn));
    if (g_use_project_tokenBody) {
        EXPECT_CALL(*httpRespone, GetBody()).WillRepeatedly(Return(g_tokenBody_1));
    } else {
        EXPECT_CALL(*httpRespone, GetBody()).WillRepeatedly(Return(g_tokenBody_catalogIsEmpty));
    }
    std::map<std::string, std::set<std::string> > getHeadersReturn = {};
    if (g_XSubjectToken_exsitInHead) {
        std::set<std::string> headerValue;
        headerValue.insert(g_tokenStr_1);
        getHeadersReturn["X-Subject-Token"] = headerValue;
    }
    EXPECT_CALL(*httpRespone, GetHeaders()).WillRepeatedly(Return(getHeadersReturn));
    std::set<std::string> getCookiesReturn = {};
    EXPECT_CALL(*httpRespone, GetCookies()).WillRepeatedly(Return(getCookiesReturn));

    IHttpClientMock* httpClient = new(std::nothrow) IHttpClientMock();
    EXPECT_CALL(*httpClient, SendRequest(_,_)).WillRepeatedly(Return(httpRespone));
    return httpClient;
}

static bool Stub_IsExpired_Failed()
{
    return false;
}

static bool StubSerialStructFailed()
{
    return false;
}

void GetRequestMock(ModelBaseMock &request)
{
    AuthObj auth;
    auth.name = "name";
    auth.passwd = "passwd";
    request.SetUserInfo(auth);
    request.SetScopeValue("e38d227edcce4631be20bfa5aad7130b");
    request.SetDomain("domain");
    request.SetEndpoint("demo.com");
    request.SetRegion("sc-cd-1");

    EXPECT_CALL(request, GetScopeType()).WillRepeatedly(Return(Scope::PROJECT));
    EXPECT_CALL(request, GetApiType()).WillRepeatedly(Return(ApiType::ECS));
}

void GetRequestMockDomain(ModelBaseMock &request)
{
    AuthObj auth;
    auth.name = "name";
    auth.passwd = "passwd";
    request.SetUserInfo(auth);
    request.SetScopeValue("domain");
    request.SetDomain("domain");
    request.SetEndpoint("demo.com");
    request.SetRegion("sc-cd-1");

    EXPECT_CALL(request, GetScopeType()).WillRepeatedly(Return(Scope::USER_DOMAIN));
    EXPECT_CALL(request, GetApiType()).WillRepeatedly(Return(ApiType::SC));
}

/*
 * 测试用例： 查询token，解析endpoint成功
 * 前置条件： 消息发送成功，ECS接口调用
 * CHECK点： GetToken 返回值为true, token值等于打桩值，endpoint和打桩值相同
 */
TEST_F(TokenMgrTest, GetTokenSuccess)
{
    std::string tokenStr, endpoint;
    ModelBaseMock request;
    GetRequestMock(request);

    Stub stub;
    gGetTokenStatusCode = 201;
    HCSTokenMgr::GetInstance().m_tokenMap.clear();
    stub.set(ADDR(Module::IHttpClient, GetInstance), Stub_CreateClient_SendRequest);
    stub.set(ADDR(BaseTokenMgr, TokenIsExpired), Stub_IsExpired_Failed);
    bool ret = HCSTokenMgr::GetInstance().GetToken(request, tokenStr, endpoint);
    EXPECT_TRUE(ret);
    EXPECT_EQ(tokenStr, g_tokenStr_1);
    EXPECT_EQ(endpoint, "https://ecs.sc-cd-1.demo.com/v2/e38d227edcce4631be20bfa5aad7130b");
    stub.reset(ADDR(Module::IHttpClient, GetInstance));
    stub.reset(ADDR(BaseTokenMgr, TokenIsExpired));
}

/*
 * 测试用例： 查询token
 * 前置条件： 消息发送成功
 * CHECK点： GetToken 返回值为true, token值等于打桩值，endpoint和打桩值相同
 */
TEST_F(TokenMgrTest, GetTokenSuccess_when_domain)
{
    std::string tokenStr, endpoint;
    ModelBaseMock request;
    GetRequestMockDomain(request);

    Stub stub;
    gGetTokenStatusCode = 201;
    HCSTokenMgr::GetInstance().m_tokenMap.clear();
    stub.set(ADDR(Module::IHttpClient, GetInstance), Stub_CreateClient_SendRequest);
    stub.set(ADDR(BaseTokenMgr, TokenIsExpired), Stub_IsExpired_Failed);
    bool ret = HCSTokenMgr::GetInstance().GetToken(request, tokenStr, endpoint);
    EXPECT_TRUE(ret);
    EXPECT_EQ(tokenStr, g_tokenStr_1);
    EXPECT_EQ(endpoint, "");
    stub.reset(ADDR(Module::IHttpClient, GetInstance));
    stub.reset(ADDR(BaseTokenMgr, TokenIsExpired));
}

/*
 * 测试用例： 查询token成功
 * 前置条件： 缓存中存在token且token未过期
 * CHECK点： GetToken 返回值为true, token值等于打桩值，endpoint和打桩值相同
 */
TEST_F(TokenMgrTest, GetTokenSuccess_when_TokenNotExpiredInMap)
{
    std::string tokenStr, endpoint;
    ModelBaseMock request;
    GetRequestMock(request);

    TokenInfo tokenInfo;
    tokenInfo.m_token = g_tokenStr_1;
    tokenInfo.m_expiresDate = "2022-07-22T07:59:59.869000Z";
    tokenInfo.m_extendInfo = g_tokenBody_1;
    std::string key_token =  HCSTokenMgr::GetInstance().GetTokenKey(request);
    HCSTokenMgr::GetInstance().m_tokenMap.clear();
    HCSTokenMgr::GetInstance().m_tokenMap[key_token] = tokenInfo;

    Stub stub;
    stub.set(ADDR(BaseTokenMgr, TokenIsExpired), Stub_IsExpired_Failed);
    bool ret = HCSTokenMgr::GetInstance().GetToken(request, tokenStr, endpoint);
    EXPECT_TRUE(ret);
    EXPECT_EQ(tokenStr, g_tokenStr_1);
    EXPECT_EQ(endpoint, "https://ecs.sc-cd-1.demo.com/v2/e38d227edcce4631be20bfa5aad7130b");
    stub.reset(ADDR(BaseTokenMgr, TokenIsExpired));
}

/*
 * 测试用例： 查询token成功
 * 前置条件： 缓存中存在token且token过期，消息发送成功，ECS接口调用
 * CHECK点： GetToken 返回值为true, token值等于打桩值，endpoint和打桩值相同
 */
TEST_F(TokenMgrTest, GetTokenSuccess_when_TokenExpiredInMap)
{
    std::string tokenStr, endpoint;
    ModelBaseMock request;
    GetRequestMock(request);

    TokenInfo tokenInfo;
    tokenInfo.m_token = g_tokenStr_1;
    tokenInfo.m_expiresDate = "2022-06-22T07:59:59.869000Z";
    tokenInfo.m_extendInfo = g_tokenBody_1;
    std::string key_token =  HCSTokenMgr::GetInstance().GetTokenKey(request);
    HCSTokenMgr::GetInstance().m_tokenMap.clear();
    HCSTokenMgr::GetInstance().m_tokenMap[key_token] = tokenInfo;

    Stub stub;
    gGetTokenStatusCode = 201;
    stub.set(ADDR(Module::IHttpClient, GetInstance), Stub_CreateClient_SendRequest);
    bool ret = HCSTokenMgr::GetInstance().GetToken(request, tokenStr, endpoint);
    EXPECT_TRUE(ret);
    EXPECT_EQ(tokenStr, g_tokenStr_1);
    EXPECT_EQ(endpoint, "https://ecs.sc-cd-1.demo.com/v2/e38d227edcce4631be20bfa5aad7130b");
    stub.reset(ADDR(Module::IHttpClient, GetInstance));
}

/*
 * 测试用例： 查询token失败
 * 前置条件： 消息发送成功，序列化失败
 * CHECK点： GetToken 返回值为false
 */
TEST_F(TokenMgrTest, GetTokenFailed_when_serialStruct)
{
    std::string tokenStr, endpoint;
    ModelBaseMock request;
    GetRequestMock(request);

    Stub stub;
    gGetTokenStatusCode = 201;
    HCSTokenMgr::GetInstance().m_tokenMap.clear();
    stub.set(ADDR(Module::IHttpClient, GetInstance), Stub_CreateClient_SendRequest);
    stub.set(ADDR(BaseTokenMgr, TokenIsExpired), Stub_IsExpired_Failed);
    stub.set(ADDR(GetTokenResponse, Serial), StubSerialStructFailed);
    bool ret = HCSTokenMgr::GetInstance().GetToken(request, tokenStr, endpoint);
    EXPECT_FALSE(ret);
    stub.reset(ADDR(GetTokenResponse, Serial));
    stub.reset(ADDR(Module::IHttpClient, GetInstance));
    stub.reset(ADDR(BaseTokenMgr, TokenIsExpired));
}

/*
 * 测试用例： 查询token失败
 * 前置条件： 消息发送成功，head中无字段“X-Subject-Token”
 * CHECK点： GetToken 返回值为false
 */
TEST_F(TokenMgrTest, GetTokenFailed_when_XSubjectToken_notExsitInHead)
{
    std::string tokenStr, endpoint;
    ModelBaseMock request;
    GetRequestMock(request);

    Stub stub;
    gGetTokenStatusCode = 201;
    g_XSubjectToken_exsitInHead = false;
    HCSTokenMgr::GetInstance().m_tokenMap.clear();
    stub.set(ADDR(Module::IHttpClient, GetInstance), Stub_CreateClient_SendRequest);
    stub.set(ADDR(BaseTokenMgr, TokenIsExpired), Stub_IsExpired_Failed);
    bool ret = HCSTokenMgr::GetInstance().GetToken(request, tokenStr, endpoint);
    EXPECT_FALSE(ret);
    g_XSubjectToken_exsitInHead = true;
    stub.reset(ADDR(Module::IHttpClient, GetInstance));
    stub.reset(ADDR(BaseTokenMgr, TokenIsExpired));
}

/*
 * 测试用例： 查询token失败
 * 前置条件： 消息发送成功，返回体中catalog为空
 * CHECK点： GetToken 返回值为false
 */
TEST_F(TokenMgrTest, GetTokenFailed_when_ParseProjectTokenBody)
{
    std::string tokenStr, endpoint;
    ModelBaseMock request;
    GetRequestMock(request);

    Stub stub;
    gGetTokenStatusCode = 201;
    g_use_project_tokenBody = false;
    HCSTokenMgr::GetInstance().m_tokenMap.clear();
    stub.set(ADDR(Module::IHttpClient, GetInstance), Stub_CreateClient_SendRequest);
    stub.set(ADDR(BaseTokenMgr, TokenIsExpired), Stub_IsExpired_Failed);
    bool ret = HCSTokenMgr::GetInstance().GetToken(request, tokenStr, endpoint);
    EXPECT_FALSE(ret);
    g_use_project_tokenBody = true;
    stub.reset(ADDR(Module::IHttpClient, GetInstance));
    stub.reset(ADDR(BaseTokenMgr, TokenIsExpired));
}

/*
 * 测试用例： 查询token失败
 * 前置条件： 消息发送失败
 * CHECK点： GetToken 返回值为false
 */
TEST_F(TokenMgrTest, GetTokenFailed)
{
    std::string tokenStr, endpoint;
    ModelBaseMock request;
    GetRequestMock(request);

    Stub stub;
    gGetTokenStatusCode = 404;
    HCSTokenMgr::GetInstance().m_tokenMap.clear();
    stub.set(ADDR(Module::IHttpClient, GetInstance), Stub_CreateClient_SendRequest);
    bool ret = HCSTokenMgr::GetInstance().GetToken(request, tokenStr, endpoint);
    EXPECT_FALSE(ret);
    stub.reset(ADDR(Module::IHttpClient, GetInstance));
}

/*
 * 测试用例： token过期
 * 前置条件： 传入时间早于现在24小时
 * CHECK点： 返回true
 */
TEST_F(TokenMgrTest, GetTokenExpired)
{
    std::string date = "2022-06-22T07:59:59.869000Z";
    bool ret = HCSTokenMgr::GetInstance().TokenIsExpired(date);
    EXPECT_TRUE(ret);
}

/*
 * 测试用例： token过期失败
 * 前置条件： 时间格式不符合规范
 * CHECK点： 返回false
 */
TEST_F(TokenMgrTest, GetTokenExpiredFailed)
{
    std::string date = "2022-06-22T07:59:59";
    bool ret = HCSTokenMgr::GetInstance().TokenIsExpired(date);
    EXPECT_FALSE(ret);

    date = ".";
    ret = HCSTokenMgr::GetInstance().TokenIsExpired(date);
    EXPECT_FALSE(ret);
}

/*
 * 测试用例： 日志转发为时间戳失败
 * 前置条件： 时间格式不符合规范
 * CHECK点： 返回false
 */
TEST_F(TokenMgrTest, DateTransToStampFailed)
{
    std::string date = "";
    std::string timStamp = "";
    bool ret = HCSTokenMgr::GetInstance().DateTransToStamp(date, timStamp);
    EXPECT_FALSE(ret);
    
    date = "2022-06-22";
    ret = HCSTokenMgr::GetInstance().DateTransToStamp(date, timStamp);
    EXPECT_FALSE(ret);

    date = "2022-06-40";
    ret = HCSTokenMgr::GetInstance().DateTransToStamp(date, timStamp);
    EXPECT_FALSE(ret);
}

/*
 * 测试用例： 添加token到缓存中失败
 * 前置条件： 入参不符合规范
 * CHECK点： 返回false
 */
TEST_F(TokenMgrTest, AddTokenToMapFailed)
{
    ModelBaseMock request;
    GetRequestMock(request);
    std::string tokenStr = HCSTokenMgr::GetInstance().AddToken(request, nullptr);
    EXPECT_EQ(tokenStr, "");
}

TEST_F(TokenMgrTest, ReplaceProjectIdSuccess)
{
    std::string url;
    std::string newId;
    std::string expectUrl;
    /* 1. replace success */
    url = "https://volume.az0.dc0.demo.com:443/v2/711f43d79ef64a7cad3ea6ad7787b771";
    newId = "711f43d79ef64a7cad3ea6ad7787b772";
    expectUrl = "https://volume.az0.dc0.demo.com:443/v2/" + newId;
    HCSTokenMgr::GetInstance().ReplaceProjectId("cinder", newId, url);
    EXPECT_EQ(url, expectUrl);
}

TEST_F(TokenMgrTest, ReplaceProjectIdSameNewOldSuccess)
{
    std::string url;
    std::string newId;
    std::string expectUrl;
    /* new and old are the same */
    url = "https://volume.az0.dc0.demo.com:443/v2/711f43d79ef64a7cad3ea6ad7787b771";
    newId = "711f43d79ef64a7cad3ea6ad7787b771";
    expectUrl = "https://volume.az0.dc0.demo.com:443/v2/" + newId;
    HCSTokenMgr::GetInstance().ReplaceProjectId("cinder", newId, url);
    EXPECT_EQ(url, expectUrl);
}

TEST_F(TokenMgrTest, ReplaceProjectIdOldLenNotMatchSuccess)
{
    std::string url;
    std::string newId;
    std::string expectUrl;

    /* malformed url(len not math) */
    url = "https://volume.az0.dc0.demo.com:443/v2/711f43d79ef64a7cad3ea6ad7787b7711";
    newId = "711f43d79ef64a7cad3ea6ad7787b771";
    expectUrl = url;
    HCSTokenMgr::GetInstance().ReplaceProjectId("cinder", newId, url);
    EXPECT_EQ(url, expectUrl);
}

TEST_F(TokenMgrTest, ReplaceProjectIdSpaceInOldSuccess)
{
    std::string url;
    std::string newId;
    std::string expectUrl;

    /* malformed url(blank charactor found) */
    url = "https://volume.az0.dc0.demo.com:443/v2/711f43d79ef64a7cad3ea6ad7787b771 1";
    newId = "711f43d79ef64a7cad3ea6ad7787b771";
    expectUrl = url;
    HCSTokenMgr::GetInstance().ReplaceProjectId("cinder", newId, url);
    EXPECT_EQ(url, expectUrl);
}

TEST_F(TokenMgrTest, ReplaceProjectIdSpecialInOldSuccess)
{
    std::string url;
    std::string newId;
    std::string expectUrl;

    /* malformed url(special charactor) */
    url = "https://volume.az0.dc0.demo.com:443/v2/711f43d79ef64a7cad3ea6ad7787b7_1";
    newId = "711f43d79ef64a7cad3ea6ad7787b771";
    expectUrl = url;
    HCSTokenMgr::GetInstance().ReplaceProjectId("cinder", newId, url);
    EXPECT_EQ(url, expectUrl);
}

TEST_F(TokenMgrTest, ReplaceProjectIdSlash1Success)
{
    std::string url;
    std::string newId;
    std::string expectUrl;

    /* malformed url(special charactor) */
    url = "https://volume.az0.dc0.demo.com:443/v2/711f43d79ef64a7cad3ea6ad7787b771//////";
    newId = "711f43d79ef64a7cad3ea6ad7787b772";
    expectUrl = "https://volume.az0.dc0.demo.com:443/v2/" + newId;
    HCSTokenMgr::GetInstance().ReplaceProjectId("cinder", newId, url);
    EXPECT_EQ(url, expectUrl);
}

TEST_F(TokenMgrTest, ReplaceProjectIdSlashe2Success)
{
    std::string url;
    std::string newId;
    std::string expectUrl;

    /* malformed url(slash) */
    url = "https://volume.az0.dc0.demo.com:443/v2//////711f43d79ef64a7cad3ea6ad7787b771/";
    newId = "711f43d79ef64a7cad3ea6ad7787b772";
    expectUrl = "https://volume.az0.dc0.demo.com:443/v2//////" + newId;
    HCSTokenMgr::GetInstance().ReplaceProjectId("cinder", newId, url);
    EXPECT_EQ(url, expectUrl);
}

TEST_F(TokenMgrTest, ReplaceProjectIdNewIdLenNotMatchSuccess)
{
    std::string url;
    std::string newId;
    std::string expectUrl;

    /* new id invalid(length not match) */
    url = "https://volume.az0.dc0.demo.com:443/v2/711f43d79ef64a7cad3ea6ad7787b771";
    newId = "711f43d79ef64a7cad3ea6ad7787b77";
    expectUrl = url;
    HCSTokenMgr::GetInstance().ReplaceProjectId("cinder", newId, url);
    EXPECT_EQ(url, expectUrl);
}

TEST_F(TokenMgrTest, ReplaceProjectIdNewIdSpecialSuccess)
{
    std::string url;
    std::string newId;
    std::string expectUrl;

    /* new id invalid(special charactor) */
    url = "https://volume.az0.dc0.demo.com:443/v2/711f43d79ef64a7cad3ea6ad7787b771";
    newId = "711f43d79ef64a7cad3ea6ad7787b7_7";
    expectUrl = url;
    HCSTokenMgr::GetInstance().ReplaceProjectId("cinder", newId, url);
    EXPECT_EQ(url, expectUrl);
}

}
