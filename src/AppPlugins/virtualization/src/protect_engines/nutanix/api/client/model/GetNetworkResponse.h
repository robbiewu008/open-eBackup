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
#ifndef NUTANIX_GET_NETWORK_RESPONSE
#define NUTANIX_GET_NETWORK_RESPONSE

#include <string>
#include <common/JsonHelper.h>
#include "common/model/ResponseModel.h"
#include "common/Structs.h"
#include "NutanixRequest.h"

namespace NutanixPlugin {

class GetNetworkRequest : public NutanixRequest {
public:
    explicit GetNetworkRequest(int32_t offset, int32_t length) : m_offset(offset), m_length(length)
    {
        url = "PrismGateway/services/rest/v2.0/networks";
    }
    ~GetNetworkRequest() = default;

    void FillRequest(RequestInfo &requestInfo)
    {
        requestInfo.m_method = "GET";
        requestInfo.m_body = "";
        requestInfo.m_queryParams["offset"] = std::to_string(m_offset);
        requestInfo.m_queryParams["length"] = std::to_string(m_length);
    }

private:
    std::string m_filter;             // Filter criteria - semicolon for AND, comma for OR
    int64_t m_offset;                 // offset - Default 0
    int64_t m_length;                 // Number of VMs to retrieve
};

class GetNetworkByIdRequest : public NutanixRequest {
public:
    explicit GetNetworkByIdRequest(std::string id) : m_id(id)
    {
        url = "PrismGateway/services/rest/v2.0/networks/{uuid}";
    }
    ~GetNetworkByIdRequest() {}

    void FillRequest(RequestInfo &requestInfo)
    {
        requestInfo.m_method = "GET";
        requestInfo.m_pathParams["uuid"]= m_id;
        requestInfo.m_body = "";
    }

private:
    std::string m_id;
};

struct NetworkMetadata {
    int32_t totalEntities;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(totalEntities, grand_total_entities)
    END_SERIAL_MEMEBER
};

struct NetworkInfo {
    std::string uuid;
    std::string name;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMEBER(uuid)
    SERIAL_MEMEBER(name)
    END_SERIAL_MEMEBER
};

struct NetworkListResponse {
    std::vector<struct NetworkInfo> entities;
    struct NetworkMetadata metadata;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMEBER(entities)
    SERIAL_MEMEBER(metadata)
    END_SERIAL_MEMEBER
};

class GetNetworkResponse : public VirtPlugin::ResponseModel {
public:
    GetNetworkResponse() {};
    ~GetNetworkResponse() = default;

    struct NetworkListResponse& GetList()
    {
        return m_networkData;
    }

    virtual bool Serial()
    {
        if (m_body.empty()) {
            return false;
        }
        if (!Module::JsonHelper::JsonStringToStruct(m_body, m_networkData)) {
            ERRLOG("Convert %s failed.", WIPE_SENSITIVE(m_body).c_str());
            return false;
        }
        return true;
    }
private:
    NetworkListResponse m_networkData;
};
};
#endif