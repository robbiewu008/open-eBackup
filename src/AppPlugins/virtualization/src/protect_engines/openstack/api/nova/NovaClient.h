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
#ifndef OPENSTACK_NOVA_CLIENT_H
#define OPENSTACK_NOVA_CLIENT_H
 
#include "log/Log.h"
#include "protect_engines/openstack/api/nova/model/ServerDetail.h"
#include "protect_engines/openstack/api/nova/model/ServerRequest.h"
#include "protect_engines/openstack/api/nova/model/ServerResponse.h"
#include "protect_engines/openstack/api/nova/model/GetProjectServersRequest.h"
#include "protect_engines/openstack/api/nova/model/GetProjectServersResponse.h"
#include "protect_engines/openstack/api/nova/model/GetServerDetailsResponse.h"
#include "protect_engines/openstack/api/nova/model/GetServerDetailsRequest.h"
#include "protect_engines/openstack/api/nova/model/AttachServerVolumeRequest.h"
#include "protect_engines/openstack/api/nova/model/AttachServerVolumeResponse.h"
#include "protect_engines/openstack/api/nova/model/GetFlavorsDetailResponse.h"
#include "protect_engines/openstack/api/nova/model/GetFlavorsDetailRequest.h"
#include "protect_engines/openstack/api/nova/model/CreateServerRequest.h"
#include "protect_engines/openstack/api/nova/model/CreateServerResponse.h"
#include "protect_engines/openstack/api/nova/model/GetAvailabilityZonesResponse.h"
#include "protect_engines/openstack/utils/OpenStackTokenMgr.h"
#include "protect_engines/openstack/common/OpenStackMacros.h"
#include "common/client/RestClient.h"

OPENSTACK_PLUGIN_NAMESPACE_BEGIN

using namespace VirtPlugin;

class NovaClient : RestClient {
public:
    NovaClient() {};
    ~NovaClient() {};
    bool CheckParams(ModelBase &model) override;
    std::shared_ptr<GetServerDetailsResponse> GetServerDetails(GetServerDetailsRequest &request);
    std::shared_ptr<GetProjectServersResponse> GetProjectServers(GetProjectServersRequest &request);
    std::shared_ptr<AttachServerVolumeResponse> AttachServerVolume(AttachServerVolumeRequest &request);
    std::shared_ptr<ServerResponse> DetachServerVolume(DetachVolumeRequest &request, const std::string& volumeId);
    std::shared_ptr<ServerResponse> PowerOnServer(ServerRequest &request);
    std::shared_ptr<ServerResponse> PowerOffServer(ServerRequest &request);
    std::shared_ptr<ServerResponse> ActServer(ServerRequest &request, const std::string& requestType);
    std::shared_ptr<GetFlavorsDetailResponse> GetFlavorsDetail(GetFlavorsDetailRequest &request);
    std::shared_ptr<CreateServerResponse> CreateServer(CreateServerRequest &request);
    std::shared_ptr<ServerResponse> DeleteServer(ServerRequest &request);
    std::shared_ptr<ServerResponse> SwapServerVolume(ServerRequest &request, const std::string& oldVolumeId,
        const std::string& newVolumeId);
    std::shared_ptr<GetAvailabilityZonesResponse> GetAvailabilityZones(ServerRequest &request);

protected:
    virtual bool UpdateToken(ModelBase &request, std::string &tokenStr);
    virtual bool GetTokenEndPoint(ModelBase &request, std::string &tokenStr, std::string &endpoint);
};
 
OPENSTACK_PLUGIN_NAMESPACE_END

#endif