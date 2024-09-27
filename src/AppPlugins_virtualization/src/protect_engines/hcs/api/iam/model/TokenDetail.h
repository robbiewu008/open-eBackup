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
#ifndef HCS_GET_TOKEN_DETAIL_H
#define HCS_GET_TOKEN_DETAIL_H

#include <vector>
#include <common/JsonHelper.h>
#include "protect_engines/hcs/common/HcsMacros.h"

namespace HcsPlugin {
struct ProjectInfo {
    std::string m_id;
    std::string m_name;
    std::string m_domianId;
    bool m_enabled;
    bool m_isDomain;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_id, id)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_name, name)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_domianId, domain_id)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_enabled, enabled)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_isDomain, is_domain)
    END_SERIAL_MEMEBER
};

struct ProjectResponse {
    std::vector<ProjectInfo> m_projects;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_projects, projects)
    END_SERIAL_MEMEBER
};
}

#endif