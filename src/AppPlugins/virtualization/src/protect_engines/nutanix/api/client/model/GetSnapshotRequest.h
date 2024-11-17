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
#ifndef NUTANIX_GET_SNAPSHOT_RES_H
#define NUTANIX_GET_SNAPSHOT_RES_H

#include <string>
#include <common/JsonHelper.h>
#include "common/Structs.h"
#include "NutanixRequest.h"
#include "protect_engines/nutanix/common/NutanixStructs.h"

namespace NutanixPlugin {

class GetSnapshotRequest : public NutanixRequest {
public:
    explicit GetSnapshotRequest(std::string snapshotId):m_snapshotId(snapshotId)
    {
        url = "PrismGateway/services/rest/v2.0/snapshots/{snapshotId}";
    }
    ~GetSnapshotRequest() = default;
    
    void FillRequest(RequestInfo &requestInfo)
    {
        requestInfo.m_method = "GET";
        requestInfo.m_pathParams["snapshotId"]= m_snapshotId;
        requestInfo.m_body = "";
    }
 
private:
    std::string m_snapshotId;
};

struct NutanixSnapshotInfo {
    bool isDeleted;
    std::string groupId;
    std::string logicalTimestamp;
    std::string name;
    std::string id;
    std::string vmId;
    NutanixVMInfo oringeVm;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(isDeleted, deleted)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(groupId, group_uuid)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(logicalTimestamp, logical_timestamp)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(name, snapshot_name)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(id, uuid)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(vmId, vm_uuid)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(oringeVm, vm_create_spec)
    END_SERIAL_MEMEBER
};
};
#endif