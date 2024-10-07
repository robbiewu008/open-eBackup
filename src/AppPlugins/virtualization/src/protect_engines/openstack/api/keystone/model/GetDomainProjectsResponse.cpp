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
#include "GetDomainProjectsResponse.h"

OPENSTACK_PLUGIN_NAMESPACE_BEGIN

GetDomainProjectsResponse::GetDomainProjectsResponse() {}

GetDomainProjectsResponse::~GetDomainProjectsResponse() {}

bool GetDomainProjectsResponse::Serial()
{
    if (m_body.empty()) {
        ERRLOG("Get domain project response is empty.");
        return false;
    }
    if (!Module::JsonHelper::JsonStringToStruct(m_body, m_projects)) {
        ERRLOG("Convert response body to projects failed.");
        return false;
    }
    return true;
}

ProjectList GetDomainProjectsResponse::GetProjectList()
{
    return m_projects;
}
OPENSTACK_PLUGIN_NAMESPACE_END
