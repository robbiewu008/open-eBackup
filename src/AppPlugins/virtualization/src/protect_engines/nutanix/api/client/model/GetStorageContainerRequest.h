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
#ifndef NUTANIX_GET_STORAGECONTAINER_RES_H
#define NUTANIX_GET_STORAGECONTAINER_RES_H

#include <string>
#include <common/JsonHelper.h>
#include "common/model/ResponseModel.h"
#include "common/Structs.h"
#include "NutanixRequest.h"

namespace NutanixPlugin {

class GetStorageContainerRequest : public NutanixRequest {
public:
    explicit GetStorageContainerRequest(std::string storagecontainerId):m_storagecontainerId(storagecontainerId)
    {
        url = "PrismGateway/services/rest/v2.0/storage_containers/{storagecontainerId}";
    }
    ~GetStorageContainerRequest() = default;
    
    void FillRequest(RequestInfo &requestInfo)
    {
        requestInfo.m_method = "GET";
        requestInfo.m_pathParams["storagecontainerId"]= m_storagecontainerId;
        requestInfo.m_body = "";
    }
 
    std::string GetStorageContainerId()
    {
        return m_storagecontainerId;
    }
 
private:
    std::string m_storagecontainerId;
};

struct NutanixStorageContainerInfo {
    std::string clusterId;
    std::string containerId;
    std::string name;
    bool markRemoval;
    int32_t maxCapacity;
    ContainerUserStats usageStats;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(clusterId, cluster_uuid)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(containerId, storage_container_uuid)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(name, name)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(markRemoval, mark_for_removal)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(maxCapacity, max_capacity)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(usageStats, usage_stats)
    END_SERIAL_MEMEBER
};

};

#endif