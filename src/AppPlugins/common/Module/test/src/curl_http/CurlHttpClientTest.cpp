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
#include <list>
#include <cstdio>
#include <algorithm>
#include <curl/curl.h>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <openssl/ssl.h>
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "llt_stub/stub.h"
#include "llt_stub/addr_pri.h"
#include "curl_http/CurlHttpClient.h"
#include "log/Log.h"
#include "curl_http/HttpStatus.h"
#include "system/System.hpp"
#include "common/CleanMemPwd.h"
#include "define/Types.h"

namespace CurlHttpTest {
class CurlHttpClientTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
};

void CurlHttpClientTest::SetUp()
{}

void CurlHttpClientTest::TearDown()
{}

void CurlHttpClientTest::SetUpTestCase()
{}

void CurlHttpClientTest::TearDownTestCase()
{}


void SendTwoWayCertRequestMock(const Module::HttpRequest& req, const uint32_t timeOut) {}

CURL* CurlEasyInitMock()
{
    return NULL;
}

CURLcode CurlEasyPerform_OK(CURL *easy_handle)
{
    return CURLE_OK;
}

bool PerformUpload_OK(const std::string& attachmentPath)
{
    return true;
}

FILE* Fopen_OK(const char* path, const char* mode)
{
    std::shared_ptr<char> ptemp = std::make_shared<char>(10);
    return (FILE*)((void*)ptemp.get());
}

char* Realpath_OK(const char* path, char* rest)
{
    std::shared_ptr<char> ptemp = std::make_shared<char>(10);
    return ptemp.get();
}

int Fstat_OK(const int fileHandle, const struct stat* stStat)
{
    std::cout << "Comming Fstsa" << std::endl;
    return Module::SUCCESS;
}

int Fileno_OK(FILE *stream)
{
    return Module::SUCCESS;
}

int Fclose_OK(FILE *stream)
{
    return Module::SUCCESS;
}

/*
 * 测试用例： 调用发送curl请求的SendRequest接口
 * 前置条件： CURL指针为空
 * CHECK点： 调用发送双向认证请求接口失败
 */
// TEST_F(CurlHttpClientTest, SendRequest_Falied)
// {
//     Module::IHttpClient* httpClient = Module::IHttpClient::GetInstance();
//     std::shared_ptr<Module::IHttpResponse> httpResponse = nullptr;
//     Module::HttpRequest req;
//     Stub stub;
//     stub.set(curl_easy_init, CurlEasyInitMock);
//     httpResponse = httpClient->SendRequest(req);
//     bool retValue = (httpResponse.get() == nullptr);

//     EXPECT_EQ(retValue, true);
//     stub.reset(curl_easy_init);
//     Module::IHttpClient::ReleaseInstance(httpClient);
// }

/*
 * 测试用例： 调用发送curl请求的SendRequest接口
 * 前置条件： CURL指针不为空
 * CHECK点： 调用发送双向认证请求接口成功
 */
TEST_F(CurlHttpClientTest, SendRequest_CurlSuccess)
{
    std::shared_ptr<Module::IHttpResponse> httpResponse = nullptr;
    Module::IHttpClient* httpClient = Module::IHttpClient::GetInstance();
    Module::HttpRequest req;

    Stub stub;
    stub.set(curl_easy_perform, CurlEasyPerform_OK);
    httpResponse = httpClient->SendRequest(req);
    bool retValue = (httpResponse.get() == nullptr);
    EXPECT_EQ(retValue, false);
    stub.reset(curl_easy_perform);

    Module::IHttpClient::ReleaseInstance(httpClient);
}

/*
 * 测试用例： 调用发送双向认证请求接口
 * 前置条件： CURL指针为空
 * CHECK点： 调用发送双向认证请求接口失败
 */
// TEST_F(CurlHttpClientTest, SendMemCertRequest_Falied)
// {
//     Module::IHttpClient* httpClient = Module::IHttpClient::GetInstance();
//     std::shared_ptr<Module::IHttpResponse> httpResponse = nullptr;
//     Module::HttpRequest req;
//     Stub stub;
//     stub.set(curl_easy_init, CurlEasyInitMock);
//     httpResponse = httpClient->SendMemCertRequest(req);
//     bool retValue = (httpResponse.get() == nullptr);

//     EXPECT_EQ(retValue, true);
//     stub.reset(curl_easy_init);
//     Module::IHttpClient::ReleaseInstance(httpClient);
// }

/*
 * 测试用例： 调用发送双向认证请求接口
 * 前置条件： 调用生产环境curl成功
 * CHECK点： 调用发送双向认证请求接口成功
 */
TEST_F(CurlHttpClientTest, SendMemCertRequest_CurlSuccess)
{
    std::shared_ptr<Module::IHttpResponse> httpResponse = nullptr;
    Module::IHttpClient* httpClient = Module::IHttpClient::GetInstance();
    Module::HttpRequest req;

    Stub stub;
    stub.set(curl_easy_perform, CurlEasyPerform_OK);
    httpResponse = httpClient->SendMemCertRequest(req);
    bool retValue = (httpResponse.get() == nullptr);
    EXPECT_EQ(retValue, false);
    stub.reset(curl_easy_perform);

    Module::IHttpClient::ReleaseInstance(httpClient);
}

TEST_F(CurlHttpClientTest, DownloadAttchment_Failed)
{
    Module::IHttpClient* httpClient = Module::IHttpClient::GetInstance();
    std::shared_ptr<Module::IHttpResponse> httpResponse = nullptr;
    Module::HttpRequest req;

    httpResponse = httpClient->DownloadAttchment(req);
    int32_t retValue = httpResponse->GetErrCode();
    EXPECT_EQ(retValue, CURLE_URL_MALFORMAT);
    Module::IHttpClient::ReleaseInstance(httpClient);
}

TEST_F(CurlHttpClientTest, DownloadAttchment_Success)
{
    std::shared_ptr<Module::IHttpResponse> httpResponse = nullptr;
    Module::IHttpClient* httpClient = Module::IHttpClient::GetInstance();
    Module::HttpRequest req;

    Stub stub;
    stub.set(curl_easy_perform, CurlEasyPerform_OK);
    httpResponse = httpClient->DownloadAttchment(req);
    int32_t retValue = httpResponse->GetErrCode();
    EXPECT_EQ(retValue, CURLE_OK);
    stub.reset(curl_easy_perform);

    Module::IHttpClient::ReleaseInstance(httpClient);
}


TEST_F(CurlHttpClientTest, UploadAttachment_Failed)
{
    Module::IHttpClient* httpClient = Module::IHttpClient::GetInstance();
    std::shared_ptr<Module::IHttpResponse> httpResponse = nullptr;
    Module::HttpRequest req;

    bool retValue = httpClient->UploadAttachment(req, httpResponse);

    EXPECT_EQ(retValue, false);
    Module::IHttpClient::ReleaseInstance(httpClient);
}

}
