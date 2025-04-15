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
#ifndef NUTANIX_DETACH_DISK_REQ_H
#define NUTANIX_DETACH_DISK_REQ_H
 
#include <string>
#include <vector>
#include <common/JsonHelper.h>
#include "common/model/ResponseModel.h"
#include "common/Structs.h"
#include "NutanixRequest.h"
#include "protect_engines/nutanix/common/NutanixStructs.h"

namespace NutanixPlugin {

struct DetachDiskAddress {
    std::string detachDiskId;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(detachDiskId, vmdisk_uuid)
    END_SERIAL_MEMEBER
};

struct DetachDiskInfo {
    DetachDiskAddress detachDisk;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(detachDisk, disk_address)
    END_SERIAL_MEMEBER
};
struct DetachDisksInfo {
    std::vector<DetachDiskInfo> vmDisks;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(vmDisks, vm_disks)
    END_SERIAL_MEMEBER
};

class DetachDiskRequest : public NutanixRequest {
public:
    explicit DetachDiskRequest(std::string vmId, std::string detachId): m_vmId(vmId)
    {
        DetachDiskInfo detachDiskInfo;
        detachDiskInfo.detachDisk.detachDiskId = detachId;
        m_detachDisk.vmDisks.push_back(detachDiskInfo);
        url = "PrismGateway/services/rest/v2.0/vms/{vmId}/disks/detach";
    }

    DetachDiskRequest(std::string vmId, const std::vector<std::string> &detachIds)
    {
        m_vmId = vmId;
        for (const auto &detachId : detachIds) {
            DetachDiskInfo detachDiskInfo;
            detachDiskInfo.detachDisk.detachDiskId = detachId;
            m_detachDisk.vmDisks.push_back(detachDiskInfo);
        }
        url = "PrismGateway/services/rest/v2.0/vms/{vmId}/disks/detach";
    }
    ~DetachDiskRequest() = default;

    std::string GetVmId()
    {
        return m_vmId;
    }
    void FillRequest(RequestInfo &requestInfo)
    {
        requestInfo.m_method = "POST";
        requestInfo.m_pathParams["vmId"]= m_vmId;
        if (! Module::JsonHelper::StructToJsonString(m_detachDisk, requestInfo.m_body)) {
            ERRLOG("Convert AttachDiskInfo failed.");
            return;
        }
    }

private:
    DetachDisksInfo m_detachDisk;
    std::string m_vmId;
};
};
 
#endif