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
#include "common/httpclient/HttpClient.h"
#include "common/Constants.h"
#include "common/Utils.h"
#include "curl_http/HttpStatus.h"

using namespace VirtPlugin;

namespace {
const std::string MODULE_NAME = "HttpClient";
const int32_t SEND_HTTP_MAX_RETRY_TIMES = 3;
const uint32_t HTTP_TIME_OUT = 30; // s
const uint32_t SEND_HTTP_DELAY_TIME = 3;  // s
const uint32_t SLEEP_TWENTY_SECONDS = 20;
const int32_t REQUEST_TIMEOUT_ERRCODE = 28;
}

VIRT_PLUGIN_NAMESPACE_BEGIN
int32_t HttpClient::Send(const Module::HttpRequest &request, std::shared_ptr<ResponseModel> response,
    int32_t retryTimes)
{
    Module::IHttpClient* httpClient = Module::IHttpClient::GetInstance();
    if (httpClient == nullptr) {
        ERRLOG("Initialize httpClient failed.");
        return FAILED;
    }

    int32_t result = FAILED;
    std::shared_ptr<Module::IHttpResponse> httpRespone = nullptr;
    while (retryTimes > 0) {
        httpRespone = httpClient->SendRequest(request, HTTP_TIME_OUT);
        if (httpRespone.get() == nullptr) {
            ERRLOG("HttpRespone is null, retry num=%d", retryTimes);
            retryTimes--;
            continue;
        }
        DBGLOG("Http response info, statusCode: %d errCode: %d errDesc: %s httpStatutsCode: %d",
            httpRespone->GetStatusCode(), httpRespone->GetErrCode(), httpRespone->GetErrString().c_str(),
            httpRespone->GetHttpStatusCode());
        if (httpRespone->Success() || httpRespone->GetStatusCode() == static_cast<uint32_t>(Module::SC_ACCEPTED)) {
            result = SUCCESS;
            break;
        } else {
            ERRLOG("Http response statusCode: %d , errCode: %d , errDesc: %s , response body: %s",
                httpRespone->GetStatusCode(), httpRespone->GetErrCode(), httpRespone->GetErrString().c_str(),
                httpRespone->GetBody().c_str());
            // 网络原因失败，重试
            if (IsNetworkError(httpRespone->GetStatusCode()) || IsTimeoutError(httpRespone->GetErrCode())) {
                retryTimes--;
                DBGLOG("Net issue retry left %d times.", retryTimes);
                sleep(SEND_HTTP_DELAY_TIME);
                continue;
            } else if (IsServiceErr(httpRespone->GetStatusCode())) {
                retryTimes--;
                DBGLOG("Service issue retry left %d times.", retryTimes);
                sleep(SLEEP_TWENTY_SECONDS);
                continue;
            }
            result = SUCCESS;
            break;
        }
    }
    if (!SetResponse(response, httpRespone)) {
        ERRLOG("Failed to get http data from response.");
        result = FAILED;
    }
    Module::IHttpClient::ReleaseInstance(httpClient);
    return result;
}

bool HttpClient::IsNetworkError(const uint32_t &statusCode)
{
    std::map<uint32_t, bool> netErrMap = {{0, true},
        {static_cast<uint32_t>(Module::SC_BAD_GATEWAY), true}};
    auto it = netErrMap.find(statusCode);
    if (it != netErrMap.end()) {
        return it->second;
    }
    return false;
}

bool HttpClient::IsTimeoutError(const int32_t &errCode)
{
    std::map<int32_t, bool> timeoutErrMap = { {REQUEST_TIMEOUT_ERRCODE, true} };
    auto it = timeoutErrMap.find(errCode);
    if (it != timeoutErrMap.end()) {
        return it->second;
    }
    return false;
}

bool HttpClient::IsServiceErr(const uint32_t &statusCode)
{
    std::map<uint32_t, bool> serviceErrMap = {{0, true},
        {static_cast<uint32_t>(Module::SC_INTERNAL_SERVER_ERROR), true},
        {static_cast<uint32_t>(Module::SC_SERVICE_UNAVAILABLE), true}};
    auto it = serviceErrMap.find(statusCode);
    if (it != serviceErrMap.end()) {
        return it->second;
    }
    return false;
}

bool HttpClient::SetResponse(std::shared_ptr<ResponseModel> response,
    std::shared_ptr<Module::IHttpResponse> httpRespone)
{
    if (response.get() == nullptr) {
        ERRLOG("Respone is null.");
        return false;
    }
    if (httpRespone.get() == nullptr) {
        ERRLOG("HttpRespone is null.");
        return false;
    }
    response->SetSuccess(httpRespone->Success());
    response->SetBusy(httpRespone->Busy());
    response->SetHttpStatusCode(httpRespone->GetHttpStatusCode());
    response->SetHttpStatusDescribe(httpRespone->GetHttpStatusDescribe());
    response->SetErrCode(httpRespone->GetErrCode());
    response->SetErrString(httpRespone->GetErrString());
    response->SetGetBody(httpRespone->GetBody());
    std::map<std::string, std::set<std::string> > headers = httpRespone->GetHeaders();
    response->SetHeaders(headers);
    std::set<std::string> cookies = httpRespone->GetCookies();
    response->SetCookies(cookies);
    response->SetStatusCode(httpRespone->GetStatusCode());
    return true;
}
VIRT_PLUGIN_NAMESPACE_END