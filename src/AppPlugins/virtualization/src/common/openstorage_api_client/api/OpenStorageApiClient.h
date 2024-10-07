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
#ifndef __OPENSTORAGE_API_CLIENT_H__
#define __OPENSTORAGE_API_CLIENT_H__

#include <string>
#include "common/Constants.h"
#include "common/Macros.h"
#include "common/httpclient/HttpClient.h"
#include "common/openstorage_api_client/api/network/model/UpdateIpRoutePolicyRequest.h"
#include "common/openstorage_api_client/api/network/model/UpdateIpRoutePolicyResponse.h"

VIRT_PLUGIN_NAMESPACE_BEGIN

class OpenStorageApiClient : public HttpClient {
public:
    OpenStorageApiClient() = default;
    virtual ~OpenStorageApiClient() = default;

    std::shared_ptr<UpdateIpRoutePolicyResponse> AddIpPolicy(UpdateIpRoutePolicyRequest &req);
};

VIRT_PLUGIN_NAMESPACE_END

#endif // __OPENSTORAGE_API_CLIENT_H__