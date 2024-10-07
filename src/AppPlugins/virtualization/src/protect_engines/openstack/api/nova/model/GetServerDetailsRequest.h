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
#ifndef OPENSTACK_GET_SERVER_DETAILS_REQUEST_H
#define OPENSTACK_GET_SERVER_DETAILS_REQUEST_H
 
#include <string>
#include "common/model/ModelBase.h"
#include "protect_engines/openstack/common/OpenStackMacros.h"
#include "common/token_mgr/TokenDetail.h"

OPENSTACK_PLUGIN_NAMESPACE_BEGIN
 
class GetServerDetailsRequest : public VirtPlugin::ModelBase {
public:
    GetServerDetailsRequest();
    ~GetServerDetailsRequest();
 
    virtual VirtPlugin::Scope GetScopeType() const override;
    VirtPlugin::ApiType GetApiType() override;
 
    std::string GetServerId() const;
    bool ServerIdIsSet() const;
    void UnsetServerId();
    void SetServerId(const std::string& value);
 
protected:
    std::string m_serverId;
    bool m_serverIdIsSet;
};
 
OPENSTACK_PLUGIN_NAMESPACE_END
 
#endif