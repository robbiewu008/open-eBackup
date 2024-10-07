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
#include "CreateGroupSnapShotRequest.h"

OPENSTACK_PLUGIN_NAMESPACE_BEGIN

CreateGroupSnapShotRequest::CreateGroupSnapShotRequest() {}

CreateGroupSnapShotRequest::~CreateGroupSnapShotRequest() {}

Scope CreateGroupSnapShotRequest::GetScopeType() const
{
    return Scope::PROJECT;
}

ApiType CreateGroupSnapShotRequest::GetApiType()
{
    return ApiType::CINDER;
}

GroupSnapShotRequestBodyMsg CreateGroupSnapShotRequest::GetBody() const
{
    return m_body;
}

bool CreateGroupSnapShotRequest::BodyIsSet() const
{
    return m_bodyIsSet;
}

void CreateGroupSnapShotRequest::UnsetBody()
{
    m_bodyIsSet = false;
}

void CreateGroupSnapShotRequest::SetBody(const GroupSnapShotRequestBodyMsg& body)
{
    m_body = body;
    m_bodyIsSet = true;
}

std::string CreateGroupSnapShotRequest::GetApiVersion() const
{
    return m_apiVersion;
}

void CreateGroupSnapShotRequest::SetApiVersion(const std::string apiVersion)
{
    m_apiVersion = apiVersion;
    m_apiVersionIsSet = true;
}

bool CreateGroupSnapShotRequest::ApiVersionIsSet() const
{
    return m_apiVersionIsSet;
}

OPENSTACK_PLUGIN_NAMESPACE_END
