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
#include <common/JsonHelper.h>
#include "CreateGroupSnapShotResponse.h"
 
OPENSTACK_PLUGIN_NAMESPACE_BEGIN
 
CreateGroupSnapShotResponse::CreateGroupSnapShotResponse() {}
 
CreateGroupSnapShotResponse::~CreateGroupSnapShotResponse() {}
 
bool CreateGroupSnapShotResponse::Serial()
{
    if (m_body.empty()) {
        ERRLOG("CreateGroupSnapShotResponse body is empty.");
        return false;
    }
    if (!Module::JsonHelper::JsonStringToStruct(m_body, m_groupSnapshot)) {
        ERRLOG("Failed to trans data from json string to snapshot.");
        return false;
    }
    return true;
}
 
const GroupSnapShotResponseBody& CreateGroupSnapShotResponse::GetGroupSnapShotDetails() const
{
    return m_groupSnapshot.m_groupSnapshot;
}
 
const std::string CreateGroupSnapShotResponse::GetGroupSnapshotId() const
{
    return m_groupSnapshot.m_groupSnapshot.m_id;
}
 
OPENSTACK_PLUGIN_NAMESPACE_END
