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
#ifndef GET_NETWORKS_RESPONSE_H
#define GET_NETWORKS_RESPONSE_H

#include <string>
#include "common/model/ResponseModel.h"
#include "common/token_mgr/TokenDetail.h"
#include "protect_engines/openstack/api/neutron/model/NetworkDetail.h"
#include "protect_engines/openstack/common/OpenStackMacros.h"
using VirtPlugin::ResponseModel;

OPENSTACK_PLUGIN_NAMESPACE_BEGIN

class GetNetworksResponse : public ResponseModel {
public:
    GetNetworksResponse();
    ~GetNetworksResponse();

    bool Serial();
    Networks GetNetworks() const;

protected:
    Networks m_networks;
};

OPENSTACK_PLUGIN_NAMESPACE_END
 
#endif