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
#include "GetSnapshotResponse.h"

OPENSTACK_PLUGIN_NAMESPACE_BEGIN

GetSnapshotResponse::GetSnapshotResponse() {}
GetSnapshotResponse::~GetSnapshotResponse() {}

bool GetSnapshotResponse::Serial()
{
    if (m_body.empty()) {
        ERRLOG("GetSnapshotResponse body is empty.");
        return false;
    }
    if (!Module::JsonHelper::JsonStringToStruct(m_body, m_snapshot)) {
        ERRLOG("Failed to trans data from json string to server list.");
        return false;
    }
    return true;
}

SnapshotDetailsMsg GetSnapshotResponse::GetSnapshotDetails() const
{
    return m_snapshot;
}

bool GetSnapshotResponse::GroupSnapshotSerial()
{
    if (m_body.empty()) {
        ERRLOG("GetSnapshotResponse body is empty.");
        return false;
    }
    if (!Module::JsonHelper::JsonStringToStruct(m_body, m_groupSnapshot)) {
        ERRLOG("Failed to trans data from json string to server list.");
        return false;
    }
    return true;
}
 
GroupSnapShotDetail GetSnapshotResponse::GetGroupSnapshotDetails() const
{
    return m_groupSnapshot;
}
 
std::string GetSnapshotResponse::GetGroupSnapshotStatus() const
{
    return m_groupSnapshot.m_groupSnapshot.m_status;
}

std::string GetSnapshotResponse::GetSnapShotStatus() const
{
    return m_snapshot.m_snapshotDetails.m_status;
}

std::string GetSnapshotResponse::GetProviderAuth()
{
    return m_snapshot.m_snapshotDetails.m_storageProviderAuth;
}

OPENSTACK_PLUGIN_NAMESPACE_END
