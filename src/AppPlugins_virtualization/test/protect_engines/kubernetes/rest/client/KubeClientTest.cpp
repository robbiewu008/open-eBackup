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
#include "gtest/gtest.h"
#include "stub.h"
#include "addr_pri.h"
#include "json/json.h"
#include "common/Constants.h"
#include "common/Structs.h"
#include "protect_engines/kubernetes/rest/client/KubeClient.h"
#include "protect_engines/kubernetes/common/KubeCommonInfo.h"
#include "common/JsonHelper.h"
#include "curl_http/HttpStatus.h"
#include "curl_http/HttpClientInterface.h"

using Module::HttpRequest;
using Module::IHttpResponse;
using Module::IHttpClient;
using Module::CurlHttpResponse;
using Module::CurlHttpClient;
using namespace KubernetesPlugin;
using Fptr = std::shared_ptr<IHttpResponse> (*)(CurlHttpClient*, const HttpRequest&, const uint32_t);

namespace HDT_TEST {
class KubeClientTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();

public:
    std::shared_ptr<KubeClient> m_kubeClient;
};

void KubeClientTest::SetUp()
{
    m_kubeClient = std::make_shared<KubeClient>();
}
void KubeClientTest::TearDown() {}
void KubeClientTest::SetUpTestCase() {}
void KubeClientTest::TearDownTestCase() {}

CURLcode CurlEasyPerform_OK(CURL* easyHandle)
{
    return CURLE_OK;
}

bool IsNetworkErrorOK(const uint32_t statusCode)
{
    return false;
}

void SleepStub()
{
    return;
}

static IHttpClient* Stub_GetInstance_Nullptr()
{
    IHttpClient* pHttpClient = nullptr;
    return pHttpClient;
}

static CURL* Stub_curl_easy_init_Nullptr()
{
    return nullptr;
}

static std::shared_ptr<IHttpResponse> Stub_SendMemCertRequest_Success(const HttpRequest& req, const uint32_t timeOut)
{
    std::shared_ptr<CurlHttpResponse> rsp = std::make_shared<CurlHttpResponse>();
    rsp->m_StatusCode = Module::SC_OK;
    rsp->m_ErrorCode = CURLE_OK;
    return rsp;
}

static std::shared_ptr<IHttpResponse> Stub_SendMemCertRequest_Status500(const HttpRequest& req, const uint32_t timeOut)
{
    std::shared_ptr<CurlHttpResponse> rsp = std::make_shared<CurlHttpResponse>();
    rsp->m_StatusCode = Module::SC_INTERNAL_SERVER_ERROR;
    rsp->m_ErrorCode = CURLE_OK;
    return rsp;
}

static std::shared_ptr<IHttpResponse> Stub_SendMemCertRequest_Status503(const HttpRequest& req, const uint32_t timeOut)
{
    std::shared_ptr<CurlHttpResponse> rsp = std::make_shared<CurlHttpResponse>();
    rsp->m_StatusCode = Module::SC_SERVICE_UNAVAILABLE;
    rsp->m_ErrorCode = CURLE_OK;
    return rsp;
}

static std::shared_ptr<IHttpResponse> Stub_SendMemCertRequest_Status504(const HttpRequest& req, const uint32_t timeOut)
{
    std::shared_ptr<CurlHttpResponse> rsp = std::make_shared<CurlHttpResponse>();
    rsp->m_StatusCode = Module::SC_GATEWAY_TIMEOUT;
    rsp->m_ErrorCode = CURLE_OK;
    return rsp;
}

static std::shared_ptr<IHttpResponse> Stub_SendMemCertRequest_Status505(const HttpRequest& req, const uint32_t timeOut)
{
    std::shared_ptr<CurlHttpResponse> rsp = std::make_shared<CurlHttpResponse>();
    rsp->m_StatusCode = Module::SC_HTTP_VERSION_NOT_SUPPORTED;
    rsp->m_ErrorCode = CURLE_OK;
    return rsp;
}

