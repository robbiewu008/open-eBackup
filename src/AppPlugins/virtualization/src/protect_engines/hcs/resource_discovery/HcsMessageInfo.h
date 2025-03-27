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
#ifndef HCS_MESSAGE_INFO_H
#define HCS_MESSAGE_INFO_H

#include <vector>
#include <string>
#include "cstdint"
#include "protect_engines/hcs/api/sc/model/ProjectDetail.h"
#include "protect_engines/hcs/common/HcsMacros.h"
#include "thrift_interface/ApplicationProtectBaseDataType_types.h"
#include "thrift_interface/ApplicationProtectPlugin_types.h"
#include "thrift_interface/ApplicationProtectFramework_types.h"

namespace HcsPlugin {
struct StoragInfoReq {
    std::string m_userName;
    std::string m_password;
    std::string m_ip;
    std::string m_storage_type;
    int32_t m_port;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_userName, username)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_password, password)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_ip, ip)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_port, port)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_storage_type, storageType)
    END_SERIAL_MEMEBER
};

struct HostIdAndStatus {
    std::string m_vmId;
    std::string m_status;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_vmId, id)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_status, status)
    END_SERIAL_MEMEBER
};


struct ResourceProjectExtendInfo {
    std::string m_projectId;
    std::string m_regionId;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_projectId, projectId)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_regionId, regionId)
    END_SERIAL_MEMEBER
};

struct ResourceExtendInfoVdcs {
    std::string m_id;
    std::string m_vdcName;
    std::string m_name;
    std::string m_passwd;
    std::string m_domainId;
    std::string m_domainName;
    std::string m_userName;
    int32_t m_level;
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_id, id)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_name, name)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_vdcName, vdcName)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_passwd, passwd)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_domainName, domainName)   // pm不用下发
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_userName, userName)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_domainId, domainId)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_level, level) // pm不用下发
    END_SERIAL_MEMEBER
};

struct TaskVdcList {
    std::vector<ResourceExtendInfoVdcs> m_resourceExtendInfoVdcs;
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_resourceExtendInfoVdcs, vdcInfos)
    END_SERIAL_MEMEBER
};

struct ResourceVdc {
    ResourceVdc() {}
    explicit ResourceVdc(const ResourceExtendInfoVdcs& reg)
        : m_name(reg.m_name), m_passwd(reg.m_passwd), m_domainName(reg.m_domainName),
        m_vdcId(reg.m_id), m_vdcName(reg.m_vdcName)
    {}
    std::string m_vdcId;
    std::string m_vdcName;
    std::string m_name;
    std::string m_passwd;
    std::string m_domainName;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_vdcId, vdcId)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_vdcName, vdcName)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_name, name)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_passwd, passwd)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_domainName, domainName)
    END_SERIAL_MEMEBER
};

struct ResourceVdcInfo {
    std::string m_id;
    std::string m_name;
    std::string m_domainName;
    std::string m_domainId;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_id, id)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_name, name)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_domainName, domainName)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_domainId, domainId)
    END_SERIAL_MEMEBER
};

struct ErrInfo {
    std::string m_errCode;
    std::string m_errMessage;
    std::string m_bodyErr;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_errCode, code)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_errMessage, message)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_bodyErr, bodyErr)
    END_SERIAL_MEMEBER
};

struct Region {
    Region() {}
    explicit Region(const RegionDetail& reg)
        : m_regId(reg.m_regionId), m_regName(reg.m_regionName.m_enUs), m_regStatus(reg.m_regionStatus)
    {}
    std::string m_regId;
    std::string m_regName;
    std::string m_regStatus;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_regId, regionId)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_regName, regionName)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_regStatus, regionStatus)
    END_SERIAL_MEMEBER
};

struct DiskInfo {
    std::string m_id;
    std::string m_name;
    std::string m_mode;
    std::string m_attr;
    std::string m_size;
    std::string m_lunWWN;
    std::string m_architecture;
    std::string m_storageManagerIp;
    std::string m_storageType;
    std::string m_encrypted;
    std::string m_cmkId;
    std::string m_cipher;
    bool m_shareable;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_id, id)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_name, name)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_mode, mode)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_attr, attr)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_size, size)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_lunWWN, lunWWN)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_architecture, architecture)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_storageManagerIp, storageManagerIp)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_storageType, storageType)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_shareable, shareable)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_encrypted, systemEncrypted)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_cmkId, systemCmkId)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_cipher, cipher)
    END_SERIAL_MEMEBER
};

