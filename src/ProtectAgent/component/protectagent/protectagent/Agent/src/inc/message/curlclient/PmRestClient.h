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
#ifndef PM_REST_CLIENT_H
#define PM_REST_CLIENT_H
#include <vector>
#include "common/Defines.h"
#include "message/curlclient/HttpClientInterface.h"
#include "message/rest/http_status.h"
#include "message/curlclient/RestClientCommon.h"
#include "message/curlclient/DmeRestClient.h"

class BaseRestClient {
public:
    mp_int32 SendReq(const HttpReqCommonParam& httpParam, HttpResponse& httpResponse);
private:
    mp_int32 UpdateUrlIp(HttpRequest& req, const mp_string& ip, const HttpReqCommonParam &httpParam);
    mp_int32 ReceiveCheckStatus(HttpResponse& httpResponse, HttpRequest req);
};

class PmRestClient : public BaseRestClient {
public:
    static PmRestClient& GetInstance()
    {
        static PmRestClient instance;
        return instance;
    }
    ~PmRestClient() {};

    mp_int32 SendRequest(HttpReqCommonParam& httpParam, HttpResponse& httpResponse);
    void SetTimeOut(int timeout);
    
private:
    PmRestClient() {};
    mp_int32 GetPMIPandPort(std::vector<std::string>& pmIps, std::string& pmPort);
    std::vector<std::string> GetConnectIps(const std::vector<std::string>& ips, const std::string& port);

private:
    int m_timeout = 0;
};
#endif