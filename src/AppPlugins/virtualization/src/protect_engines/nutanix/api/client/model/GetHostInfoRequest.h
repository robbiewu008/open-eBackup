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
#ifndef NUTANIX_GET_HOSTINFO_RESPONSE
#define NUTANIX_GET_HOSTINFO_RESPONSE

#include <string>
#include <common/JsonHelper.h>
#include "common/Structs.h"
#include "NutanixRequest.h"

namespace NutanixPlugin {

class GetHostInfoRequest : public NutanixRequest {
public:
    explicit GetHostInfoRequest(std::string hostId) : m_hostId(hostId)
    {
        url = "PrismGateway/services/rest/v2.0/hosts/{hostId}";
    }
    ~GetHostInfoRequest() = default;

    void FillRequest(RequestInfo &requestInfo)
    {
        requestInfo.m_method = "GET";
        requestInfo.m_pathParams["hostId"]= m_hostId;
        requestInfo.m_body = "";
    }

private:
    std::string m_hostId;
};

struct HostInfo {
    std::string clusterId;
    std::string id;
    std::string name;
    int32_t numVms;
    std::string hostType;
    std::string ip;
    bool isMaintain {false};
    bool isDegraded {false};

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(clusterId, cluster_uuid)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(id, uuid)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(name, name)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(numVms, num_vms)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(hostType, host_type)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(ip, ipmiaddress)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(isMaintain, host_in_maintenance_mode)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(isDegraded, is_degraded)
    END_SERIAL_MEMEBER
};
};
#endif