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
#ifndef GET_VM_INFO_RESPONSE_H
#define GET_VM_INFO_RESPONSE_H

#include <string>
#include <common/JsonHelper.h>
#include "common/model/ResponseModel.h"

namespace CNwarePlugin {

enum class VMStatus {
    ABNORMAL = 0,
    RUNNING,
    SHUTDOWN,
    PAUSED
};

struct DomainBridgeInterface {
    std::string m_ip;
    std::string m_portGroupId;

    BEGIN_SERIAL_MEMEBER;
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_ip, ip);
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_portGroupId, portGroupId);
    END_SERIAL_MEMEBER;
};

struct DomainCdromDevices {
    std::string m_sourceFile;

    BEGIN_SERIAL_MEMEBER;
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_sourceFile, sourceFile);
    END_SERIAL_MEMEBER;
};

struct DomainBindCpuRsp {
    std::string m_cpus;
    std::string m_vcpu;

    BEGIN_SERIAL_MEMEBER;
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_cpus, cpus);
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_vcpu, vcpu);
    END_SERIAL_MEMEBER;
};

struct DomainCpu {
    uint32_t m_arch = 0;
    uint32_t m_sockets = 0;
    uint32_t m_cores = 2;
    uint32_t m_current = 0;
    uint32_t m_shares = 0;
    uint32_t m_mode = 0;
    uint32_t m_gurantees = 0;
    uint32_t m_quota = 0;
    std::vector<DomainBindCpuRsp> m_bindCpu;

    BEGIN_SERIAL_MEMEBER;
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_arch, arch);
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_sockets, sockets);
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_cores, cores);
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_current, current);
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_shares, shares);
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_mode, mode);
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_gurantees, gurantees);
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_shares, shares);
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_quota, quota);
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_bindCpu, bindCpu);
    END_SERIAL_MEMEBER;
};

struct DomainMemory {
    bool m_autoMem = true;
    uint64_t m_currentMemory = 0;
    uint64_t m_memory = 0;
    uint32_t m_locked = 0;
    uint64_t m_limit = 0;
    std::string m_mode;
    std::string m_nodeSet;
    bool m_isHugePages = false;
    bool m_isOpenVnuma = false;

    BEGIN_SERIAL_MEMEBER;
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_autoMem, autoMem);
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_currentMemory, currentMemory);
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_memory, memory);
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_locked, locked);
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_limit, limit);
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_mode, mode);
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_nodeSet, nodeSet);
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_isHugePages, isHugePages);
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_isOpenVnuma, isOpenVnuma);
    END_SERIAL_MEMEBER;
};

struct VMPciDev {
    std::string m_address;

    BEGIN_SERIAL_MEMEBER;
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_address, address);
    END_SERIAL_MEMEBER;
};
struct CNwareVMInfo {
    std::string m_id;
    std::string m_name;
    std::string m_hostId;
    uint32_t m_status = 0;
    bool m_autoMigrate = false;
    uint32_t m_blkiotune = 0;
    std::vector<BridgeInterfaces> m_bridgeInterfaces;
    std::vector<DomainCdromDevices> m_cdromDevices;
    uint32_t m_clock = 0;
    uint32_t m_bootType = 0;
    DomainCpu m_cpu;
    std::string m_domainName;
    bool m_isHardwareEncrypt = false;
    DomainMemory m_memory;
    uint32_t m_osType = 0;
    std::string m_osVersion;
    std::vector<VMPciDev> m_pciDevices;
    std::string m_remark;

    BEGIN_SERIAL_MEMEBER;
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_id, id);
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_name, name);
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_hostId, hostId);
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_status, status);
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_autoMigrate, autoMigrate);
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_blkiotune, blkiotune);
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_bridgeInterfaces, bridgeInterfaces);
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_cdromDevices, cdromDevices);
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_clock, clock);
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_bootType, bootType);
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_cpu, cpu);
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_domainName, domainName);
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_isHardwareEncrypt, isHardwareEncrypt);
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_memory, memory);
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_osType, osType);
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_osVersion, osVersion);
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_pciDevices, pciDevices);
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_remark, remark);
    END_SERIAL_MEMEBER;
};

class GetVMInfoResponse : public VirtPlugin::ResponseModel {
public:
    GetVMInfoResponse() {}
    ~GetVMInfoResponse() {}

    bool Serial()
    {
        if (m_body.empty()) {
            return false;
        }
        if (!Module::JsonHelper::JsonStringToStruct(m_body, m_vmInfo)) {
            ERRLOG("Convert %s failed.", WIPE_SENSITIVE(m_body).c_str());
            return false;
        }
        return true;
    }

    CNwareVMInfo GetInfo()
    {
        return m_vmInfo;
    }

private:
    CNwareVMInfo m_vmInfo;
};
};

#endif