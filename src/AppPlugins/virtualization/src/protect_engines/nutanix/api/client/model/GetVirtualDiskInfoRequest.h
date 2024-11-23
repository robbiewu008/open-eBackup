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
#ifndef NUTANIX_GET_VMDISK_RESPONSE
#define NUTANIX_GET_VMDISK_RESPONSE

#include <string>
#include <common/JsonHelper.h>
#include "common/Structs.h"
#include "NutanixRequest.h"

namespace NutanixPlugin {

class GetVirtualDiskInfoRequest : public NutanixRequest {
public:
    explicit GetVirtualDiskInfoRequest(std::string vmdiskId) : m_vmdiskId(vmdiskId)
    {
        url = "PrismGateway/services/rest/v2.0/virtual_disks/{vmdiskId}";
    }
    ~GetVirtualDiskInfoRequest() = default;

    void FillRequest(RequestInfo &requestInfo)
    {
        requestInfo.m_method = "GET";
        requestInfo.m_pathParams["vmdiskId"]= m_vmdiskId;
        requestInfo.m_body = "";
    }

private:
    std::string m_vmdiskId;
};

struct VirtualDiskInfo {
    std::string attachedVmId;
    std::string attachedVmName;
    std::string attachedVolGroupId;
    std::string clusterId;
    std::string storageContainerId;
    std::string id;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(attachedVmId, attached_vm_id)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(attachedVmName, attached_vm_name)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(attachedVolGroupId, attached_volume_group_id)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(clusterId, cluster_uuid)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(storageContainerId, storage_container_uuid)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(id, uuid)
    END_SERIAL_MEMEBER
};
};
#endif