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
#ifndef GET_VM_LIST_RESPONSE_H
#define GET_VM_LIST_RESPONSE_H

#include <string>
#include <common/JsonHelper.h>
#include "common/model/ResponseModel.h"

namespace CNwarePlugin {
struct DomainDiskDbRsp {
    int32_t bootOrder;
    std::string bus;
    std::string dev;
    std::string fileName;
    std::string id;
    std::string volId;
    std::string type;
    bool shareable;
    int64_t virtualSize;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMEBER(bootOrder)
    SERIAL_MEMEBER(bus)
    SERIAL_MEMEBER(dev)
    SERIAL_MEMEBER(fileName)
    SERIAL_MEMEBER(id)
    SERIAL_MEMEBER(volId)
    SERIAL_MEMEBER(type)
    SERIAL_MEMEBER(shareable)
    SERIAL_MEMEBER(virtualSize)
    END_SERIAL_MEMEBER
};

struct DomainListResponse {
    std::string hostId;
    std::string hostName;
    std::string id;
    std::string uuid;
    std::vector<DomainDiskDbRsp> domainDiskDbRspList;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMEBER(hostId)
    SERIAL_MEMEBER(hostName)
    SERIAL_MEMEBER(id)
    SERIAL_MEMEBER(uuid)
    SERIAL_MEMEBER(domainDiskDbRspList)
    END_SERIAL_MEMEBER
};

struct DataResponse {
    std::vector<DomainListResponse> data;
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

class GetVMListResponse : public VirtPlugin::ResponseModel {
public:
    GetVMListResponse() {}
    ~GetVMListResponse() {}

    bool Serial()
    {
        if (m_body.empty()) {
            return false;
        }
        if (!Module::JsonHelper::JsonStringToStruct(m_body, m_vmList)) {
            ERRLOG("Convert %s failed.", WIPE_SENSITIVE(m_body).c_str());
            return false;
        }
        return true;
    }

    DataResponse GetVMList()
    {
        return m_vmList;
    }

private:
    DataResponse m_vmList;
};
};

#endif