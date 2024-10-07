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
#ifndef MIGRATE_HOSTS_REQUEST_H
#define MIGRATE_HOSTS_REQUEST_H

#include <string>
#include <common/JsonHelper.h>
#include "CNwareRequest.h"
#include "common/Constants.h"

namespace CNwarePlugin {
struct BodyOfMigrateHostsReq {
    std::string mClusterId;
    std::string mHostId;
    std::string mPoolId;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mClusterId, clusterId)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mHostId, hostId)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mPoolId, poolId)
    END_SERIAL_MEMEBER
};
class MigrateHostsRequest : public CNwareRequest {
public:
    MigrateHostsRequest() {}
    ~MigrateHostsRequest() {}

    void SetTaskId(const std::string clusterId, const std::string hostId, const std::string poolId)
    {
        m_migrateHostsReq.mClusterId = clusterId;
        m_migrateHostsReq.mHostId = hostId;
        m_migrateHostsReq.mPoolId = poolId;
    }

    int32_t MigrateHostsReqToJson()
    {
        if (!Module::JsonHelper::StructToJsonString(m_migrateHostsReq, m_body)) {
            ERRLOG("Convert snapinfo to json string failed!");
            return VirtPlugin::FAILED;
        }
        return VirtPlugin::SUCCESS;
    }

private:
    BodyOfMigrateHostsReq m_migrateHostsReq;
};
};  // End namespase CNwarePlugin
#endif  // GET_TASK_DETAIL_REQUEST_H