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
#include "DeleteSnapshotRequest.h"
 
OPENSTACK_PLUGIN_NAMESPACE_BEGIN
 
DeleteSnapshotRequest::DeleteSnapshotRequest() {}
 
DeleteSnapshotRequest::~DeleteSnapshotRequest() {}
 
Scope DeleteSnapshotRequest::GetScopeType() const
{
    return m_scope;
}
 
ApiType DeleteSnapshotRequest::GetApiType()
{
    return m_apiType;
}
 
std::string DeleteSnapshotRequest::GetSnapshotId() const
{
    return m_snapshotId;
}
 
bool DeleteSnapshotRequest::SnapshotIdIsSet() const
{
    return m_snapshotIdIsSet;
}
 
void DeleteSnapshotRequest::UnsetSnapshotId()
{
    m_snapshotIdIsSet = false;
}
 
void DeleteSnapshotRequest::SetSnapshotId(const std::string& id)
{
    m_snapshotId = id;
    m_snapshotIdIsSet = true;
}

void DeleteSnapshotRequest::SetGroupSnapshotId(const std::string& groupSnapshotId)
{
    m_groupSnapshotId = groupSnapshotId;
}

std::string DeleteSnapshotRequest::GetGroupSnapshotId() const
{
    return m_groupSnapshotId;
}
 
void DeleteSnapshotRequest::SetApiVersion(const std::string &apiVersion)
{
    m_apiVersion = apiVersion;
}
 
std::string DeleteSnapshotRequest::GetApiVersion() const
{
    return m_apiVersion;
}
OPENSTACK_PLUGIN_NAMESPACE_END