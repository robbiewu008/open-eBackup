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
#include <boost/algorithm/string/replace.hpp>
#include "curl_http/HttpStatus.h"
#include "common/Utils.h"
#include "common/utils/Utils.h"
#include "config_reader/ConfigIniReader.h"
#include "RestClient.h"

using namespace VirtPlugin;

namespace {
const std::string MODULE_NAME = "RestClient";
const std::string HTTP_RESPONSE_HEADER_TOKNE_FILED_NAME = "X-Auth-Token";
const std::string HTTP_RESPONSE_HEADER_SUB_TOKNE_FILED_NAME = "X-Subject-Token";
const int32_t SEND_HTTP_DELAY_TIME = 15;
const int32_t BASE_RETRY_TIMES = 1;
using Defer = std::shared_ptr<void>;
}

VIRT_PLUGIN_NAMESPACE_BEGIN
int32_t RestClient::CallApi(RequestInfo &requestInfo, std::shared_ptr<ResponseModel> response, ModelBase &model,
    bool isOpService)
{
    Module::HttpRequest request;
    request.method = requestInfo.m_method;
    request.url = GetResourcePath(requestInfo.m_resourcePath, requestInfo.m_pathParams);
    std::string queryParamsHttp = GetQueryParams(requestInfo.m_queryParams);
    if (!queryParamsHttp.empty()) {
        request.url += "?" + queryParamsHttp;
    }
    // 添加header
    AddHeaderParams(request, requestInfo.m_headerParams);
    // 添加body
    request.body = requestInfo.m_body;
    // 添加cert
    request.cert = requestInfo.m_auth.cert;
    request.revocationList = requestInfo.m_auth.revocationList;
    request.isVerify = requestInfo.m_auth.certVerifyEnable ? Module::CACertVerification::VCENTER_VERIFY
        : Module::CACertVerification::DO_NOT_VERIFY;
    Utils::InnerAgentAppointNetDevice(request, !model.GetToken().empty() || isOpService);
    if (DoHttpRequestSync(request, response, model) != SUCCESS) {
        ERRLOG("Failed to do http request, url: %s", request.url.c_str());
        return FAILED;
    }

    return SUCCESS;
}

int32_t RestClient::DoHttpRequestSync(Module::HttpRequest &request, std::shared_ptr<ResponseModel> response,
    ModelBase &model)
{
    int32_t retryCount = m_retryTimes;  // token过期重试
    HttpClient httpRestClient;
    int32_t result = FAILED;
    do {
        httpRestClient.SetTimeOut(Module::ConfigReader::getUint("General", "ConnectTimeOut"),
            Module::ConfigReader::getUint("General", "TotalTimeOut"));
        int ret = httpRestClient.Send(request, response, m_retryTimes);
        if (ret != SUCCESS) {
            ERRLOG("Failed to call rest api.");
            break;
        }
        if (response == nullptr) {
            ERRLOG("Response is null, send request faield.");
            return FAILED;
        }
        if (response->GetStatusCode() != static_cast<uint32_t>(Module::SC_UNAUTHORIZED) ||
            !model.GetNeedRetry()) {
            DBGLOG("Success to call rest api.");
            return SUCCESS;
        }
        // 发送成功但是状态为401是token错误
        std::string tokenStr;
        Defer _(nullptr, [&](...) { Module::CleanMemoryPwd(tokenStr); });
        if (!UpdateToken(model, tokenStr)) {
            ERRLOG("Failed to reacquire token, retry: %d", (retryCount - 1));
            retryCount--;
            sleep(SEND_HTTP_DELAY_TIME);
            continue;
        }
        std::set<std::pair<std::string, std::string> > copyHeads;
        for (auto it = request.heads.begin(); it != request.heads.end(); ++it) {
            if (it->first == HTTP_RESPONSE_HEADER_TOKNE_FILED_NAME ||
                it->first == HTTP_RESPONSE_HEADER_SUB_TOKNE_FILED_NAME) {
                continue;
            }
            copyHeads.insert(std::make_pair(it->first, it->second));
        }
        copyHeads.insert(std::make_pair(HTTP_RESPONSE_HEADER_TOKNE_FILED_NAME, tokenStr));
        copyHeads.insert(std::make_pair(HTTP_RESPONSE_HEADER_SUB_TOKNE_FILED_NAME, tokenStr));
        request.heads = copyHeads;
        retryCount--;
        sleep(SEND_HTTP_DELAY_TIME);
    } while (retryCount);
    return result;
}

std::string RestClient::GetResourcePath(const std::string &uri, const std::map<std::string, std::string> &pathParams)
{
    std::string res = uri;
    for (const auto &path : pathParams) {
        boost::replace_all(res, "{" + path.first + "}", path.second);
    }
    return res;
}

std::string RestClient::GetQueryParams(const std::map<std::string, std::string> &queryParams)
{
    std::string res;
    int index = 1;
    int paramSize = queryParams.size();
    for (const auto &query : queryParams) {
        res.append(query.first + "=" + query.second);
        if (index < paramSize) {
            res.append("&");
        }
        index++;
    }
    return res;
}

void RestClient::AddHeaderParams(Module::HttpRequest &request, const std::map<std::string, std::string> &headerParams)
{
    for (const auto &header : headerParams) {
        request.heads.insert(std::make_pair(header.first, header.second));
    }
}

void RestClient::SetRetryTimes(const int &retryTimes)
{
    if (retryTimes >= BASE_RETRY_TIMES) {
        m_retryTimes = retryTimes;
    }
}
VIRT_PLUGIN_NAMESPACE_END