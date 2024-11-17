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
#ifndef NUTANIX_GET_HOST_LIST_RESPONSE_H
#define NUTANIX_GET_HOST_LIST_RESPONSE_H

#include <string>
#include <common/JsonHelper.h>
#include "common/model/ResponseModel.h"
#include "common/Structs.h"
#include "NutanixRequest.h"
#include "protect_engines/nutanix/common/NutanixStructs.h"

namespace NutanixPlugin {
    struct HostPosition {
        std::string name;
        int32_t ordinal;
        std::string physicalPosition;

        BEGIN_SERIAL_MEMEBER
        SERIAL_MEMBER_TO_SPECIFIED_NAME(name, name)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(ordinal, ordinal)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(physicalPosition, physical_position)
        END_SERIAL_MEMEBER
    };

    struct HostListResponse {
        std::string name;
        struct HostPosition position;
        int64_t numVms;
        std::string uuid;
        std::string state;
        std::string cpuModel;
        std::string clusterUuid;

        BEGIN_SERIAL_MEMEBER
        SERIAL_MEMBER_TO_SPECIFIED_NAME(name, name)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(position, position)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(numVms, num_vms)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(uuid, uuid)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(state, state)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(cpuModel, cpu_model)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(clusterUuid, cluster_uuid)
        END_SERIAL_MEMEBER
    };

    struct HostListDataResponse {
        std::vector<struct HostListResponse> entities;
        struct ClusterListMetadata metadata;

        BEGIN_SERIAL_MEMEBER
        SERIAL_MEMBER_TO_SPECIFIED_NAME(entities, entities)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(metadata, metadata)
        END_SERIAL_MEMEBER
    };

using namespace VirtPlugin;

class GetHostListRequest : public NutanixRequest {
public:
    GetHostListRequest(int32_t count, int32_t page) : m_count(count), m_page(page)
    {
        url = "PrismGateway/services/rest/v2.0/hosts";
    }
    ~GetHostListRequest() = default;
    void FillRequest(RequestInfo &requestInfo)
    {
        requestInfo.m_method = "GET";
        requestInfo.m_queryParams["length"] = std::to_string(m_count);
        requestInfo.m_queryParams["offset"] = std::to_string(m_page);
        requestInfo.m_body = "";
    }
private:
    int32_t m_count;                      // Number of Clusters to retrieve
    std::string m_filterCriteria;        // Filter criteria
    std::string m_sortCriteria;          // Sort criteria
    std::string m_searchString;          // Search string
    std::vector<std::string> m_searchAttributeList;               // Search attribute list
    int32_t m_page;                       // Page number
    std::string m_projection;             // Projections on the attributes
};

};

#endif