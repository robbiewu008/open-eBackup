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
#ifndef OPENSTACK_NETWORK_DETAIL_H
#define OPENSTACK_NETWORK_DETAIL_H
 
#include <string>
#include <vector>
#include <json/json.h>
#include <common/JsonHelper.h>
#include "protect_engines/openstack/common/OpenStackMacros.h"

OPENSTACK_PLUGIN_NAMESPACE_BEGIN

struct Network {
    std::string m_id;
    std::string m_name;
    bool  m_adminStateUp = {false};
    bool m_shared = {false};
    std::string m_status;
    std::string m_projectId;    // 默认域service项目id
    std::string m_providerNetworkType;    // 此网络映射到的物理网络的类型。例如，平面、vlan、vxlan或gre。有效值取决于网络后端
    std::string m_providerPhysicalNetwork;    // 实施此网络/网段的物理网络

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_id, id)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_name, name)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_adminStateUp, admin_state_up)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_shared, shared)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_status, status)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_projectId, project_id)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_providerNetworkType, provider:network_type)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_providerPhysicalNetwork, provider:physical_network)
    END_SERIAL_MEMEBER
};

struct NetworkInfo {
    Network m_network;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_network, network)
    END_SERIAL_MEMEBER
};

struct Networks {
    std::vector<Network> m_networks;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_networks, networks)
    END_SERIAL_MEMEBER
};


OPENSTACK_PLUGIN_NAMESPACE_END
#endif