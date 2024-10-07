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
#include "GroupTypeRequest.h"

OPENSTACK_PLUGIN_NAMESPACE_BEGIN

GroupTypeRequest::GroupTypeRequest() {}

GroupTypeRequest::~GroupTypeRequest() {}

Scope GroupTypeRequest::GetScopeType() const
{
    return Scope::PROJECT;
}

ApiType GroupTypeRequest::GetApiType()
{
    return ApiType::CINDER;
}

GroupTypeRequestBodyMsg GroupTypeRequest::GetBody() const
{
    return m_body;
}

bool GroupTypeRequest::BodyIsSet() const
{
    return m_bodyIsSet;
}

void GroupTypeRequest::UnsetBody()
{
    m_bodyIsSet = false;
}

void GroupTypeRequest::SetBody(const GroupTypeRequestBodyMsg& body)
{
    m_body = body;
    m_bodyIsSet = true;
}

void GroupTypeRequest::SetGroupTypeId(const std::string &groupTypeId)
{
    m_groupTypeId = groupTypeId;
}
 
std::string GroupTypeRequest::GetGroupTypeId() const
{
    return m_groupTypeId;
}
 
std::string GroupTypeRequest::GetApiVersion() const
{
    return m_apiVersion;
}
 
void GroupTypeRequest::SetApiVersion(const std::string apiVersion)
{
    m_apiVersion = apiVersion;
}

OPENSTACK_PLUGIN_NAMESPACE_END
