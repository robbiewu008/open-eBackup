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
#ifdef LINUX
#include "common/Log.h"
#include "common/Defines.h"
#include "message/curlclient/OSAClient.h"

namespace {
const std::string OSA_SERVICE_PORT = "30173";
const std::string OSA_IP_POLICY_URI = "/v1/internal/deviceManager/rest/ip_rule/";
const std::set<std::string> VALID_OPER = { "add", "delete" };
}

int32_t OSAClient::ModifyIpPolicy(const IpPolicyParams &reqParams)
{
    DBGLOG("Enter");
    if (!CheckIpPolicyParams(reqParams)) {
        ERRLOG("Invalid Parameters.");
        return MP_FAILED;
    }

    HttpRequest req;
    // https://{IP}:30173/v1/internal/deviceManager/rest/ip_rule/{oper}
    req.url = "https://" + m_osaIp + ":" + OSA_SERVICE_PORT + OSA_IP_POLICY_URI + reqParams.oper;
    INFOLOG("req.url: %s", req.url.c_str());
    req.method = "POST";
    req.caInfo = INTERNAL_CA_FILE;
    req.sslCert = INTERNAL_CERT_FILE;
    req.sslKey = INTERNAL_KEY_FILE;
    /* inter-container communication uses container network, not bind to device */
    req.notBindToDevice = true;

    if (!BuildRequestBody<IpPolicyParams>(reqParams, req.body)) {
        ERRLOG("Convert request parameters to json string failed.");
        return MP_FAILED;
    }

    Json::Value rsp;
    bool iRet = SendHttpRequest(req, rsp);
    if (!iRet || (!rsp.isMember("error") && !rsp["error"].isMember("code") &&
        rsp["error"]["code"].asInt() != 0)) {
        int32_t errorCode = rsp["error"]["code"].asInt();
        ERRLOG("add or delete ip %s failed, error :%d", reqParams.destinationIp.c_str(), errorCode);
        return errorCode;
    }
    DBGLOG("Add or delete ip route for %s success!", reqParams.destinationIp.c_str());
    return MP_SUCCESS;
}

bool OSAClient::CheckIpPolicyParams(const IpPolicyParams &reqParams)
{
    if (m_osaIp.empty()) {
        ERRLOG("Invalid Parameters: empty osa ip.");
        return false;
    }
    if (VALID_OPER.find(reqParams.oper) == VALID_OPER.end()) {
        ERRLOG("Invalid operation: %s, should be: \"add\" or \"delete\"", reqParams.oper.c_str());
        return false;
    }
    return true;
}

template<typename T>
bool OSAClient::BuildRequestBody(const T &reqParams, std::string &body)
{
    T reqBody = reqParams;
    if (!JsonHelper::StructToJsonString(reqBody, body)) {
        ERRLOG("Convert request body to json string failed");
        return false;
    }
    return true;
}
#endif