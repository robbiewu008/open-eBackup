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
#ifndef HCS_VDC_LIST_DETAIL_H
#define HCS_VDC_LIST_DETAIL_H

#include <vector>
#include <string>
#include <common/JsonHelper.h>
#include "protect_engines/hcs/common/HcsMacros.h"

namespace HcsPlugin {
struct VdcList {
    std::string m_id;
    std::string m_name;
    std::string m_tag;
    std::string m_description;
    std::string m_upperVdcId;
    std::string m_upperVdcName;
    std::string m_topVdcId;
    std::string m_extra;  // object
    double m_ecsUsed;
    double m_evsUsed;
    int32_t m_projectCount;
    bool m_enabled;
    std::string m_domainId;
    int32_t m_level;
    std::string m_createUserId;
    std::string m_createUserName;
    int64_t m_createAt;
    std::string m_utcCreateAt;
    std::string m_domainName;
    std::string m_ldapId;
    std::string m_thirdId;
    std::string m_idpName;
    std::string m_thirdType;
    std::string m_regionId;
    std::string m_enterpriseId;
    std::string m_azId;
    std::string m_enterpriseProjectId;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_id, id)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_name, name)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_tag, tag)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_description, description)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_upperVdcId, upper_vdc_id)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_upperVdcName, upper_vdc_name)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_topVdcId, top_vdc_id)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_extra, extra)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_ecsUsed, ecs_used)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_evsUsed, evs_used)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_projectCount, project_count)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_enabled, enabled)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_domainId, domain_id)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_level, level)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_createUserId, create_user_id)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_createUserName, create_user_name)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_createAt, create_at)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_utcCreateAt, utc_create_at)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_domainName, domain_name)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_ldapId, ldap_id)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_thirdId, third_id)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_idpName, idp_name)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_thirdType, third_type)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_regionId, region_id)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_enterpriseId, enterprise_id)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_azId, az_id)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_enterpriseProjectId, enterprise_project_id)
    END_SERIAL_MEMEBER
};

struct VdcListDetail {
    int32_t m_total;
    std::vector<VdcList> m_vdcs;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_total, total)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_vdcs, vdcs)
    END_SERIAL_MEMEBER
};

struct UserRole {
    std::string m_name;
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_name, name)
    END_SERIAL_MEMEBER
};

struct VDCUserInfo {
    std::string m_name;
    std::string m_id;
    std::string m_vdcId;
    std::vector<UserRole> m_roleList;
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_name, name)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_id, id)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_vdcId, vdc_id)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_roleList, roles)
    END_SERIAL_MEMEBER
};

struct UserDetail {
    VDCUserInfo m_user;
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_user, user)
    END_SERIAL_MEMEBER
};
}

#endif