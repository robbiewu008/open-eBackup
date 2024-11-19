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
#include "GetDomainsResponse.h"

OPENSTACK_PLUGIN_NAMESPACE_BEGIN
GetDomainsResponse::GetDomainsResponse() {}
GetDomainsResponse::~GetDomainsResponse() {}

bool GetDomainsResponse::Serial()
{
    if (m_body.empty()) {
        ERRLOG("Get domain response is empty.");
        return false;
    }
    if (!Module::JsonHelper::JsonStringToStruct(m_body, m_domains)) {
        ERRLOG("Convert response body to domain object failed.");
        return false;
    }
    return true;
}

DomainInfo GetDomainsResponse::GetDomainList()
{
    return m_domains;
}
OPENSTACK_PLUGIN_NAMESPACE_END