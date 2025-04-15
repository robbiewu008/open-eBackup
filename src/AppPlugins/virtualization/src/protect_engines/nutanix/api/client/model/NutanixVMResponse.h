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
#ifndef NUTANIX_VM_LIST_RESPONSE_H
#define NUTANIX_VM_LIST_RESPONSE_H

#include <string>
#include <common/JsonHelper.h>
#include "common/model/ResponseModel.h"
#include "common/Structs.h"
#include "NutanixRequest.h"
#include "protect_engines/nutanix/common/NutanixStructs.h"

namespace NutanixPlugin {
    struct SetVMPowerStateBody {
        // ['ON', 'OFF', 'POWERCYCLE', 'RESET', 'PAUSE', 'SUSPEND', 'RESUME', 'SAVE', 'ACPI_SHUTDOWN', 'ACPI_REBOOT']
        std::string transition;
        std::string uuid;

        BEGIN_SERIAL_MEMEBER
        SERIAL_MEMEBER(transition)
        SERIAL_MEMEBER(uuid)
        END_SERIAL_MEMEBER
    };

using namespace VirtPlugin;

class PowerOffVmRequest : public NutanixRequest {
public:
    explicit PowerOffVmRequest(const std::string &vmUuid) : m_vmUuid(vmUuid)
    {
        if (vmUuid.empty()) {
            ERRLOG("VM uuid is empty.");
        }
        url = "PrismGateway/services/rest/v2.0/vms/" + vmUuid + "/set_power_state";
        m_powerState.transition = "OFF";
        m_powerState.uuid = vmUuid;
    }

    ~PowerOffVmRequest() = default;
    void FillRequest(RequestInfo &requestInfo)
    {
        requestInfo.m_method = "POST";
        Module::JsonHelper::StructToJsonString(m_powerState, requestInfo.m_body);
    }
private:
    std::string m_vmUuid;
    SetVMPowerStateBody m_powerState;
};

class PowerOnVmRequest : public NutanixRequest {
public:
    PowerOnVmRequest(const std::string &vmUuid) : m_vmUuid(vmUuid)
    {
        if (vmUuid.empty()) {
            ERRLOG("VM uuid is empty.");
        }
        url = "PrismGateway/services/rest/v2.0/vms/" + vmUuid + "/set_power_state";
        m_powerState.transition = "ON";
        m_powerState.uuid = vmUuid;
    }

    ~PowerOnVmRequest() = default;
    void FillRequest(RequestInfo &requestInfo)
    {
        requestInfo.m_method = "POST";
        Module::JsonHelper::StructToJsonString(m_powerState, requestInfo.m_body);
    }
private:
    std::string m_vmUuid;
    SetVMPowerStateBody m_powerState;
};

};

#endif