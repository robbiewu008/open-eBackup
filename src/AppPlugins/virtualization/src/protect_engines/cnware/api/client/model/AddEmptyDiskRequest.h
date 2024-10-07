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
#ifndef ADD_EMPTY_DISK_REQ_H
#define ADD_EMPTY_DISK_REQ_H
 
#include <string>
#include "CNwareRequest.h"
#include "GetVMDiskInfoResponse.h"

namespace CNwarePlugin {
struct AddDiskReq {
    int32_t bus = 1;                // 必填
    int32_t cache = 4;          // 可选
    int64_t capacity;           // 必填
    int64_t ioHangTimeout = 1800000; // 可选
    std::string poolName;       // 必填
    std::string preallocation = "off";  // 可选
    int32_t type = 1;           // 可选
    std::string volName;        // 必填

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMEBER(bus)
    SERIAL_MEMEBER(cache)
    SERIAL_MEMEBER(capacity)
    SERIAL_MEMEBER(ioHangTimeout)
    SERIAL_MEMEBER(poolName)
    SERIAL_MEMEBER(preallocation)
    SERIAL_MEMEBER(type)
    SERIAL_MEMEBER(volName)
    END_SERIAL_MEMEBER
};

class AddEmptyDiskRequest : public CNwareRequest {
public:
    AddEmptyDiskRequest() {}
    ~AddEmptyDiskRequest() {}
 
    void SetDomainId(const std::string &domainId)
    {
        m_domainId = domainId;
    }
 
    std::string GetDomainId()
    {
        return m_domainId;
    }

    void SetAddDiskReq(AddDiskReq &addReq)
    {
        m_addDiskReq = addReq;
    }

    int32_t GetAddDiskReqString()
    {
        std::string addDiskReqStr;
        if (!Module::JsonHelper::StructToJsonString(m_addDiskReq, m_body)) {
            ERRLOG("Convert AddDiskReq to string failed.");
            return VirtPlugin::FAILED;
        }
        return VirtPlugin::SUCCESS;
    }

private:
    std::string m_domainId;
    AddDiskReq m_addDiskReq;
};
};
 
#endif