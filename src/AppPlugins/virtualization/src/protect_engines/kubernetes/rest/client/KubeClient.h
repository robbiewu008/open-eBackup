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
#ifndef VIRTUALIZATION_PLUGIN_KUBECLIENT_H
#define VIRTUALIZATION_PLUGIN_KUBECLIENT_H

#include <string>
#include "protect_engines/kubernetes/common/KubeMacros.h"
#include "protect_engines/kubernetes/common/KubeCommonInfo.h"
#include "curl_http/CurlHttpClient.h"
#include "curl_http/HttpClientInterface.h"

namespace KubernetesPlugin {

class KubeClient {
public:
    KubeClient();

    ~KubeClient();

    void InitHttpRequest(Module::HttpRequest &req);

    int32_t SendRequest(const Module::HttpRequest &httpParam, HttpResponseInfo &httpResponse);

    int32_t CheckAccessAuthentication(const AccessAuthParam &accessAuth, const std::string &url,
                                              HttpResponseInfo &httpResponse);

private:
    int32_t Send(const Module::HttpRequest &req, HttpResponseInfo &httpResponse);

    bool IsNetworkError(const uint32_t statusCode) const;
};
}

#endif // VIRTUALIZATION_PLUGIN_KUBECLIENT_H