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
#ifndef NUTANIX_QUERY_TASK_RESPONSE_H
#define NUTANIX_QUERY_TASK_RESPONSE_H

#include <string>
#include <vector>
#include <common/JsonHelper.h>
#include "common/model/ResponseModel.h"
#include "common/Structs.h"
#include "NutanixRequest.h"

namespace NutanixPlugin {

const std::string NUTANIX_TASK_STATUS_NONE = "None";
const std::string NUTANIX_TASK_STATUS_FAILED = "Failed";
const std::string NUTANIX_TASK_STATUS_SUSPENDED = "Suspended";
const std::string NUTANIX_TASK_STATUS_ABORTED = "Aborted";
const std::string NUTANIX_TASK_STATUS_SUCCEEDED = "Succeeded";
const std::string NUTANIX_TASK_STATUS_RUNNING = "Running";
const std::string NUTANIX_TASK_STATUS_QUEUED = "Queued";

class QueryTaskRequest : public NutanixRequest {
public:
    explicit QueryTaskRequest(std::string taskId):m_taskId(taskId)
    {
        url = "PrismGateway/services/rest/v2.0/tasks/{taskId}";
    }
    ~QueryTaskRequest() = default;

    void FillRequest(RequestInfo &requestInfo)
    {
        requestInfo.m_method = "GET";
        requestInfo.m_pathParams["taskId"]= m_taskId;
        requestInfo.m_body = "";
    }
 
    std::string GetTaskId()
    {
        return m_taskId;
    }
 
private:
    std::string m_taskId;
};

struct MetaResponse {
    int32_t errorCode;
    std::string errorDetail;

    BEGIN_SERIAL_MEMEBER;
    SERIAL_MEMBER_TO_SPECIFIED_NAME(errorCode, error_code)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(errorDetail, error_detail)
    END_SERIAL_MEMEBER;
};

struct MetaRequest {
    std::string methodName;

    BEGIN_SERIAL_MEMEBER;
    SERIAL_MEMBER_TO_SPECIFIED_NAME(methodName, method_name)
    END_SERIAL_MEMEBER;
};

struct Entity {
    std::string entityId;
    std::string entityName;
    std::string entityType;

    BEGIN_SERIAL_MEMEBER;
    SERIAL_MEMBER_TO_SPECIFIED_NAME(entityId, entity_id)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(entityName, entity_name)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(entityType, entity_type)
    END_SERIAL_MEMEBER;
};

struct NutanixTaskResponse {
    std::string clusterId;
    MetaRequest metaRequest;
    MetaResponse errMsg;
    std::string operationType;
    std::string status;
    std::string id;
    std::vector<Entity> entites;

    BEGIN_SERIAL_MEMEBER;
    SERIAL_MEMBER_TO_SPECIFIED_NAME(clusterId, cluster_uuid)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(metaRequest, meta_request)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(errMsg, meta_response)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(operationType, operation_type)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(status, progress_status)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(id, uuid)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(entites, entity_list)
    END_SERIAL_MEMEBER;
};

};

#endif