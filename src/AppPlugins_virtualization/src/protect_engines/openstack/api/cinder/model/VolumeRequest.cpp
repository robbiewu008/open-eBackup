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
#include "VolumeRequest.h"
#include <string>

OPENSTACK_PLUGIN_NAMESPACE_BEGIN

VolumeRequest::VolumeRequest() {}
VolumeRequest::~VolumeRequest() {}

Scope VolumeRequest::GetScopeType() const
{
    return Scope::PROJECT;
}

ApiType VolumeRequest::GetApiType()
{
    return ApiType::CINDER;
}

void VolumeRequest::SetNewVolumeInfo(const VolumeRequestBody& confInfo)
{
    m_newVolumeConfInfo = confInfo;
}

bool VolumeRequest::BuildCreateBody(std::string& reqBody)
{
    if (!Module::JsonHelper::StructToJsonString(m_newVolumeConfInfo, reqBody)) {
        ERRLOG("Convert new VolumeConfInfo to json string failed");
        return false;
    }
    return true;
}

std::string VolumeRequest::GetVolumeId() const
{
    return m_volumeId;
}

void VolumeRequest::SetVolumeId(const std::string& volumeId)
{
    m_volumeId = volumeId;
}

void VolumeRequest::SetBody(std::string body)
{
    m_body = body;
}
 
std::string VolumeRequest::GetBody()
{
    return m_body;
}

OPENSTACK_PLUGIN_NAMESPACE_END
