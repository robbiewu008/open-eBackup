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
#ifndef OPENSTACK_NEUTRON_CLIENT_H
#define OPENSTACK_NEUTRON_CLIENT_H
 
#include "log/Log.h"
#include "protect_engines/openstack/utils/OpenStackTokenMgr.h"
#include "protect_engines/openstack/common/OpenStackMacros.h"
#include "protect_engines/openstack/api/neutron/model/GetNetworksRequest.h"
#include "protect_engines/openstack/api/neutron/model/GetNetworksResponse.h"
#include "common/client/RestClient.h"

using VirtPlugin::RestClient;
using VirtPlugin::RequestInfo;
using VirtPlugin::SUCCESS;

OPENSTACK_PLUGIN_NAMESPACE_BEGIN


class NeutronClient : RestClient {
public:
    NeutronClient() {};
    ~NeutronClient() {};
    bool CheckParams(ModelBase &model) override;

    std::shared_ptr<GetNetworksResponse> GetNetworks(GetNetworksRequest &request);

protected:
    virtual bool UpdateToken(ModelBase &request, std::string &tokenStr);
    virtual bool GetTokenEndPoint(ModelBase &request, std::string &tokenStr, std::string &endpoint);
};

OPENSTACK_PLUGIN_NAMESPACE_END

#endif