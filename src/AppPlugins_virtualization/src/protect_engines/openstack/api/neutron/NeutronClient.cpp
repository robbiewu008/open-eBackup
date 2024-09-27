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
#include "NeutronClient.h"
#include "protect_engines/openstack/utils/OpenStackTokenMgr.h"

namespace {
const std::string MODULE_NAME = "NeutronClient";
using Defer = std::shared_ptr<void>;
}

OPENSTACK_PLUGIN_NAMESPACE_BEGIN

bool NeutronClient::CheckParams(ModelBase &model)
{
    if (!model.UserInfoIsSet()) {
        ERRLOG("User info does not set.");
        return false;
    }
    return true;
}
bool NeutronClient::UpdateToken(ModelBase &request, std::string &tokenStr)
{
    if (!OpenStackTokenMgr::GetInstance().ReacquireToken(request, tokenStr)) {
        ERRLOG("Get token failed.");
        return false;
    }
    return true;
}

bool NeutronClient::GetTokenEndPoint(ModelBase &request, std::string &tokenStr, std::string &endpoint)
{
    if (!OpenStackTokenMgr::GetInstance().GetToken(request, tokenStr, endpoint)) {
        ERRLOG("Get token failed.");
        return false;
    }
    return true;
}

std::shared_ptr<GetNetworksResponse> NeutronClient::GetNetworks(GetNetworksRequest &request)
{
    if (!CheckParams(request)) {
        return nullptr;
    }
    RequestInfo requestInfo;
    requestInfo.m_method = "GET";
    requestInfo.m_resourcePath = "{endpoint}/v2.0/networks";
    std::string endpoint;
    std::string tokenStr;
    Defer _(nullptr, [&](...) { Module::CleanMemoryPwd(tokenStr); });
    if (!OpenStackTokenMgr::GetInstance().GetToken(request, tokenStr, endpoint)) {
        ERRLOG("Get token failed.");
        return nullptr;
    }
    requestInfo.m_pathParams["endpoint"] = std::move(endpoint);
    requestInfo.m_queryParams = {};
    requestInfo.m_headerParams["X-Auth-Token"] = std::move(tokenStr);
    requestInfo.m_auth = request.GetUserInfo();
    std::shared_ptr<GetNetworksResponse> response = std::make_shared<GetNetworksResponse>();
    if (CallApi(requestInfo, response, request) != SUCCESS) {
        ERRLOG("Get netwoks response failed, errorCode:%d, errorString:%s", response->GetErrCode(),
            response->GetErrString().c_str());
        return nullptr;
    }
    if (!response->Serial()) {
        ERRLOG("Get netwoks serial failed.");
        return nullptr;
    }
    return response;
}

OPENSTACK_PLUGIN_NAMESPACE_END