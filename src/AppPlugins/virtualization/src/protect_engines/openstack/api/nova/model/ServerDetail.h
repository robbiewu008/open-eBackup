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
#ifndef OPENSTACK_SERVER_DETAIL_H
#define OPENSTACK_SERVER_DETAIL_H
 
#include <string>
#include <vector>
#include <json/json.h>
#include <common/JsonHelper.h>
#include "protect_engines/openstack/common/OpenStackMacros.h"
#include "protect_engines/openstack/api/neutron/model/NetworkDetail.h"
#include "protect_engines/openstack/api/cinder/model/VolumeDetail.h"
 
OPENSTACK_PLUGIN_NAMESPACE_BEGIN
/**
 *  快捷链接信息
 */
struct Link {
    std::string m_href;     // 对应快捷链接
    std::string m_rel;     // 快捷链接标记名称，枚举值：self, bookmark, alternate
 
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_href, href)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_rel, rel)
    END_SERIAL_MEMEBER
};

/**
 *  云服务器安全组信息
 */
struct ServerSecurityGroup {
    std::string m_name;     // 安全组名称
 
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_name, name)
    END_SERIAL_MEMEBER
};

/**
 *  云服务器网络配置信息
 */
struct ServerNetWork {
    std::string m_osEXTIPSMACmacAddr;     // MAC地址
    std::string m_osEXTIPStype;     // 分配IP地址方式
    std::string m_addr;     // IP地址信息
    int32_t m_version;     // IP地址类型，值为4或6
 
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_osEXTIPSMACmacAddr, OS-EXT-IPS-MAC:mac_addr)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_osEXTIPStype, OS-EXT-IPS:type)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_addr, addr)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_version, version)
    END_SERIAL_MEMEBER
};

/**
 *  云服务器镜像信息
 */
struct ServerImage {
    std::string m_uuid;     // 镜像ID
    std::vector<Link> m_links;     // 镜像相关标记快捷链接信息
 
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_uuid, id)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_links, links)
    END_SERIAL_MEMEBER
};

struct ServerFlavorExtraSpecs {
    std::string m_key;
    std::string m_value;
 
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_key, key)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_value, value)
    END_SERIAL_MEMEBER
};

/**
 *  云服务器类型信息
 */
struct ServerFlavor {
    int32_t m_vcpus;    // 分配虚拟CPU的个数
    int32_t m_ram;      // flavor的RAM大小，单位 MiB
    int32_t m_disk;     // root disk大小，单位 GiB
    int32_t m_osFlvExtDataEphemeral;    // 临时磁盘大小，单位 GiB
    std::string m_osFlvDisabled;    // 是否被管理禁用，管理可见
    std::string m_osFlavorAccessIsPublic;    // 是否对其他项目可用，默认为true
    int32_t m_swap;     // swap磁盘大小，单位 MiB
    std::string m_name; // flavor的名称
    std::string m_id;    //flavor的id
    ServerFlavorExtraSpecs m_extraspecs;
 
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_vcpus, vcpus)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_ram, ram)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_disk, disk)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_osFlvExtDataEphemeral, OS-FLV-EXT-DATA:ephemeral)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_osFlvDisabled, OS-FLV-DISABLED:disabled)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_osFlavorAccessIsPublic, os-flavor-access:is_public)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_swap, swap)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_name, name)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_id, id)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_extraspecs, extraspecs)
    END_SERIAL_MEMEBER
};

struct ServerFlavorInfo {
    ServerFlavor m_flavor;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_flavor, flavor)
    END_SERIAL_MEMEBER
};

struct ServerExtendInfo {
    ServerFlavor m_flavor;
    std::vector<Network> m_networks;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_flavor, flavor)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_networks, network)
    END_SERIAL_MEMEBER
};

struct ServerFlavorList {
    std::vector<ServerFlavor> m_flavors;
 
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_flavors, flavors)
    END_SERIAL_MEMEBER
};

 /**
 *  云服务器卷信息
 */
struct ServerVolume {
    std::string m_uuid;     // 云硬盘ID
 
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_uuid, id)
    END_SERIAL_MEMEBER
};

struct HostMetaData {
    std::string m_softDelete;
    std::string m_os_type;
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_softDelete, __softdelete)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_os_type, os_type)
    END_SERIAL_MEMEBER
};

struct VMFaultInfo {
    uint64_t m_code = 0;
    std::string m_message;
    std::string m_details;
    std::string m_created;
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_code, code)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_message, message)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_details, details)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_created, created)
    END_SERIAL_MEMEBER
};

/**
 *  虚拟机详情响应结构
 */