struct HostResource {
    std::string m_id;
    std::string m_name;
    std::string m_regionId;
    std::string m_projectId;
    std::string m_status;
    std::string m_osType;
    std::vector<std::string> m_vmIp;
    std::vector<DiskInfo> m_diskInfo;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_id, id)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_name, name)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_regionId, regionId)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_projectId, projectId)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_status, status)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_diskInfo, diskInfo)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_osType, os_type)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_vmIp, vm_ip)
    END_SERIAL_MEMEBER
};

struct DiskInfoString {
    std::string m_diskInfo;
    std::string m_userName;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_diskInfo, diskInfo)
    std::string m_userName;
    END_SERIAL_MEMEBER
};

struct HostExtendInfo {
    std::string m_hostResource;
    std::string m_osType;
    std::string m_vmIp;
    std::string m_userName;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_hostResource, host)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_osType, os_type)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_vmIp, vm_ip)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_userName, userName)
    END_SERIAL_MEMEBER
};

struct ProjectResource {
    ProjectResource() {}
    ProjectResource(const ProjectDetail& proDetail, const ResourceExtendInfoVdcs &vdcInfo)
    {
        m_id = proDetail.m_projectDetailInfo.m_id;
        m_name = proDetail.m_projectDetailInfo.m_name;
        m_tenantId = proDetail.m_projectDetailInfo.m_tenantId;
        m_tenantName = proDetail.m_projectDetailInfo.m_tenantName;

        m_resourceVdc = ResourceVdc(vdcInfo);
        
        for (const auto& it : proDetail.m_projectDetailInfo.m_regions) {
            Region reg(it);
            m_region.push_back(reg);
        }
    }
    std::string m_id;
    std::string m_name;
    std::string m_tenantId;
    std::string m_tenantName;
    std::vector<Region> m_region;
    ResourceVdc m_resourceVdc;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_id, id)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_name, name)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_tenantId, tenantId)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_tenantName, tenantName)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_region, regions)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_resourceVdc, vdcInfo)  // todo
    END_SERIAL_MEMEBER
};

struct ProjectExtendInfo {
    std::string m_projectResource;
    std::string m_userName;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_projectResource, project)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_userName, userName)
    END_SERIAL_MEMEBER
};

struct ProjectList {
    std::string m_id;
    std::vector<Region> m_region;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_id, id)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_region, region)
    END_SERIAL_MEMEBER
};

struct InvalidVdcInfo {
    std::string m_invalidUserName;
    std::string m_errorCode;
    std::string m_errorMsg;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_invalidUserName, invalidUser)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_errorCode, errorCode)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_errorMsg, errorMsg)
    END_SERIAL_MEMEBER
};

class HcsOpServiceGetInfo {
public:
    std::string m_domain;
    std::string m_tokenInfo;
    std::string m_projectId;
    std::string m_project_id;
    std::string m_regionId;
    std::string m_region_id;
    std::string m_isOpService;
    
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_domain, domain)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_tokenInfo, hcs_token)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_isOpService, isOpService)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_projectId, projectId)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_project_id, project_id)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_regionId, regionId)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_region_id, region_id)
    END_SERIAL_MEMEBER

    ~HcsOpServiceGetInfo() {
        Module::CleanMemoryPwd(m_tokenInfo);
    }
};

class HcsOpServiceTokenInfo {
public:
    std::string m_expiresAt;
    std::string m_token;
    std::string m_tokenStr;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_expiresAt, expires_at)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_token, token)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_tokenStr, tokenStr)
    END_SERIAL_MEMEBER

    ~HcsOpServiceTokenInfo() {
        Module::CleanMemoryPwd(m_token);
        Module::CleanMemoryPwd(m_tokenStr);
    }
};
}

#endif