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
#ifndef NUTANIX_GET_VM_LIST_RESPONSE_H
#define NUTANIX_GET_VM_LIST_RESPONSE_H

#include <string>
#include <common/JsonHelper.h>
#include "common/model/ResponseModel.h"
#include "common/Structs.h"
#include "NutanixRequest.h"
#include "protect_engines/nutanix/common/NutanixStructs.h"

namespace NutanixPlugin {
    struct VMListResponse {
        std::string guestOs;
        std::string hostUuid;
        std::string name;
        // "UNKNOWN" "OFF" "POWERING_ON" "ON" "SHUTTING_DOWN" "POWERING_OFF" "PAUSING" "PAUSED" "SUSPENDING"
        // "SUSPENDED" "RESUMING" "RESETTING" "MIGRATING"
        std::string powerState;
        std::string uuid;
        std::string description;
        std::vector<VmNic> vmNics;
        std::vector<VmDiskInfo> vmDiskInfo;

        BEGIN_SERIAL_MEMEBER
        SERIAL_MEMBER_TO_SPECIFIED_NAME(guestOs, guest_os)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(hostUuid, host_uuid)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(name, name)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(powerState, power_state)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(uuid, uuid)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(description, description)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(vmNics, vm_nics)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(vmDiskInfo, vm_disk_info)
        END_SERIAL_MEMEBER
    };

    struct VMListDataResponse {
        std::vector< struct VMListResponse > entities;
        struct ClusterListMetadata metadata;

        BEGIN_SERIAL_MEMEBER
        SERIAL_MEMEBER(entities)
        SERIAL_MEMEBER(metadata)
        END_SERIAL_MEMEBER
    };

using namespace VirtPlugin;

// 在服务端自动过滤管理虚拟机cvm;
class GetVMListRequest : public NutanixRequest {
public:
    GetVMListRequest(int32_t offset, int32_t count, std::string filter = "") : m_offset(offset), m_count(count),
        m_includeVMDiskConfig(true), m_includeVMNicConfig(true) {
        if (!filter.empty()) {
            m_filter = "vm_name==" + filter;
        }
        url = "PrismGateway/services/rest/v2.0/vms";
    }
    ~GetVMListRequest() {}
    void FillRequest(RequestInfo &requestInfo)
    {
        requestInfo.m_method = "GET";
        if (m_filter.length() > 0) {
            requestInfo.m_queryParams["filter"] = m_filter;
        }
        requestInfo.m_queryParams["offset"] = std::to_string(m_offset);
        requestInfo.m_queryParams["count"] = std::to_string(m_count);
        requestInfo.m_queryParams["include_vm_disk_config"] = m_includeVMDiskConfig ? "True" : "False";
        requestInfo.m_queryParams["include_vm_nic_config"] = m_includeVMNicConfig ? "True" : "False";
        requestInfo.m_body = "";
    }
private:
    std::string m_filter;             // Filter criteria - semicolon for AND, comma for OR
    int64_t m_offset;                 // offset - Default 0
    int64_t m_count;                 // Number of VMs to retrieve
    std::string m_sortOrder;         // Sort order
    std::string m_sortAttribute;     // Sort Attribute
    bool m_includeVMDiskConfig;    // Whether to include Virtual Machine disk information.
    bool m_includeVMNicConfig;     // Whether to include network information.
};

struct NewVMBootStruct {
    bool uefiBoot;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(uefiBoot, uefi_boot)
    END_SERIAL_MEMEBER
};

struct CreateNewVMStruct {
    std::string name;
    std::string description;
    std::string timezone;
    int32_t numVcpus;
    int32_t numCoresPerVcpu;
    int32_t memoryMB;
    struct NewVMBootStruct boot;
    std::vector< struct NewVMNicStruct > vmNics;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMEBER(name)
    SERIAL_MEMEBER(description)
    SERIAL_MEMEBER(timezone)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(numVcpus, num_vcpus)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(numCoresPerVcpu, num_cores_per_vcpu)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(memoryMB, memory_mb)
    SERIAL_MEMEBER(boot)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(vmNics, vm_nics)
    END_SERIAL_MEMEBER
};

struct CreateNewVMStructWithAffinity : public CreateNewVMStruct {
    AffinityStruct affinity;

    CreateNewVMStructWithAffinity(CreateNewVMStruct vmPara, AffinityStruct newAffinity)
        : CreateNewVMStruct(vmPara), affinity(newAffinity)
    {}

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMEBER(name)
    SERIAL_MEMEBER(description)
    SERIAL_MEMEBER(timezone)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(numVcpus, num_vcpus)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(numCoresPerVcpu, num_cores_per_vcpu)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(memoryMB, memory_mb)
    SERIAL_MEMEBER(boot)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(vmNics, vm_nics)
    SERIAL_MEMEBER(affinity)
    END_SERIAL_MEMEBER
};

class CreateVMRequest : public NutanixRequest {
public:
    CreateVMRequest()
    {
        url = "PrismGateway/services/rest/v2.0/vms";
    }
    ~CreateVMRequest() = default;
    void FillRequest(RequestInfo &requestInfo)
    {
        requestInfo.m_method = "POST";
        if (!m_affinity.hostUuids.empty()) {
            CreateNewVMStructWithAffinity vmPara(m_vmPara, m_affinity);
            if (!Module::JsonHelper::StructToJsonString(vmPara, requestInfo.m_body)) {
                ERRLOG("Convert affinity paremeter failed.");
            }
        } else {
            if (!Module::JsonHelper::StructToJsonString(m_vmPara, requestInfo.m_body)) {
                ERRLOG("Convert VM paremeter failed.");
            }
        }
    }

    void SetHostAffinity(const std::string &hostUuid)
    {
        m_affinity.hostUuids.emplace_back(hostUuid);
    }

    void FillParameter(const struct NutanixVMInfo &srcVMInfo, std::vector< struct NewVMNicStruct > newVMNics)
    {
        m_vmPara.name = srcVMInfo.name;
        m_vmPara.description = "OceanProtect Created";
        m_vmPara.timezone = srcVMInfo.timezone;
        m_vmPara.numVcpus = srcVMInfo.numVcpus;
        m_vmPara.numCoresPerVcpu = srcVMInfo.numCoresPerVcpu;
        m_vmPara.memoryMB = srcVMInfo.memoryMb;
        m_vmPara.boot.uefiBoot = srcVMInfo.boot.uefiBoot;
        m_vmPara.vmNics = newVMNics;
        m_affinity = srcVMInfo.affinity;
    }

    const struct CreateNewVMStruct &Get()
    {
        return m_vmPara;
    }
private:
    struct CreateNewVMStruct m_vmPara;
    struct AffinityStruct m_affinity;
};

};

#endif