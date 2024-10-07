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
#ifndef HCS_GET_SERVER_DETAILS_RESPONSE_H
#define HCS_GET_SERVER_DETAILS_RESPONSE_H

#include <vector>
#include "common/JsonHelper.h"
#include "ServerDetail.h"
#include "common/model/ResponseModel.h"
#include "protect_engines/hcs/common/HcsMacros.h"
#include "protect_engines/openstack/api/nova/model/ServerDetail.h"

using namespace VirtPlugin;
using OpenStackPlugin::ServerDetail;

namespace HcsPlugin {
class GetServerDetailsResponse : public ResponseModel {
public:
    GetServerDetailsResponse();
    ~GetServerDetailsResponse();
    bool Serial();
    ServerDetail GetServerDetails() const;
    std::string GetServerStatus() const;
    std::string GetOsExtstsVmState() const;
    std::string GetSoftDeleteFlag() const;

protected:
    ServerDetail m_serverDetails;

};
}

#endif