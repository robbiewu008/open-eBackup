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
#include "GetSnapshotListRequest.h"
#include <string>
 
OPENSTACK_PLUGIN_NAMESPACE_BEGIN
GetSnapshotListRequest::GetSnapshotListRequest()
{}
 
GetSnapshotListRequest::~GetSnapshotListRequest() {}
 
Scope GetSnapshotListRequest::GetScopeType() const
{
    return Scope::PROJECT;
}
 
ApiType GetSnapshotListRequest::GetApiType()
{
    return ApiType::CINDER;
}

std::string GetSnapshotListRequest::GetVolumeId() const
{
    return m_volumeId;
}

void GetSnapshotListRequest::SetVolumeId(const std::string &volumeId)
{
    m_volumeId = volumeId;
}
OPENSTACK_PLUGIN_NAMESPACE_END
