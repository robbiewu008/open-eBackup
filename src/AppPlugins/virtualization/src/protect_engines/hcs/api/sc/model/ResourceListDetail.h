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
#ifndef HCS_RESOURCE_LIST_DETAIL_H
#define HCS_RESOURCE_LIST_DETAIL_H

#include <vector>
#include <string>
#include <common/JsonHelper.h>
#include "protect_engines/hcs/common/HcsMacros.h"

namespace HcsPlugin {
struct RegionName {
    std::string m_zhCn;
    std::string m_enUs;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_zhCn, zh_cn)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_enUs, en_us)
    END_SERIAL_MEMEBER
};

struct RegionParams {
    std::string m_regionId;
    RegionName m_regionName;
    std::string m_regionType;
    std::string m_regionStatus;
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_regionId, region_id)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_regionName, region_name)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_regionType, region_type)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_regionStatus, region_status)
    END_SERIAL_MEMEBER
};

struct Project {
    std::string m_id;
    std::string m_name;
    std::string m_description;
    std::string m_domainId;
    bool m_enabled;
    std::string m_tenantId;
    bool m_isShare;  // todo
    std::string m_tenantName;
    std::string m_createUserId;
    std::string m_createUserName;
    std::vector<RegionParams> m_regions;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_id, id)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_name, name)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_description, description)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_domainId, domain_id)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_enabled, enabled)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_tenantId, tenant_id)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_isShare, is_share)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_tenantName, tenant_name)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_createUserId, create_user_id)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_createUserName, create_user_name)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_regions, regions)
    END_SERIAL_MEMEBER
};

struct ResourceListDetail {
    int32_t m_total;
    std::vector<Project> m_projects;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_total, total)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_projects, projects)
    END_SERIAL_MEMEBER
};
}

#endif