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
#ifndef GET_AVAILABILITY_ZONES_RESPONSE_H
#define GET_AVAILABILITY_ZONES_RESPONSE_H
 
#include <string>
#include "common/model/ResponseModel.h"
#include "common/token_mgr/TokenDetail.h"
#include "protect_engines/openstack/common/OpenStackMacros.h"
#include "protect_engines/openstack/common/Structs.h"

OPENSTACK_PLUGIN_NAMESPACE_BEGIN
 
class GetAvailabilityZonesResponse : public VirtPlugin::ResponseModel {
public:
    GetAvailabilityZonesResponse() {}
    ~GetAvailabilityZonesResponse() {}
 
    bool Serial() {
        if (m_body.empty()) {
            ERRLOG("Null response body.");
            return false;
        }
        if (!Module::JsonHelper::JsonStringToStruct(m_body, m_availabilityZones)) {
            ERRLOG("Failed to trans data from json string to struct.");
            return false;
        }
        return true;
    }

    AvailabilityZoneInfoList GetAvailabilityZones() const
    {
        return m_availabilityZones;
    }

protected:
    AvailabilityZoneInfoList m_availabilityZones;
};
 
OPENSTACK_PLUGIN_NAMESPACE_END
 
#endif