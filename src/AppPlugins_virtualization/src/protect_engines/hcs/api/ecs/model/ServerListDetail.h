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
#ifndef HCS_SERVER_LIST_DETAIL_H
#define HCS_SERVER_LIST_DETAIL_H

#include <string>
#include <vector>
#include <json/json.h>
#include <common/JsonHelper.h>
#include "protect_engines/hcs/common/HcsMacros.h"
#include "protect_engines/openstack/api/nova/model/ServerDetail.h"

namespace HcsPlugin {
struct ServerList {
    std::string m_uuid;     // 云服务器ID
    std::string m_name;     // 云服务器名称

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_uuid, id)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_name, name)
    END_SERIAL_MEMEBER
};

struct ServerListDetail {
    int32_t m_count;     // 云服务器列表总数
    std::vector<ServerList> m_servers;     // 云服务器列表

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_count, count)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_servers, servers)
    END_SERIAL_MEMEBER
};

struct ProjectServerDetailList {
    std::vector<OpenStackPlugin::OpenStackServerInfo> m_serversInfo;     // 云服务器信息

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_serversInfo, servers)
    END_SERIAL_MEMEBER
};
}

#endif