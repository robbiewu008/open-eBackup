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
#include "OpenStorageApiClient.h"

VIRT_PLUGIN_NAMESPACE_BEGIN

namespace {
const std::string MODULE_NAME = "OpenStorageApiClient";
const int32_t RETRY_TIMES = 5;
}

std::shared_ptr<UpdateIpRoutePolicyResponse> OpenStorageApiClient::AddIpPolicy(UpdateIpRoutePolicyRequest &req)
{
    INFOLOG("Enter");
    Module::HttpRequest httpReq;
    httpReq.method = "POST";
    httpReq.url = "https://protectengine.dpa.svc.cluster.local:30173/v1/internal/deviceManager/rest/ip_rule/add";
    httpReq.isVerify = Module::CACertVerification::INTERNAL_VERIFY;
    if (!req.BuildRequestBody(httpReq.body)) {
        ERRLOG("Failed to build request body.");
        return nullptr;
    }

    std::shared_ptr<UpdateIpRoutePolicyResponse> response = std::make_shared<UpdateIpRoutePolicyResponse>();
    if (response == nullptr) {
        ERRLOG("Create UpdateIpRoutePolicyResponse pointer failed.");
        return nullptr;
    }

    if (Send(httpReq, response, RETRY_TIMES) != SUCCESS) {
        ERRLOG("Send http request failed.");
        return nullptr;
    }

    if (!response->Serial()) {
        ERRLOG("Serialize response body failed!");
        return nullptr;
    }

    DBGLOG("Call AddIpPolicy success!");
    return response;
}

VIRT_PLUGIN_NAMESPACE_END