/*
 * 测试用例： 发送rest请求
 * 前置条件： 内存加载双向证书成功
 * CHECK点： rest请求结果成功
 */
TEST_F(KubeClientTest, SendMemCertRequest_Success)
{
    Module::HttpRequest httpReqest;
    HttpResponseInfo response;

    Stub stub;
    stub.set(curl_easy_perform, CurlEasyPerform_OK);
    stub.set(ADDR(KubeClient, IsNetworkError), IsNetworkErrorOK);
    m_kubeClient->InitHttpRequest(httpReqest);
    int32_t retValue = m_kubeClient->SendRequest(httpReqest, response);

    EXPECT_EQ(retValue, Module::SUCCESS);
    stub.reset(curl_easy_perform);
    stub.reset(ADDR(KubeClient, IsNetworkError));
}

/*
 * 测试用例： 发送rest请求
 * 前置条件： 获取httpClient指针为空
 * CHECK点： 发送rest请求失败
 */
TEST_F(KubeClientTest, SendRequest_Falied)
{
    Module::HttpRequest httpReqest;
    HttpResponseInfo response;

    Stub stub;
    stub.set(ADDR(IHttpClient, GetInstance), Stub_GetInstance_Nullptr);
    m_kubeClient->InitHttpRequest(httpReqest);
    int32_t retValue = m_kubeClient->SendRequest(httpReqest, response);
    EXPECT_EQ(retValue, Module::FAILED);
}

/*
 * 测试用例： 发送rest请求
 * 前置条件： 初始化Curl失败，指针为空
 * CHECK点： 发送rest请求失败
 */
TEST_F(KubeClientTest, SendRequest_CurlNullptr)
{
    Module::HttpRequest httpReqest;
    HttpResponseInfo response;

    Stub stub;
    stub.set(curl_easy_init, Stub_curl_easy_init_Nullptr);
    m_kubeClient->InitHttpRequest(httpReqest);
    int32_t retValue = m_kubeClient->SendRequest(httpReqest, response);
    EXPECT_EQ(retValue, Module::FAILED);
}

/*
 * 测试用例： 发送rest请求
 * 前置条件： 发送双向认证请求成功
 * CHECK点： 发送rest请求成功
 */
TEST_F(KubeClientTest, SendRequest_StatusSuccess)
{
    Fptr sendFunc = (Fptr)(&CurlHttpClient::SendMemCertRequest);
    Module::HttpRequest httpReqest;
    HttpResponseInfo response;

    Stub stub;
    stub.set(sendFunc, Stub_SendMemCertRequest_Success);
    m_kubeClient->InitHttpRequest(httpReqest);
    int32_t retValue = m_kubeClient->SendRequest(httpReqest, response);
    EXPECT_EQ(retValue, Module::SUCCESS);
}

/*
 * 测试用例： 发送rest请求
 * 前置条件： 网络状态原因500
 * CHECK点： 发送rest请求失败
 */
TEST_F(KubeClientTest, SendRequest_Status500)
{
    Fptr sendFunc = (Fptr)(&CurlHttpClient::SendMemCertRequest);
    Module::HttpRequest httpReqest;
    HttpResponseInfo response;

    Stub stub;
    stub.set(sendFunc, Stub_SendMemCertRequest_Status500);
    stub.set(sleep, SleepStub);
    m_kubeClient->InitHttpRequest(httpReqest);
    int32_t retValue = m_kubeClient->SendRequest(httpReqest, response);
    EXPECT_EQ(retValue, Module::FAILED);
}

/*
 * 测试用例： 发送rest请求
 * 前置条件： 网络状态原因503
 * CHECK点： 发送rest请求失败
 */