struct OpenStackServerInfo {
    std::string m_name;     // 名称
    std::string m_uuid;     // 唯一标志, uuid
    std::string m_status;     // 状态
    std::string m_created;     // 创建时间
    std::string m_updated;     // 更新时间
    ServerFlavor m_flavor;  // 规格
    std::string m_tenantId;     // 项目ID
    std::string m_keyName;     // SSH密钥名称
    std::string m_userId;     // 用户ID
    std::string m_hostId;     // 主机ID
    std::vector<ServerSecurityGroup> m_securityGroups;     // 所属安全组列表
    std::string m_oSDCFdiskConfig;     // 磁盘配置方式，AUTO or MANUAL
    std::string m_oSEXTAZavailabilityZone;     // 可用分区
    std::string m_oSEXTSRVATTRhost;     // 虚拟机宿主名称
    std::string m_oSEXTSRVATTRhypervisorHostname;     // hypervisor主机名
    std::string m_oSEXTSRVATTRinstanceName;     // 实例ID
    int32_t m_oSEXTSTSpowerState;     // 电源状态
    std::string m_oSEXTSTStaskState;     // 任务状态
    std::string m_oSEXTSTSvmState;     // 虚拟机状态
    std::string m_oSSRVUSGlaunchedAt;     // 启动时间
    std::string m_oSSRVUSGterminatedAt;     // 删除时间
    std::vector<ServerVolume> m_osExtendedVolumesvolumesAttached;     // 挂载的云磁盘信息
    std::string m_description;     // 描述信息
    std::string m_hostStatus;     // 宿主机状态
    std::string m_oSEXTSRVATTRhostname;     // 云服务器hostname
    std::string m_oSEXTSRVATTRreservationId;     // 预留ID，批量创建虚拟机时可以用来识别虚拟机
    int32_t m_oSEXTSRVATTRlaunchIndex;     // 批量创建虚拟机时，虚拟机的创建顺序
    std::vector<std::string> m_tags;     // 标签ID列表
    std::string m_accessIPv4;     // 预留属性
    std::string m_accessIPv6;     // 预留属性
    std::vector<std::string> m_addresses;    // 虚拟机网卡
    std::vector<std::string> m_addressesIp;    // 虚拟机ip
    std::string m_networks;    // 虚拟机网络名字
    std::string m_configDrive;     // 是否使用配置磁盘
    int32_t m_progress;     // 操作云服务器时的进度百分比，最小值0，最大值100
    HostMetaData m_metadata;
    VMFaultInfo m_faultInfo;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_name, name)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_uuid, id)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_status, status)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_created, created)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_updated, updated)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_flavor, flavor)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_tenantId, tenant_id)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_keyName, key_name)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_userId, user_id)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_hostId, hostId)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_securityGroups, security_groups)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_oSDCFdiskConfig, OS-DCF:diskConfig)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_oSEXTAZavailabilityZone, OS-EXT-AZ:availability_zone)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_oSEXTSRVATTRhost, OS-EXT-SRV-ATTR:host)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_oSEXTSRVATTRhypervisorHostname, OS-EXT-SRV-ATTR:hypervisor_hostname)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_oSEXTSRVATTRinstanceName, OS-EXT-SRV-ATTR:instance_name)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_oSEXTSTSpowerState, OS-EXT-STS:power_state)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_oSEXTSTStaskState, OS-EXT-STS:task_state)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_oSEXTSTSvmState, OS-EXT-STS:vm_state)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_oSSRVUSGlaunchedAt, OS-SRV-USG:launched_at)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_oSSRVUSGterminatedAt, OS-SRV-USG:terminated_at)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_osExtendedVolumesvolumesAttached, os-extended-volumes:volumes_attached)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_description, description)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_hostStatus, host_status)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_oSEXTSRVATTRhostname, OS-EXT-SRV-ATTR:hypervisor_hostname)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_oSEXTSRVATTRreservationId, OS-EXT-SRV-ATTR:reservation_id)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_oSEXTSRVATTRlaunchIndex, OS-EXT-SRV-ATTR:launch_index)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_tags, tags)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_accessIPv4, accessIPv4)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_accessIPv6, accessIPv6)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_configDrive, config_drive)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_progress, progress)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_metadata, metadata)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_faultInfo, fault)
    END_SERIAL_MEMEBER
};

struct ServerDetail {
    OpenStackServerInfo m_hostServerInfo;
 
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_hostServerInfo, server)
    END_SERIAL_MEMEBER
};

struct ServerList {
    std::vector<OpenStackServerInfo> m_serverList;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_serverList, servers)
    END_SERIAL_MEMEBER
};

struct AttachVolumeDetail {
    VolumeAttachment m_volumeAttachment;
 
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_volumeAttachment, volumeAttachment)
    END_SERIAL_MEMEBER
};
OPENSTACK_PLUGIN_NAMESPACE_END
#endif