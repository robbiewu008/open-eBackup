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
#include "UpdateVolumeBootableRequest.h"
#include <string>

OPENSTACK_PLUGIN_NAMESPACE_BEGIN
UpdateVolumeBootableRequest::UpdateVolumeBootableRequest() : m_volumeId(""), m_volumeIdIsSet(false)
{}

UpdateVolumeBootableRequest::~UpdateVolumeBootableRequest() {}

Scope UpdateVolumeBootableRequest::GetScopeType() const
{
    return Scope::PROJECT;
}

ApiType UpdateVolumeBootableRequest::GetApiType()
{
    return ApiType::CINDER;
}

std::string UpdateVolumeBootableRequest::GetVolumeId() const
{
    return m_volumeId;
}

void UpdateVolumeBootableRequest::SetVolumeId(const std::string& value)
{
    m_volumeId = value;
    m_volumeIdIsSet = true;
}

bool UpdateVolumeBootableRequest::VolumeIdIsSet() const
{
    return m_volumeIdIsSet;
}

void UpdateVolumeBootableRequest::UnsetVolumeId()
{
    m_volumeIdIsSet = false;
}

OPENSTACK_PLUGIN_NAMESPACE_END