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
#include "GetServerDetailsRequest.h"
#include "common/token_mgr/TokenDetail.h"

using namespace VirtPlugin;

OPENSTACK_PLUGIN_NAMESPACE_BEGIN
 
GetServerDetailsRequest::GetServerDetailsRequest() : m_serverId(""), m_serverIdIsSet(false)
{}
 
GetServerDetailsRequest::~GetServerDetailsRequest() {}
 
Scope GetServerDetailsRequest::GetScopeType() const
{
    return Scope::PROJECT;
}
 
ApiType GetServerDetailsRequest::GetApiType()
{
    return ApiType::NOVA;
}
 
std::string GetServerDetailsRequest::GetServerId() const
{
    return m_serverId;
}
 
void GetServerDetailsRequest::SetServerId(const std::string& value)
{
    m_serverId = value;
    m_serverIdIsSet = true;
}
 
bool GetServerDetailsRequest::ServerIdIsSet() const
{
    return m_serverIdIsSet;
}
 
void GetServerDetailsRequest::UnsetServerId()
{
    m_serverIdIsSet = false;
}

OPENSTACK_PLUGIN_NAMESPACE_END
