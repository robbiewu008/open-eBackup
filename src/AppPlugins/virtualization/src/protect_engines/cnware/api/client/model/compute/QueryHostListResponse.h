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
#ifndef __QUERY_HOSTLIST_RESPONSE__
#define __QUERY_HOSTLIST_RESPONSE__

#include <string>
#include <common/JsonHelper.h>
#include "common/model/ResponseModel.h"

namespace CNwarePlugin {
struct HostRsp {
    std::string clusterId;
    std::string clusterName;
    std::string id;
    std::string ip;
    std::string cpuArchitecture; /* 主机CPU架构: x86_64 aarch64 mips64el sw_64 loongarch64 */
    std::string hostname;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMEBER(clusterId)
    SERIAL_MEMEBER(clusterName)
    SERIAL_MEMEBER(id)
    SERIAL_MEMEBER(ip)
    SERIAL_MEMEBER(cpuArchitecture)
    SERIAL_MEMEBER(hostname)
    END_SERIAL_MEMEBER
};

struct HostDataResponse {
    std::vector<HostRsp> data;
    std::string order;
    int32_t size;
    std::string sort;
    int32_t start;
    int64_t total;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMEBER(data)
    SERIAL_MEMEBER(order)
    SERIAL_MEMEBER(size)
    SERIAL_MEMEBER(sort)
    SERIAL_MEMEBER(start)
    SERIAL_MEMEBER(total)
    END_SERIAL_MEMEBER
};

class QueryHostListResponse : public VirtPlugin::ResponseModel {
public:
    QueryHostListResponse() {};
    ~QueryHostListResponse() {};

    HostDataResponse GetHostData()
    {
        return m_hostData;
    }
    virtual bool Serial()
    {
        if (m_body.empty()) {
            return false;
        }
        if (!Module::JsonHelper::JsonStringToStruct(m_body, m_hostData)) {
            ERRLOG("Convert %s failed.", WIPE_SENSITIVE(m_body).c_str());
            return false;
        }
        return true;
    }
private:
    HostDataResponse m_hostData;
};
};
#endif