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
#ifndef OPENSTACK_MESSAGE_INFO_H
#define OPENSTACK_MESSAGE_INFO_H

#include <vector>
#include <string>
#include "cstdint"

#include "protect_engines/openstack/common/OpenStackMacros.h"
#include "thrift_interface/ApplicationProtectBaseDataType_types.h"
#include "thrift_interface/ApplicationProtectPlugin_types.h"
#include "thrift_interface/ApplicationProtectFramework_types.h"

OPENSTACK_PLUGIN_NAMESPACE_BEGIN

struct CloudServerResource {
    std::string id;
    std::string name;
    std::string projectId;
    std::string m_domainId;
    std::string status;
    std::string m_domainName;
    std::string m_flavorId;
    std::string m_networks;
    std::string vmIp;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(id, id)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(name, name)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(projectId, projectId)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(status, status)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_domainId, domainId)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_domainName, domainName)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_flavorId, flavorId)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_networks, networks)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(vmIp, vm_ip)
    END_SERIAL_MEMEBER
};

struct FlavorResouce {
    std::string m_name;
    std::string m_id;
    std::string m_disabled;
    std::string m_public;
 
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_id, id)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_name, name)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_disabled, disabled)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_public, public)
    END_SERIAL_MEMEBER
};

struct VolumeResource {
    std::string id;
    std::string name;
    std::string bootable;
    std::string size;
    std::string architecture;
    bool shareable;
    std::string m_fullClone;
    std::string m_volumeType;
    std::string mDevice;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(id, id)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(name, name)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(bootable, bootable)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(size, size)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(architecture, architecture)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(shareable, shareable)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_fullClone, fullClone)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_volumeType, volumeType)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mDevice, Device)
    END_SERIAL_MEMEBER
};

struct VolumeTypeResource {
    std::string m_name;
    std::string m_id;
    std::string m_description;
    bool m_isPublic;
    std::string volumeBackEndName;
    std::string availabilityZone;
 
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_name, name)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_id, id)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_isPublic, isPublic)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_description, description)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(volumeBackEndName, volume_backend_name)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(availabilityZone, HW:availability_zone)
    END_SERIAL_MEMEBER
};

struct NetworkResource {
    std::string m_id;
    std::string m_name;
    bool m_shared;
    std::string m_status;
    std::string m_projectId;    // 默认域service项目id
    std::string m_networkType;    // 此网络映射到的物理网络的类型。例如，平面、vlan、vxlan或gre。有效值取决于网络后端
    std::string m_physicalNetwork;    // 实施此网络/网段的物理网络

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_id, id)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_name, name)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_shared, shared)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_status, status)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_projectId, projectId)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_networkType, networkType)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_physicalNetwork, physicalNetwork)
    END_SERIAL_MEMEBER
};

struct AvailabilityZoneResource {
    std::string zoneName;
    std::string available;
    std::string hosts;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(zoneName, zoneName)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(available, available)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(hosts, hosts)
    END_SERIAL_MEMEBER
};

// 运营面对外http错误码定义
enum class OpenStackExternalErrorCode {
    // 一切正常
    CURLE_OK = 0,
    // 被回调中止，回调将中止返回
    CURLE_ABORTED_BY_CALLBACK = 42,
    // 远程服务器的SSL证书不正常
    CURLE_PEER_FAILED_VERIFICATION = 60,
    // 读取SSL的CA证书时出现问题（路径或访问权限）
    CURLE_SSL_CACERT_BADFILE = 77,
    // 读取SSL的吊销列表出现问题
    CURLE_SSL_CACRL_BADFILE = 82,
};

const std::set<int32_t> OpenStackCertErrorCode = {
    static_cast<int32_t>(OpenStackExternalErrorCode::CURLE_ABORTED_BY_CALLBACK),
    static_cast<int32_t>(OpenStackExternalErrorCode::CURLE_PEER_FAILED_VERIFICATION),
    static_cast<int32_t>(OpenStackExternalErrorCode::CURLE_SSL_CACERT_BADFILE),
    static_cast<int32_t>(OpenStackExternalErrorCode::CURLE_SSL_CACRL_BADFILE)
};

OPENSTACK_PLUGIN_NAMESPACE_END
#endif