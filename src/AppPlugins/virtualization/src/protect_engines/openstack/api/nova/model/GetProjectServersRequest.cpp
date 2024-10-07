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
#include "GetProjectServersRequest.h"
#include <string>

OPENSTACK_PLUGIN_NAMESPACE_BEGIN

GetProjectServersRequest::GetProjectServersRequest() {}
GetProjectServersRequest::~GetProjectServersRequest() {}

Scope GetProjectServersRequest::GetScopeType() const
{
    return Scope::PROJECT;
}

ApiType GetProjectServersRequest::GetApiType()
{
    return ApiType::NOVA;
}

std::string GetProjectServersRequest::GetServerMarker() const
{
    return m_marker;
}

bool GetProjectServersRequest::ServerMarkerIsSet() const
{
    return m_markerIsSet;
}

void GetProjectServersRequest::UnsetServerMarker()
{
    m_markerIsSet = false;
}

void GetProjectServersRequest::SetServerMarker(const std::string& value)
{
    m_marker = value;
    m_markerIsSet = true;
}

int32_t GetProjectServersRequest::GetServerLimit() const
{
    return m_limit;
}

bool GetProjectServersRequest::ServerLimitIsSet() const
{
    return m_limitIsSet;
}

void GetProjectServersRequest::UnsetServerLimit()
{
    m_limitIsSet = false;
}

void GetProjectServersRequest::SetServerLimit(const int32_t& value)
{
    m_limit = value;
    m_limitIsSet = true;
}
OPENSTACK_PLUGIN_NAMESPACE_END
