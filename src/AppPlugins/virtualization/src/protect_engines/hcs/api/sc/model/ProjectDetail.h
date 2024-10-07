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
#ifndef HCS_PROJECT_DETAIL_H
#define HCS_PROJECT_DETAIL_H

#include <string>
#include <vector>
#include <common/JsonHelper.h>
#include "protect_engines/hcs/common/HcsMacros.h"

namespace HcsPlugin {
struct RegionLanguageName {
    std::string m_zhCn;
    std::string m_enUs;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_zhCn, zh_cn)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_enUs, en_us)
    END_SERIAL_MEMEBER
};

struct CloudInfras {
    std::string m_cloudInfraId;
    std::string m_cloudInfraName;
    std::string m_cloudInfraStatus;
    std::string m_cloudInfraType;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_cloudInfraId, cloud_infra_id)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_cloudInfraName, cloud_infra_name)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_cloudInfraStatus, cloud_infra_status)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_cloudInfraType, cloud_infra_type)
    END_SERIAL_MEMEBER
};

struct RegionDetail {
    std::string m_regionId;
    RegionLanguageName m_regionName;
    std::string m_regionType;
    std::string m_regionStatus;
    std::vector<CloudInfras> m_cloudInfras;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_regionId, region_id)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_regionName, region_name)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_regionType, region_type)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_regionStatus, region_status)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_cloudInfras, cloud_infras)
    END_SERIAL_MEMEBER
};

struct ProjectDetailInfo {
    std::string m_tenantId;
    std::string m_createUserId;
    std::string m_tenantName;
    std::string m_createUserName;
    std::string m_description;
    std::string m_tenantType;
    bool m_enabled;
    std::string m_domainId;
    std::string m_domainName;
    std::string m_contractNumber;
    bool m_isShared;
    std::string m_name;
    std::string m_iamProjectName;
    std::string m_displayName;
    std::string m_id;
    std::string m_ownerId;
    std::string m_ownerName;
    std::string m_regionName;
    std::vector<RegionDetail> m_regions;
    std::string m_attachmentId;
    std::string m_attachmentName;
    int32_t m_attachmentSize;
    bool m_isSupportHwsService;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_tenantId, tenant_id)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_createUserId, create_user_id)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_tenantName, tenant_name)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_createUserName, create_user_name)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_description, description)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_tenantType, tenant_type)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_enabled, enabled)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_domainId, domain_id)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_domainName, domain_name)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_contractNumber, contract_number)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_isShared, is_shared)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_name, name)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_iamProjectName, iam_project_name)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_displayName, display_name)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_id, id)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_ownerId, owner_id)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_ownerName, owner_name)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_regionName, region_name)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_regions, regions)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_attachmentId, attachment_id)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_attachmentName, attachment_name)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_attachmentSize, attachment_size)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_isSupportHwsService, is_support_hws_service)
    END_SERIAL_MEMEBER
};

struct ProjectDetail {
    ProjectDetailInfo m_projectDetailInfo;
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_projectDetailInfo, project)
    END_SERIAL_MEMEBER
};
}

#endif