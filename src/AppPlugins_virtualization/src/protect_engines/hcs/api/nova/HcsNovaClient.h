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
#ifndef HCS_NOVA_CLIENT_H
#define HCS_NOVA_CLIENT_H
 
#include "log/Log.h"
#include "protect_engines/hcs/api/ecs/model/GetServerListResponse.h"
#include "protect_engines/hcs/api/ecs/model/GetServerListRequest.h"
#include "protect_engines/hcs/utils/HCSTokenMgr.h"
#include "protect_engines/hcs/common/HcsMacros.h"
#include "protect_engines/openstack/api/nova/NovaClient.h"

using namespace VirtPlugin;
using OpenStackPlugin::NovaClient;
namespace HcsPlugin {
class HcsNovaClient : public NovaClient {
public:
    HcsNovaClient() {};
    ~HcsNovaClient() {};
protected:
    bool UpdateToken(ModelBase &request, std::string &tokenStr);
    bool GetTokenEndPoint(ModelBase &request, std::string &tokenStr, std::string &endpoint) override;
};
 
}

#endif