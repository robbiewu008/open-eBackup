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
#include "DeleteVolumeGroupRequest.h"
 
OPENSTACK_PLUGIN_NAMESPACE_BEGIN
 
DeleteVolumeGroupRequest::DeleteVolumeGroupRequest() {}
 
DeleteVolumeGroupRequest::~DeleteVolumeGroupRequest() {}
 
Scope DeleteVolumeGroupRequest::GetScopeType() const
{
    return Scope::PROJECT;
}
 
ApiType DeleteVolumeGroupRequest::GetApiType()
{
    return ApiType::CINDER;
}
 
DeleteVolumeGroupRequestBodyMsg DeleteVolumeGroupRequest::GetBody() const
{
    return m_body;
}
 
void DeleteVolumeGroupRequest::SetBody(const DeleteVolumeGroupRequestBodyMsg& body)
{
    m_body = body;
}
 
void DeleteVolumeGroupRequest::SetApiVersion(const std::string apiVersion)
{
    m_apiVersion = apiVersion;
}
 
std::string DeleteVolumeGroupRequest::GetApiVersion() const
{
    return m_apiVersion;
}
 
void DeleteVolumeGroupRequest::SetGroupId(const std::string& groupId)
{
    m_groupId = groupId;
}
 
std::string DeleteVolumeGroupRequest::GetGroupId()
{
    return m_groupId;
}
 
OPENSTACK_PLUGIN_NAMESPACE_END
