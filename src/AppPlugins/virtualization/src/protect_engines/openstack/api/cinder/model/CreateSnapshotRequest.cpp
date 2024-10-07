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
#include "CreateSnapshotRequest.h"

OPENSTACK_PLUGIN_NAMESPACE_BEGIN

CreateSnapshotRequest::CreateSnapshotRequest() {}

CreateSnapshotRequest::~CreateSnapshotRequest() {}

Scope CreateSnapshotRequest::GetScopeType() const
{
    return Scope::PROJECT;
}

ApiType CreateSnapshotRequest::GetApiType()
{
    return ApiType::CINDER;
}

CreateSnapshotRequestBodyMsg CreateSnapshotRequest::GetBody() const
{
    return m_body;
}

bool CreateSnapshotRequest::BodyIsSet() const
{
    return m_bodyIsSet;
}

void CreateSnapshotRequest::UnsetBody()
{
    m_bodyIsSet = false;
}

void CreateSnapshotRequest::SetBody(const CreateSnapshotRequestBodyMsg& body)
{
    m_body = body;
    m_bodyIsSet = true;
}
OPENSTACK_PLUGIN_NAMESPACE_END
