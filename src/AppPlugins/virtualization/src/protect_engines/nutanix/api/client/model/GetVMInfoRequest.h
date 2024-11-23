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
#ifndef NUTANIX_GET_VM_INFO_RESPONSE_H
#define NUTANIX_GET_VM_INFO_RESPONSE_H

#include <string>
#include <vector>
#include <common/JsonHelper.h>
#include "common/model/ResponseModel.h"
#include "common/Structs.h"
#include "NutanixRequest.h"
#include "protect_engines/nutanix/common/NutanixStructs.h"

namespace NutanixPlugin {
class GetVMInfoRequest : public NutanixRequest {
public:
    explicit GetVMInfoRequest(std::string vmId) : m_vmId(vmId), m_includeVmDiskConfig(true), m_includeVmNicConfig(true)
    {
        url = "PrismGateway/services/rest/v2.0/vms/{vmId}";
    }
    ~GetVMInfoRequest() = default;

    void FillRequest(RequestInfo &requestInfo)
    {
        requestInfo.m_method = "GET";
        requestInfo.m_pathParams["vmId"] = m_vmId;
        requestInfo.m_queryParams["include_vm_disk_config"] = m_includeVmDiskConfig ? "True" : "False";
        requestInfo.m_queryParams["include_vm_nic_config"] = m_includeVmNicConfig ? "True" : "False";
        requestInfo.m_body = "";
    }

    void SetVmDiskConfig(bool includeVmDiskConfig)
    {
        m_includeVmDiskConfig = includeVmDiskConfig;
    }

    void SetVmNicConfig(bool includeVmNicConfig)
    {
        m_includeVmNicConfig = includeVmNicConfig;
    }

    std::string GetVmId()
    {
        return m_vmId;
    }

private:
    std::string m_vmId;
    bool m_includeVmDiskConfig;    // Whether to include Virtual Machine disk information.
    bool m_includeVmNicConfig;     // Whether to include network information.
};
};

#endif