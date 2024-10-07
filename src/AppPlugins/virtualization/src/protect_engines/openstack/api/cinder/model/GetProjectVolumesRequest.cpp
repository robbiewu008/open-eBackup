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
#include "GetProjectVolumesRequest.h"
#include <string>

OPENSTACK_PLUGIN_NAMESPACE_BEGIN

GetProjectVolumesRequest::GetProjectVolumesRequest() {}
GetProjectVolumesRequest::~GetProjectVolumesRequest() {}

Scope GetProjectVolumesRequest::GetScopeType() const
{
    return Scope::PROJECT;
}

ApiType GetProjectVolumesRequest::GetApiType()
{
    return ApiType::CINDER;
}

int32_t GetProjectVolumesRequest::GetVolumeOffSet() const
{
    return m_offset;
}
void GetProjectVolumesRequest::SetVolumeOffset(int32_t offset)
{
    m_offset = offset;
    m_offstIsSet = true;
}
bool GetProjectVolumesRequest::GetVolumeOffsetIsSet() const
{
    return m_offstIsSet;
}
int32_t GetProjectVolumesRequest::GetVolumeLimit() const
{
    return m_limit;
}
void GetProjectVolumesRequest::SetVolumeLimit(int32_t limit)
{
    m_limit = limit;
    m_limitIsSet = true;
}
bool GetProjectVolumesRequest::GetVolumeLimitIsSet() const
{
    return m_limitIsSet;
}

void GetProjectVolumesRequest::SetSnapShotId(const std::string &snapshotId)
{
    m_snapshotId = snapshotId;
}

std::string GetProjectVolumesRequest::GetSnapShotId() const
{
    return m_snapshotId;
}

void GetProjectVolumesRequest::SetVolumeStatus(const std::string &status)
{
    m_volumeStatus = status;
}
std::string GetProjectVolumesRequest::GetVolumeStatus() const
{
    return m_volumeStatus;
}
OPENSTACK_PLUGIN_NAMESPACE_END
