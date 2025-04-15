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
#ifndef NUTANIX_GET_CLUSTER_LIST_RESPONSE_H
#define NUTANIX_GET_CLUSTER_LIST_RESPONSE_H

#include <string>
#include <vector>
#include <common/JsonHelper.h>
#include "common/model/ResponseModel.h"
#include "NutanixRequest.h"
#include "protect_engines/nutanix/common/NutanixStructs.h"

namespace NutanixPlugin {

struct ClusterListResponse {
    std::string uuid;
    std::string version;
    std::string name;
    std::string arch;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMEBER(uuid)
    SERIAL_MEMEBER(version)
    SERIAL_MEMEBER(name)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(arch, cluster_arch)
    END_SERIAL_MEMEBER
};

struct ClusterListDataResponse {
    std::vector< struct ClusterListResponse > entities;
    struct ClusterListMetadata metadata;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMEBER(entities)
    SERIAL_MEMEBER(metadata)
    END_SERIAL_MEMEBER
};

using namespace VirtPlugin;

class GetClusterListRequest : public NutanixRequest {
public:
    GetClusterListRequest(int32_t count, int32_t page) : m_count(count), m_page(page)
    {
        url = "PrismGateway/services/rest/v2.0/clusters";
    }
    ~GetClusterListRequest() = default;
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
    int32_t m_page;                       // Page number, 从1开始,指第一页;
    std::string m_projection;             // Projections on the attributes
};

class GetClusterRequest : public NutanixRequest {
public:
    explicit GetClusterRequest(std::string clusterUUID) : m_clusterUUID(clusterUUID)
    {
        url = "PrismGateway/services/rest/v2.0/clusters/" + clusterUUID;
        if (clusterUUID.empty()) {
            ERRLOG("cluster uuid is emtpy.");
        }
    }
    ~GetClusterRequest() = default;
    void FillRequest(RequestInfo &requestInfo)
    {
        requestInfo.m_method = "GET";
        requestInfo.m_queryParams["id"] = m_clusterUUID;
    }

private:
    std::string m_clusterUUID;
};

};

#endif