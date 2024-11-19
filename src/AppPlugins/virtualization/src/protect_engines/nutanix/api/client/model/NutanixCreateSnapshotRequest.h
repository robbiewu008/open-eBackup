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
#ifndef NUTANIX_CREATE_NUSNAPSHOT_REQ_H
#define NUTANIX_CREATE_NUSNAPSHOT_REQ_H
 
#include <string>
#include <common/JsonHelper.h>
#include "common/model/ResponseModel.h"
#include "common/Structs.h"
#include "NutanixRequest.h"

namespace NutanixPlugin {

struct SnapshotSpec {
    std::string uuid;
    std::string vmUuid;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(uuid, uuid)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(vmUuid, vm_uuid)
    END_SERIAL_MEMEBER
};

struct CreateSnapshotInfo {
    std::vector<SnapshotSpec> snapshotSpec;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(snapshotSpec, snapshot_specs)
    END_SERIAL_MEMEBER
};

class NutanixCreateSnapshotRequest : public NutanixRequest {
public:
    NutanixCreateSnapshotRequest(std::string snapshotId, std::string vmId)
    {
        SnapshotSpec tmp;
        tmp.uuid = snapshotId;
        tmp.vmUuid = vmId;
        m_snapshotCreate.snapshotSpec.push_back(tmp);
        url = "PrismGateway/services/rest/v2.0/snapshots";
    }
    ~NutanixCreateSnapshotRequest() = default;

    void FillRequest(RequestInfo &requestInfo)
    {
        requestInfo.m_method = "POST";
        if (! Module::JsonHelper::StructToJsonString(m_snapshotCreate, requestInfo.m_body)) {
            ERRLOG("Convert CreateSnapshotInfo failed.");
            return;
        }
    }

private:
    CreateSnapshotInfo m_snapshotCreate;
};
};
 
#endif