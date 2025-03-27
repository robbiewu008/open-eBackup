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
#ifndef HTTP_CLIENT_H
#define HTTP_CLIENT_H

#include <cstdint>
#include "common/Macros.h"
#include "common/model/ModelBase.h"
#include "common/model/ResponseModel.h"
#include "curl_http/CurlHttpClient.h"
#include "curl_http/HttpClientInterface.h"

VIRT_PLUGIN_NAMESPACE_BEGIN

const int32_t SEND_HTTP_MAX_RETRY_TIMES = 6;
const uint32_t CONN_HTTP_TIME_OUT = 30; // unit - seconds
const uint32_t HTTP_TIME_OUT = 300; // unit - seconds

class HttpClient {
public:
    HttpClient() noexcept {}
    ~HttpClient() {}
    int32_t Send(const Module::HttpRequest &request, std::shared_ptr<ResponseModel> response,
        int32_t retryTimes = SEND_HTTP_MAX_RETRY_TIMES);
    void SetTimeOut(const uint32_t connTimeOut, const uint32_t timeOut);

protected:
    bool IsNetworkError(const uint32_t &statusCode);
    bool IsTimeoutError(const int32_t &errCode);
    bool IsServiceErr(const uint32_t &statusCode);
    bool SetResponse(std::shared_ptr<ResponseModel> response, std::shared_ptr<Module::IHttpResponse> httpRespone);
    uint32_t m_connTimeOut { CONN_HTTP_TIME_OUT };
    uint32_t m_timeOut { HTTP_TIME_OUT };
};
VIRT_PLUGIN_NAMESPACE_END

#endif