TEST_F(KubeClientTest, SendRequest_Status503)
{
    Fptr sendFunc = (Fptr)(&CurlHttpClient::SendMemCertRequest);
    Module::HttpRequest httpReqest;
    HttpResponseInfo response;

    Stub stub;
    stub.set(sendFunc, Stub_SendMemCertRequest_Status503);
    stub.set(sleep, SleepStub);
    m_kubeClient->InitHttpRequest(httpReqest);
    int32_t retValue = m_kubeClient->SendRequest(httpReqest, response);
    EXPECT_EQ(retValue, Module::FAILED);
}

/*
 * 测试用例： 发送rest请求
 * 前置条件： 网络状态原因504
 * CHECK点： 发送rest请求失败
 */
TEST_F(KubeClientTest, SendRequest_Status504)
{
    Fptr sendFunc = (Fptr)(&CurlHttpClient::SendMemCertRequest);
    Module::HttpRequest httpReqest;
    HttpResponseInfo response;

    Stub stub;
    stub.set(sendFunc, Stub_SendMemCertRequest_Status504);
    stub.set(sleep, SleepStub);
    m_kubeClient->InitHttpRequest(httpReqest);
    int32_t retValue = m_kubeClient->SendRequest(httpReqest, response);
    EXPECT_EQ(retValue, Module::FAILED);
}

/*
 * 测试用例： 发送rest请求
 * 前置条件： 非网络状态原因505，不重试
 * CHECK点： 发送rest请求成功
 */
TEST_F(KubeClientTest, SendRequest_Status505)
{
    Fptr sendFunc = (Fptr)(&CurlHttpClient::SendMemCertRequest);
    Module::HttpRequest httpReqest;
    HttpResponseInfo response;

    Stub stub;
    stub.set(sendFunc, Stub_SendMemCertRequest_Status505);
    m_kubeClient->InitHttpRequest(httpReqest);
    int32_t retValue = m_kubeClient->SendRequest(httpReqest, response);
    EXPECT_EQ(retValue, Module::SUCCESS);
}

/*
 * 测试用例： 发送rest请求
 * 前置条件： 内存加载双向证书失败
 * CHECK点： rest请求结果失败
 */
TEST_F(KubeClientTest, SendMemCertRequest_Falied)
{
    Module::HttpRequest httpReqest;
    HttpResponseInfo response;

    Stub stub;
    stub.set(curl_easy_perform, CurlEasyPerform_OK);
    stub.set(sleep, SleepStub);
    m_kubeClient->InitHttpRequest(httpReqest);
    int32_t retValue = m_kubeClient->SendRequest(httpReqest, response);

    EXPECT_EQ(retValue, Module::FAILED);
    stub.reset(curl_easy_perform);
    stub.reset(sleep);
}

/*
 * 测试用例： 获取存储设备iBaseToken
 * 前置条件： 用户名错误
 * CHECK点： 获取存储设备iBaseToken失败
 */
TEST_F(KubeClientTest, CheckAccessAuthentication_Falied1)
{
    Module::HttpRequest httpReqest;
    HttpResponseInfo response;
    AccessAuthParam accs("admin", "Admin@12345", "0");
    std::string url = "https://8.40.111.70:8088/deviceManager/rest/xxxxx/sessions";

    Stub stub;
    stub.set(curl_easy_perform, CurlEasyPerform_OK);
    stub.set(sleep, SleepStub);
    int32_t retValue = m_kubeClient->CheckAccessAuthentication(accs, url, response);

    EXPECT_EQ(retValue, Module::FAILED);
    stub.reset(curl_easy_perform);
    stub.reset(sleep);
}

/*
 * 测试用例： 获取存储设备iBaseToken
 * 前置条件： 密码错误
 * CHECK点： 获取存储设备iBaseToken失败
 */
