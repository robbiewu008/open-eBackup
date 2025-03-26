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
#ifndef NUTANIX_GET_CONTAINER_RESPONSE
#define NUTANIX_GET_CONTAINER_RESPONSE

#include <string>
#include <common/JsonHelper.h>
#include "common/model/ResponseModel.h"
#include "common/Structs.h"
#include "NutanixRequest.h"

namespace NutanixPlugin {

class GetContainerRequest : public NutanixRequest {
public:
    explicit GetContainerRequest(int32_t offset, int32_t count) : m_offset(offset), m_count(count)
    {
        url = "PrismGateway/services/rest/v1/containers";
    }
    ~GetContainerRequest() = default;

    void FillRequest(RequestInfo &requestInfo)
    {
        requestInfo.m_method = "GET";
        requestInfo.m_body = "";
        requestInfo.m_queryParams["offset"] = std::to_string(m_offset);
        requestInfo.m_queryParams["count"] = std::to_string(m_count);
    }

private:
    std::string m_filter;             // Filter criteria - semicolon for AND, comma for OR
    int64_t m_offset;                 // offset - Default 0
    int64_t m_count;                 // Number of VMs to retrieve
};

struct ContainerInfo {
        std::string containerUuid;
        std::string name;
        std::string storagePoolId;
        uint64_t maxCapacity;
        ContainerUserStats usageStats;
        bool isNutanixManaged = false;

        BEGIN_SERIAL_MEMEBER
        SERIAL_MEMEBER(containerUuid)
        SERIAL_MEMEBER(name)
        SERIAL_MEMEBER(storagePoolId)
        SERIAL_MEMEBER(maxCapacity)
        SERIAL_MEMEBER(usageStats)
        SERIAL_MEMEBER(isNutanixManaged)
        END_SERIAL_MEMEBER
};

struct ContainerListResponse {
        std::vector<struct ContainerInfo> entities;
        struct ClusterListMetadata metadata;

        BEGIN_SERIAL_MEMEBER
        SERIAL_MEMEBER(entities)
        SERIAL_MEMEBER(metadata)
        END_SERIAL_MEMEBER
};

class GetContainerResponse : public VirtPlugin::ResponseModel {
public:
    GetContainerResponse() {};
    ~GetContainerResponse() = default;

    struct ContainerListResponse GetList()
    {
        return m_containerData;
    }

    virtual bool Serial()
    {
        if (m_body.empty()) {
            return false;
        }
        if (!Module::JsonHelper::JsonStringToStruct(m_body, m_containerData)) {
            ERRLOG("Convert %s failed.", WIPE_SENSITIVE(m_body).c_str());
            return false;
        }
        return true;
    }
private:
    ContainerListResponse m_containerData;
};
};
#endif