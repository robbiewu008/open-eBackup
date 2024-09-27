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
#include "VolumeGroupRequest.h"

OPENSTACK_PLUGIN_NAMESPACE_BEGIN

VolumeGroupRequest::VolumeGroupRequest() {}

VolumeGroupRequest::~VolumeGroupRequest() {}

Scope VolumeGroupRequest::GetScopeType() const
{
    return Scope::PROJECT;
}

ApiType VolumeGroupRequest::GetApiType()
{
    return ApiType::CINDER;
}

VolumeGroupRequestBodyMsg VolumeGroupRequest::GetBody() const
{
    return m_body;
}

bool VolumeGroupRequest::BodyIsSet() const
{
    return m_bodyIsSet;
}

void VolumeGroupRequest::UnsetBody()
{
    m_bodyIsSet = false;
}

void VolumeGroupRequest::SetBody(const VolumeGroupRequestBodyMsg& body)
{
    m_body = body;
    m_bodyIsSet = true;
}

void VolumeGroupRequest::SetApiVersion(const std::string apiVersion)
{
    m_apiVersion = apiVersion;
}

std::string VolumeGroupRequest::GetApiVersion() const
{
    return m_apiVersion;
}

bool VolumeGroupRequest::ApiVersionIsSet() const
{
    return  m_apiVersionIsSet;
}

void VolumeGroupRequest::SetGroupId(const std::string groupId)
{
    m_groupId = groupId;
}
 
std::string VolumeGroupRequest::GetGroupId() const
{
    return m_groupId;
}

OPENSTACK_PLUGIN_NAMESPACE_END
