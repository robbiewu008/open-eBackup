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
#ifndef NUTANIX_DEL_NUSNAPSHOT_REQ_H
#define NUTANIX_DEL_NUSNAPSHOT_REQ_H
 
#include <string>
#include <common/JsonHelper.h>
#include "common/model/ResponseModel.h"
#include "common/Structs.h"
#include "NutanixRequest.h"

namespace NutanixPlugin {

class DelSnapshotRequest : public NutanixRequest {
public:
    explicit DelSnapshotRequest(std::string snapshotId):m_snapshotId(snapshotId)
    {
        url = "PrismGateway/services/rest/v2.0/snapshots/{snapshotId}";
    }
    ~DelSnapshotRequest() = default;

    std::string GetSnapshotId()
    {
        return m_snapshotId;
    }
    void FillRequest(RequestInfo &requestInfo)
    {
        requestInfo.m_method = "DELETE";
        requestInfo.m_pathParams["snapshotId"]= m_snapshotId;
        requestInfo.m_body = "";
    }

private:
    std::string m_snapshotId;
};
};
#endif