TEST_F(KubeClientTest, CheckAccessAuthentication_Falied2)
{
    Module::HttpRequest httpReqest;
    HttpResponseInfo response;
    AccessAuthParam accs("admin", "Admin@12345678", "0");
    std::string url = "https://8.40.111.70:8088/deviceManager/rest/xxxxx/sessions";

    Stub stub;
    stub.set(curl_easy_perform, CurlEasyPerform_OK);
    stub.set(sleep, SleepStub);
    int32_t retValue = m_kubeClient->CheckAccessAuthentication(accs, url, response);

    EXPECT_EQ(retValue, Module::FAILED);
    stub.reset(curl_easy_perform);
    stub.reset(sleep);
}

/*
 * 测试用例： 获取存储设备iBaseToken
 * 前置条件： 存储API错误
 * CHECK点： 获取存储设备iBaseToken失败
 */

TEST_F(KubeClientTest, CheckAccessAuthentication_Falied3)
{
    Module::HttpRequest httpReqest;
    HttpResponseInfo response;
    AccessAuthParam accs("admin", "Admin@12345", "0");
    std::string url = "https://8.40.111.70:8088/deviceManager/rest/xxxxx/sessions/test";

    Stub stub;
    stub.set(curl_easy_perform, CurlEasyPerform_OK);
    stub.set(sleep, SleepStub);
    int32_t retValue = m_kubeClient->CheckAccessAuthentication(accs, url, response);

    EXPECT_EQ(retValue, Module::FAILED);
    stub.reset(curl_easy_perform);
    stub.reset(sleep);
}

/*
 * 测试用例： 获取存储设备iBaseToken
 * 前置条件： 信息全为空
 * CHECK点： 获取存储设备iBaseToken失败
 */

TEST_F(KubeClientTest, CheckAccessAuthentication_MessageNullFalied)
{
    Module::HttpRequest httpReqest;
    HttpResponseInfo response;
    AccessAuthParam accs("", "", "0");
    std::string url = "";

    Stub stub;
    stub.set(curl_easy_perform, CurlEasyPerform_OK);
    stub.set(sleep, SleepStub);
    int32_t retValue = m_kubeClient->CheckAccessAuthentication(accs, url, response);

    EXPECT_EQ(retValue, Module::FAILED);
    stub.reset(curl_easy_perform);
    stub.reset(sleep);
}

/*
 * 测试用例： 获取存储设备iBaseToken
 * 前置条件： 存储API错误
 * CHECK点： 获取存储设备iBaseToken成功
 */
TEST_F(KubeClientTest, CheckAccessAuthentication_Success)
{
    Module::HttpRequest httpReqest;
    HttpResponseInfo response;
    AccessAuthParam accs("admin", "Admin@12345", "0");
    std::string url = "https://8.40.111.70:8088/deviceManager/rest/xxxxx/sessions";
    
    Stub stub;
    stub.set(curl_easy_perform, CurlEasyPerform_OK);
    stub.set(ADDR(KubeClient, IsNetworkError), IsNetworkErrorOK);
    int32_t retValue = m_kubeClient->CheckAccessAuthentication(accs, url, response);
    EXPECT_EQ(retValue, Module::SUCCESS);
    stub.reset(curl_easy_perform);
    stub.reset(ADDR(KubeClient, IsNetworkError));
}

IHttpClient* Stub_GetInstance_Null()
{
    return nullptr;
}
/**
 * 测试用例：Send接口调用失败
 * 前置条件：IHTTPClient::GetInstance接口返回为空
 * Check点：iRet返回值是否为Failed
 */
TEST_F(KubeClientTest, Send_Failed_WhenCannotGetInstance)
{
    Stub stub;
    stub.set(ADDR(IHttpClient, GetInstance), Stub_GetInstance_Null);
    HttpRequest request;
    HttpResponseInfo responseInfo;
    int32_t retValue = m_kubeClient->Send(request, responseInfo);

    EXPECT_EQ(retValue, Module::FAILED);
}

std::shared_ptr<IHttpResponse> Stub_CurlHttpClient_SendMemCertRequest_RetNull(void *obj, const HttpRequest& req, const uint32_t timeOut)
{
    return nullptr;
}

