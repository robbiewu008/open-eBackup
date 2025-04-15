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
#include "AttachServerVolumeRequest.h"

using namespace VirtPlugin;

OPENSTACK_PLUGIN_NAMESPACE_BEGIN
AttachServerVolumeRequest::AttachServerVolumeRequest()
    : m_serverId(""), m_serverIdIsSet(false), m_volumeId(""), m_volumeIdIsSet(false), m_device(""), m_deviceIsSet(false)
{}

AttachServerVolumeRequest::~AttachServerVolumeRequest() = default;

Scope AttachServerVolumeRequest::GetScopeType() const
{
    return Scope::ADMIN_PROJECT;
}

ApiType AttachServerVolumeRequest::GetApiType()
{
    return ApiType::NOVA;
}

std::string AttachServerVolumeRequest::GetServerId() const
{
    return m_serverId;
}

void AttachServerVolumeRequest::SetServerId(const std::string &value)
{
    m_serverId = value;
    m_serverIdIsSet = true;
}

bool AttachServerVolumeRequest::ServerIdIsSet() const
{
    return m_serverIdIsSet;
}

void AttachServerVolumeRequest::UnsetServerId()
{
    m_volumeIdIsSet = false;
}

std::string AttachServerVolumeRequest::GetVolumeId() const
{
    return m_volumeId;
}

void AttachServerVolumeRequest::SetVolumeId(const std::string &value)
{
    m_volumeId = value;
    m_volumeIdIsSet = true;
}

bool AttachServerVolumeRequest::VolumeIdIsSet() const
{
    return m_volumeIdIsSet;
}

void AttachServerVolumeRequest::UnsetVolumeId()
{
    m_volumeIdIsSet = false;
}

std::string AttachServerVolumeRequest::GetDevice() const
{
    return m_device;
}

void AttachServerVolumeRequest::SetDevice(const std::string &value)
{
    m_device = value;
    m_deviceIsSet = true;
}

bool AttachServerVolumeRequest::DeviceIsSet() const
{
    return m_deviceIsSet;
}

void AttachServerVolumeRequest::UnsetDevice()
{
    m_deviceIsSet = false;
}
OPENSTACK_PLUGIN_NAMESPACE_END