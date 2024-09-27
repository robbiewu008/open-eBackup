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
#ifndef OPENSTACK_KEYSTONE_CLIENT_H
#define OPENSTACK_KEYSTONE_CLIENT_H
#include "common/client/RestClient.h"
#include "common/token_mgr/GetTokenResponse.h"
#include "protect_engines/openstack/api/keystone/model/GetTokenRequest.h"
#include "protect_engines/openstack/api/keystone/model/GetDomainProjectsRequest.h"
#include "protect_engines/openstack/api/keystone/model/GetDomainProjectsResponse.h"
#include "protect_engines/openstack/api/keystone/model/GetServicesRequest.h"
#include "protect_engines/openstack/api/keystone/model/GetServicesResponse.h"
#include "protect_engines/openstack/api/keystone/model/GetDomainsRequest.h"
#include "protect_engines/openstack/api/keystone/model/GetDomainsResponse.h"
#include "protect_engines/openstack/utils/OpenStackTokenMgr.h"
using VirtPlugin::RestClient;
using VirtPlugin::ModelBase;
using VirtPlugin::GetTokenResponse;
using VirtPlugin::Scope;
using VirtPlugin::RequestInfo;
using VirtPlugin::SUCCESS;

OPENSTACK_PLUGIN_NAMESPACE_BEGIN

class KeyStoneClient : public RestClient {
public:
    KeyStoneClient() {};
    ~KeyStoneClient() {};

    bool CheckParams(ModelBase &model);
    std::shared_ptr<GetTokenResponse> GetToken(GetTokenRequest &request);
    std::shared_ptr<GetDomainProjectsResponse> GetDomainProjects(GetDomainProjectsRequest &request);
    std::shared_ptr<GetServicesResponse> GetServices(GetServicesRequest &request);
    std::shared_ptr<GetDomainsResponse> GetDomains(GetDomainsRequest &request);

protected:
    bool UpdateToken(ModelBase &request, std::string &tokenStr) override;

private:
    std::string BuildQueryBody(const GetTokenRequest &request);
    Json::Value GenDomain(const GetTokenRequest &request);
    Json::Value GenUserInfo(const GetTokenRequest &request);
    Json::Value GenPwdInfo(const GetTokenRequest &request);
    Json::Value GenIdentity(const GetTokenRequest &request);
    Json::Value GenAuth(const GetTokenRequest &request);
    Json::Value GenProject(const GetTokenRequest &request);
    Json::Value GenScope(const GetTokenRequest &request);
};
OPENSTACK_PLUGIN_NAMESPACE_END
#endif