typedef std::shared_ptr<IHttpResponse> (*SendMemCertRequestPtr) (Module::CurlHttpClient *obj, const HttpRequest& req, const uint32_t timeOut);
/**
 * 测试用例：Send接口调用失败
 * 前置条件：httpClient->SendMemCertRequest无返回Response
 * Check点：iRet返回值是否为Failed
 */
TEST_F(KubeClientTest, Send_Failed_WhenSendMemCertRequestNoResponse)
{
    Stub stub;
    SendMemCertRequestPtr A_SendMemCertRequest = (SendMemCertRequestPtr)(&Module::CurlHttpClient::SendMemCertRequest);

    stub.set(A_SendMemCertRequest, Stub_CurlHttpClient_SendMemCertRequest_RetNull);
    HttpRequest request;
    HttpResponseInfo responseInfo;
    int32_t retValue = m_kubeClient->Send(request, responseInfo);

    EXPECT_EQ(retValue, Module::FAILED);
}

std::shared_ptr<IHttpResponse> Stub_CurlHttpClient_SendMemCertRequest_RetSuccess(void *obj, const HttpRequest& req, const uint32_t timeOut)
{
    auto retPtr = std::make_shared<Module::CurlHttpResponse>();
    return retPtr;
}

bool Stub_IHttpResponse_Success(void *obj)
{
    return true;
}
typedef bool (*SuccessPtr) (Module::CurlHttpResponse *obj);

/**
 * 测试用例：Send接口调用成功
 * 前置条件：dpaHttpRespone->Success()返回成功
 * Check点：iRet返回值是否为Success
 */
TEST_F(KubeClientTest, Send_Success_WhenGetHttpStatusCodeSuccess)
{
    Stub stub;
    SendMemCertRequestPtr A_SendMemCertRequest = (SendMemCertRequestPtr)(&Module::CurlHttpClient::SendMemCertRequest);
    stub.set(A_SendMemCertRequest, Stub_CurlHttpClient_SendMemCertRequest_RetSuccess);

    SuccessPtr A_SuccessPtr = (SuccessPtr)(&Module::CurlHttpResponse::Success);
    stub.set(A_SuccessPtr, Stub_IHttpResponse_Success);
    HttpRequest request;
    HttpResponseInfo responseInfo;
    int32_t retValue = m_kubeClient->Send(request, responseInfo);

    EXPECT_EQ(retValue, Module::SUCCESS);
}

/**
 * 测试用例：isNetworkError调用成功
 * 前置条件：statusCode为SC_BAD_GATEWAY、SC_SERVICE_UNAVAILABLE、SC_GATEWAY_TIMEOUT、SC_INTERNAL_SERVER_ERROR、 0
 * Check点：iRet返回值是否为True
 */
TEST_F(KubeClientTest, isNetworkError_ReturnTrue)
{
    EXPECT_EQ(true, m_kubeClient->IsNetworkError(Module::SC_BAD_GATEWAY));
    EXPECT_EQ(true, m_kubeClient->IsNetworkError(Module::SC_SERVICE_UNAVAILABLE));
    EXPECT_EQ(true, m_kubeClient->IsNetworkError(Module::SC_GATEWAY_TIMEOUT));
    EXPECT_EQ(true, m_kubeClient->IsNetworkError(Module::SC_INTERNAL_SERVER_ERROR));
    EXPECT_EQ(true, m_kubeClient->IsNetworkError(0));
}

/**
 * 测试用例：isNetworkError调用成功
 * 前置条件：statusCode非SC_BAD_GATEWAY、SC_SERVICE_UNAVAILABLE、SC_GATEWAY_TIMEOUT、SC_INTERNAL_SERVER_ERROR、 0
 * Check点：iRet返回值是否为False
 */
TEST_F(KubeClientTest, isNetworkError_ReturnFalse)
{
    EXPECT_EQ(false, m_kubeClient->IsNetworkError(123));
}

}