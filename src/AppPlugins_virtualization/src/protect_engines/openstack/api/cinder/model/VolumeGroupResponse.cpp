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
#include "VolumeGroupResponse.h"
OPENSTACK_PLUGIN_NAMESPACE_BEGIN

VolumeGroupResponse::VolumeGroupResponse() {}
VolumeGroupResponse::~VolumeGroupResponse() {}

bool VolumeGroupResponse::Serial()
{
    if (m_body.empty()) {
        ERRLOG("VolumeGroupResponse body is empty.");
        return false;
    }
    if (!Module::JsonHelper::JsonStringToStruct(m_body, m_group)) {
        ERRLOG("Failed to trans data from json string to group.");
        return false;
    }
    return true;
}

const VolumeGroupResponseBodyMsg& VolumeGroupResponse::GetGroupResponseBody() const
{
    return m_group;
}

std::string VolumeGroupResponse::GetGroupID() const
{
    return m_group.m_group.m_id;
}
 
std::string VolumeGroupResponse::GetGroupStatus() const
{
    return m_group.m_group.m_status;
}

OPENSTACK_PLUGIN_NAMESPACE_END
