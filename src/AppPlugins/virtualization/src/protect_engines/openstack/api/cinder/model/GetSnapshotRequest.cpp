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
#include "GetSnapshotRequest.h"
#include <string>

OPENSTACK_PLUGIN_NAMESPACE_BEGIN
GetSnapshotRequest::GetSnapshotRequest() : m_snapshotId(""), m_snapshotIdIsSet(false)
{}

GetSnapshotRequest::~GetSnapshotRequest() {}

Scope GetSnapshotRequest::GetScopeType() const
{
    return Scope::PROJECT;
}

ApiType GetSnapshotRequest::GetApiType()
{
    return ApiType::CINDER;
}

std::string GetSnapshotRequest::GetSnapshotId() const
{
    return m_snapshotId;
}

void GetSnapshotRequest::SetSnapshotId(const std::string& value)
{
    m_snapshotId = value;
    m_snapshotIdIsSet = true;
}

bool GetSnapshotRequest::SnapshotIdIsSet() const
{
    return m_snapshotIdIsSet;
}

void GetSnapshotRequest::UnsetSnapshotId()
{
    m_snapshotIdIsSet = false;
}

void GetSnapshotRequest::SetApiVersion(const std::string apiVersion)
{
    m_apiVersion = apiVersion;
}
 
std::string GetSnapshotRequest::GetApiVersion() const
{
    return m_apiVersion;
}
 
void GetSnapshotRequest::SetGroupSnapshotId(const std::string& groupSnapshotId)
{
    m_groupSnapshotId = groupSnapshotId;
}
 
std::string GetSnapshotRequest::GetGroupSnapshotId() const
{
    return m_groupSnapshotId;
}

OPENSTACK_PLUGIN_NAMESPACE_END
