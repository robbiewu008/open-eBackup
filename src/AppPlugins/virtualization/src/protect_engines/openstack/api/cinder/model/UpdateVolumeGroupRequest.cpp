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
#include "UpdateVolumeGroupRequest.h"

OPENSTACK_PLUGIN_NAMESPACE_BEGIN

UpdateVolumeGroupRequest::UpdateVolumeGroupRequest() {}

UpdateVolumeGroupRequest::~UpdateVolumeGroupRequest() {}

Scope UpdateVolumeGroupRequest::GetScopeType() const
{
    return Scope::PROJECT;
}

ApiType UpdateVolumeGroupRequest::GetApiType()
{
    return ApiType::CINDER;
}

UpdateGroupRequestBodyMsg UpdateVolumeGroupRequest::GetBody() const
{
    return m_body;
}

bool UpdateVolumeGroupRequest::BodyIsSet() const
{
    return m_bodyIsSet;
}

void UpdateVolumeGroupRequest::UnsetBody()
{
    m_bodyIsSet = false;
}

void UpdateVolumeGroupRequest::SetBody(const UpdateGroupRequestBodyMsg& body)
{
    m_body = body;
    m_bodyIsSet = true;
}

void UpdateVolumeGroupRequest::SetGroupId(const std::string groupId)
{
    m_groupId = groupId;
}

std::string UpdateVolumeGroupRequest::GetGroupId() const
{
    return m_groupId;
}

std::string UpdateVolumeGroupRequest::GetApiVersion() const
{
    return m_apiVersion;
}

void UpdateVolumeGroupRequest::SetApiVersion(const std::string apiVersion)
{
    m_apiVersion = apiVersion;
    m_apiVersionIsSet = true;
}

bool UpdateVolumeGroupRequest::ApiVersionIsSet() const
{
    return m_apiVersionIsSet;
}

OPENSTACK_PLUGIN_NAMESPACE_END
