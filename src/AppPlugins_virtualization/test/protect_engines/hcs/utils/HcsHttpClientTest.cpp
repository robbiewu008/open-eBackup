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
#include "common/httpclient/HttpClient.h"
#include "common/model/ResponseModelMock.h"
#include "common/CommonMock.h"

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


namespace HDT_TEST {
class HttpClientTest : public testing::Test {
public:
    void InitLogger()
    {
        std::string logFileName = "http_client_test.log";
        std::string logFilePath = "/tmp/log/";
        int logLevel = DEBUG;
        int logFileCount = 10;
        int logFileSize = 30;
        Module::CLogger::GetInstance().Init(
                logFileName.c_str(), logFilePath, logLevel, logFileCount, logFileSize);
    }
protected:
    void SetUp() {
        stub.set(sleep, Stub_Sleep);
        InitLogger();
    }
    void TearDown() {}
public:
    Stub stub;
};

int g_netErr_statusCode = 0;

static Module::IHttpClient* Stub_CreateClient_SendRequest_Success()
{
    std::shared_ptr<IHttpResponseMock> httpRespone = std::make_shared<IHttpResponseMock>();
    testing::Mock::AllowLeak(httpRespone.get());
    EXPECT_CALL(*httpRespone, Success()).WillRepeatedly(Return(true));
    EXPECT_CALL(*httpRespone, GetStatusCode()).WillRepeatedly(Return(200));
    EXPECT_CALL(*httpRespone, GetErrCode()).WillRepeatedly(Return(0));
    EXPECT_CALL(*httpRespone, GetErrString()).WillRepeatedly(Return(""));
    EXPECT_CALL(*httpRespone, Busy()).WillRepeatedly(Return(false));
    EXPECT_CALL(*httpRespone, GetHttpStatusCode()).WillRepeatedly(Return(0));
    EXPECT_CALL(*httpRespone, GetHttpStatusDescribe()).WillRepeatedly(Return(""));
    std::set<std::string> GetHeadByNameReturn = {};
    EXPECT_CALL(*httpRespone, GetHeadByName(_)).WillRepeatedly(Return(GetHeadByNameReturn));
    EXPECT_CALL(*httpRespone, GetBody()).WillRepeatedly(Return(""));
    std::map<std::string, std::set<std::string> > getHeadersReturn = {};
    EXPECT_CALL(*httpRespone, GetHeaders()).WillRepeatedly(Return(getHeadersReturn));
    std::set<std::string> getCookiesReturn = {};
    EXPECT_CALL(*httpRespone, GetCookies()).WillRepeatedly(Return(getCookiesReturn));

    IHttpClientMock* httpClient = new(std::nothrow) IHttpClientMock();
    testing::Mock::AllowLeak(httpClient);
    EXPECT_CALL(*httpClient, SendRequest(_,_)).WillRepeatedly(Return(httpRespone));
    EXPECT_CALL(*httpClient, SendRequest(_,_)).WillRepeatedly(Return(httpRespone));
    return httpClient;
}

static Module::IHttpClient* Stub_CreateClient_SendRequest_Success_ServerErr()
{
    std::shared_ptr<IHttpResponseMock> httpRespone = std::make_shared<IHttpResponseMock>();
    testing::Mock::AllowLeak(httpRespone.get());
    EXPECT_CALL(*httpRespone, Success()).WillRepeatedly(Return(false));
    EXPECT_CALL(*httpRespone, GetStatusCode()).WillRepeatedly(Return(111));
    EXPECT_CALL(*httpRespone, GetErrCode()).WillRepeatedly(Return(111));
    EXPECT_CALL(*httpRespone, GetErrString()).WillRepeatedly(Return("Server failed"));
    EXPECT_CALL(*httpRespone, Busy()).WillRepeatedly(Return(false));
    EXPECT_CALL(*httpRespone, GetHttpStatusCode()).WillRepeatedly(Return(0));
    EXPECT_CALL(*httpRespone, GetHttpStatusDescribe()).WillRepeatedly(Return(""));
    std::set<std::string> GetHeadByNameReturn = {};
    EXPECT_CALL(*httpRespone, GetHeadByName(_)).WillRepeatedly(Return(GetHeadByNameReturn));
    EXPECT_CALL(*httpRespone, GetBody()).WillRepeatedly(Return(""));
    std::map<std::string, std::set<std::string> > getHeadersReturn = {};
    EXPECT_CALL(*httpRespone, GetHeaders()).WillRepeatedly(Return(getHeadersReturn));
    std::set<std::string> getCookiesReturn = {};
    EXPECT_CALL(*httpRespone, GetCookies()).WillRepeatedly(Return(getCookiesReturn));

    IHttpClientMock* httpClient = new(std::nothrow) IHttpClientMock();
    testing::Mock::AllowLeak(httpClient);
    EXPECT_CALL(*httpClient, SendRequest(_,_)).WillRepeatedly(Return(httpRespone));
    EXPECT_CALL(*httpClient, SendRequest(_,_)).WillRepeatedly(Return(httpRespone));
    return httpClient;
}

static Module::IHttpClient* Stub_CreateClient_SendRequest_FAILED_when_netErr()
{
    std::shared_ptr<IHttpResponseMock> httpRespone = std::make_shared<IHttpResponseMock>();
    testing::Mock::AllowLeak(httpRespone.get());
    EXPECT_CALL(*httpRespone, Success()).WillRepeatedly(Return(false));
    EXPECT_CALL(*httpRespone, GetStatusCode()).WillRepeatedly(Return(g_netErr_statusCode));
    EXPECT_CALL(*httpRespone, GetErrCode()).WillRepeatedly(Return(111));
    EXPECT_CALL(*httpRespone, GetErrString()).WillRepeatedly(Return("send request failed"));
    EXPECT_CALL(*httpRespone, Busy()).WillRepeatedly(Return(false));
    EXPECT_CALL(*httpRespone, GetHttpStatusCode()).WillRepeatedly(Return(0));
    EXPECT_CALL(*httpRespone, GetHttpStatusDescribe()).WillRepeatedly(Return(""));
    std::set<std::string> GetHeadByNameReturn = {};
    EXPECT_CALL(*httpRespone, GetHeadByName(_)).WillRepeatedly(Return(GetHeadByNameReturn));
    EXPECT_CALL(*httpRespone, GetBody()).WillRepeatedly(Return(""));
    std::map<std::string, std::set<std::string> > getHeadersReturn = {};
    EXPECT_CALL(*httpRespone, GetHeaders()).WillRepeatedly(Return(getHeadersReturn));
    std::set<std::string> getCookiesReturn = {};
    EXPECT_CALL(*httpRespone, GetCookies()).WillRepeatedly(Return(getCookiesReturn));

    IHttpClientMock* httpClient = new(std::nothrow) IHttpClientMock();
    testing::Mock::AllowLeak(httpClient);
    EXPECT_CALL(*httpClient, SendRequest(_,_)).WillRepeatedly(Return(httpRespone));
    return httpClient;
}

static Module::IHttpClient* Stub_CreateClient_SendRequest_FAILED_when_httpResponseIsNull()
{
    IHttpClientMock* httpClient = new(std::nothrow) IHttpClientMock();
    testing::Mock::AllowLeak(httpClient);
    EXPECT_CALL(*httpClient, SendRequest(_,_)).WillRepeatedly(Return(nullptr));
    return httpClient;
}

static Module::IHttpClient* Stub_CreateClient_SendRequest_FAILED_when_httpClientIsNull()
{
    return nullptr;
}

/*
 * 测试用例： 客户端发送成功
 * 前置条件： 发送接口返回FAILED
 * CHECK点： 调用发送接口成功
 */
TEST_F(HttpClientTest, SendSuccess)
{
    HttpClient hcsHttpClient;
    Module::HttpRequest request;
    std::shared_ptr<ResponseModelMock> response = std::make_shared<ResponseModelMock>();
    stub.set(ADDR(Module::IHttpClient, GetInstance), Stub_CreateClient_SendRequest_Success);
    int32_t ret = hcsHttpClient.Send(request, response);
    EXPECT_EQ(ret, SUCCESS);
    stub.reset(ADDR(Module::IHttpClient, GetInstance));
}

/*
 * 测试用例： 客户端发送成功,非网络错误，服务器处理异常
 * 前置条件： 发送接口返回SUCCESS
 * CHECK点： 调用发送接口成功
 */
TEST_F(HttpClientTest, SendSuccessButServerErr)
{
    HttpClient hcsHttpClient;
    Module::HttpRequest request;
    std::shared_ptr<ResponseModelMock> response = std::make_shared<ResponseModelMock>();
    stub.set(ADDR(Module::IHttpClient, GetInstance), Stub_CreateClient_SendRequest_Success_ServerErr);
    int32_t ret = hcsHttpClient.Send(request, response);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(response->GetErrString(), std::string("Server failed"));
    stub.reset(ADDR(Module::IHttpClient, GetInstance));
}

/*
 * 测试用例： 客户端发送失败，网络错误
 * 前置条件： 发送接口返回FAILED
 * CHECK点： 调用发送接口失败
 */
TEST_F(HttpClientTest, SendFailedWhenNetErr)
{
    HttpClient hcsHttpClient;
    Module::HttpRequest request;
    std::shared_ptr<ResponseModelMock> response = std::make_shared<ResponseModelMock>();
    std::map<int, int> codeMap = {{0, 0}, {1, 500}, {2, 502}, {3, 503}};
    for (int i=0; i < 4; i++) {
        g_netErr_statusCode = codeMap.find(i)->second;
        stub.set(ADDR(Module::IHttpClient, GetInstance), Stub_CreateClient_SendRequest_FAILED_when_netErr);
        int32_t ret = hcsHttpClient.Send(request, response);
        EXPECT_EQ(ret, FAILED);
    }
    stub.reset(ADDR(Module::IHttpClient, GetInstance));
}

/*
 * 测试用例： 客户端发送失败，httpRespone
 * 前置条件： httpRespone 返回为nullptr
 * CHECK点： 调用发送接口失败
 */
TEST_F(HttpClientTest, SendFailedWhenHttpResponseIsNull)
{
    HttpClient hcsHttpClient;
    Module::HttpRequest request;
    std::shared_ptr<ResponseModelMock> response = std::make_shared<ResponseModelMock>();
    stub.set(ADDR(Module::IHttpClient, GetInstance), Stub_CreateClient_SendRequest_FAILED_when_httpResponseIsNull);
    int32_t ret = hcsHttpClient.Send(request, response);
    EXPECT_EQ(ret, FAILED);
    stub.reset(ADDR(Module::IHttpClient, GetInstance));
}

/*
 * 测试用例： 创建客户端发送实例失败
 * 前置条件： IHttpClient::GetInstance返回为nullptr
 * CHECK点： 调用发送接口失败
 */
TEST_F(HttpClientTest, SendFailedWhenHttpCreateHttpClientFailed)
{
    HttpClient hcsHttpClient;
    Module::HttpRequest request;
    std::shared_ptr<ResponseModelMock> response = std::make_shared<ResponseModelMock>();
    stub.set(ADDR(Module::IHttpClient, GetInstance), Stub_CreateClient_SendRequest_FAILED_when_httpClientIsNull);
    int32_t ret = hcsHttpClient.Send(request, response);
    EXPECT_EQ(ret, FAILED);
    stub.reset(ADDR(Module::IHttpClient, GetInstance));
}
}
