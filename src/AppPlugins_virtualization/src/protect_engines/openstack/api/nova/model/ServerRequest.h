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
#ifndef OPENSTACK_SERVER_REQUEST_H
#define OPENSTACK_SERVER_REQUEST_H

#include <string>
#include "common/model/ModelBase.h"
#include "protect_engines/openstack/common/OpenStackMacros.h"
#include "common/token_mgr/TokenDetail.h"

OPENSTACK_PLUGIN_NAMESPACE_BEGIN

class ServerRequest : public VirtPlugin::ModelBase {
public:
    ServerRequest() {}
    ~ServerRequest() {}

    virtual VirtPlugin::Scope GetScopeType() const override
    {
        return VirtPlugin::Scope::PROJECT;
    }
    virtual VirtPlugin::ApiType GetApiType() override
    {
        return VirtPlugin::ApiType::NOVA;
    }

    std::string GetServerId() const
    {
        return m_serverId;
    }
    bool IsServerIdSet() const
    {
        return m_isServerIdSet;
    }
    void UnsetServerId()
    {
        m_isServerIdSet = false;
    }
    void SetServerId(const std::string& id)
    {
        m_serverId = id;
        m_isServerIdSet = true;
    }

protected:
    std::string m_serverId;
    bool m_isServerIdSet {false};
};

class DetachVolumeRequest : public ServerRequest {
public:
    DetachVolumeRequest() {}
    ~DetachVolumeRequest() {}
    virtual VirtPlugin::Scope GetScopeType() const override
    {
        return VirtPlugin::Scope::ADMIN_PROJECT;
    }
};
OPENSTACK_PLUGIN_NAMESPACE_END

#endif