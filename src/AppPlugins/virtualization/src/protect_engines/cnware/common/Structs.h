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
#ifndef CNWARE_STRUCTS_H
#define CNWARE_STRUCTS_H
#include <vector>
#include <string>
#include <common/JsonHelper.h>
#include <common/Structs.h>

namespace CNwarePlugin {

struct LoginRequestBody {
    std::string m_user;
    std::string m_pwd;
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_user, user)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_pwd, pwd)
    END_SERIAL_MEMEBER
};

struct LoginResponseBody {
    int64_t m_errorCode = 0;
    std::string m_message;
    BEGIN_SERIAL_MEMEBER
        SERIAL_MEMBER_TO_SPECIFIED_NAME(m_errorCode, errorCode)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(m_message, message)
    END_SERIAL_MEMEBER
};

struct CNwareVersionInfo {
    std::string m_productName;
    std::string m_copyrightInformation;
    std::string m_officialWebsite;
    std::string m_productVersion;
    std::string m_productDesc1;
    std::string m_productDesc2;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_productName, productName)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_copyrightInformation, copyrightInformation)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_officialWebsite, officialWebsite)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_productVersion, productVersion)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_productDesc1, productDesc1)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_productDesc2, productDesc2)
    END_SERIAL_MEMEBER
};

struct PortGroup {
    std::string m_id;
    std::string m_name;
    int32_t m_vlanId = 0;
    std::string m_dvswitchId;
    std::string m_dvswitchName;
    std::string m_networkStrategy;
    int32_t m_portNum = 0;
    bool m_distributedFlag;
    BEGIN_SERIAL_MEMEBER;
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_id, id)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_name, name)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_vlanId, vlanId)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_dvswitchId, dvswitchId)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_dvswitchName, dvswitchName)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_networkStrategy, networkStrategy)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_portNum, portNum)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_distributedFlag, distributedFlag)
    END_SERIAL_MEMEBER;
};

struct VolExist {
    bool m_exist {false};

    BEGIN_SERIAL_MEMEBER;
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_exist, exist)
    END_SERIAL_MEMEBER;
};

struct VolExistInfo {
    VolExist m_data;

    BEGIN_SERIAL_MEMEBER;
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_data, data)
    END_SERIAL_MEMEBER;
};

enum class CNWareStorageType {
    DEFAULT = 0,
    FC = 1,
    ISCSI = 2,
    NFS = 3,
    DISTRIBUTED = 4,
    LOCAL = 5,
    FS_SHARE = 6
};

struct DomainDiskDevicesResp {
    std::string m_bus;                // 总线类型
    std::string m_dev;                // 设备号
    std::string m_sourceFile;         // 源文件
    int32_t m_bootOrder;              // 引导顺序
    int64_t m_capacity = 1;           // 可用容量
    std::string m_volId;              // 卷ID
    std::string m_storagePoolId;      // 存储池ID
    std::string m_storagePoolName;    // 存储池名称
    // 1.fc存储 2.iscsi存储 3.nfs存储 4.分布式存储 5.本地存储 6.fs共享存储
    int32_t m_storagePoolType = static_cast<int>(CNWareStorageType::DEFAULT);
    std::string m_preallocation;      // 制备类型 1.off(精简置备) 2.falloc(厚置备延迟置零) 3.full(厚置备置零)
    bool m_shareable = false;         // 是否是共享卷
    bool m_status = false;            // 磁盘状态
    int32_t m_cache = 4;              // 缓存方式
    int32_t m_busType = 1;            // 总线 1.高速硬盘 2.IDE硬盘 3.SCSI硬盘 4.SATA硬盘 5.USB硬盘(arm 和mips平台 只有scsi磁盘)
    int32_t m_diskType = 1;           // 卷类型
    int32_t m_driverType;             // 存储格式 1.智能 2.高速
    bool m_isEncrypt = false;         // 是否加密
    int64_t m_ioHangTimeout = 1800000;  // io超时时间
    int64_t m_readBytesSecond = 0;    // IO读速率
    int64_t m_readIops = 0;           // 读IOPS
    int64_t m_writeBytesSecond = 0;   // IO写速率
    int64_t m_writeIops = 0;          // 写IOPS

    BEGIN_SERIAL_MEMEBER;
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_bus, bus)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_dev, dev)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_sourceFile, sourceFile)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_bootOrder, bootOrder)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_capacity, capacity)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_volId, volId)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_shareable, shareable)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_status, status)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_preallocation, preallocation)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_storagePoolId, storagePoolId)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_storagePoolName, storagePoolName)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_storagePoolType, storagePoolType)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_cache, cache)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_busType, busType)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_diskType, diskType)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_driverType, driverType)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_isEncrypt, isEncrypt)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_ioHangTimeout, ioHangTimeout)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_readBytesSecond, readBytesSecond)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_readIops, readIops)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_writeBytesSecond, writeBytesSecond)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_writeIops, writeIops)
    END_SERIAL_MEMEBER;
};

struct DomainDiskInfoResponse {
    std::vector<struct DomainDiskDevicesResp> m_diskDevices;

    BEGIN_SERIAL_MEMEBER;
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_diskDevices, diskDevices)
    END_SERIAL_MEMEBER;
};

struct InterfacePortPair {
    VirtPlugin::BridgeInterfaceInfo m_bridge;
    PortGroup m_portGroup;

    BEGIN_SERIAL_MEMEBER;
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_bridge, bridge)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_portGroup, portGroup)
    END_SERIAL_MEMEBER;
};

struct InterfacePortPairList {
    std::vector<InterfacePortPair> m_detail;

    BEGIN_SERIAL_MEMEBER;
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_detail, detail)
    END_SERIAL_MEMEBER;
};

enum CNwareType {
    Pool,
    Cluster,
    Host,
    VM,
    Disk,
    All
};

enum class CNwareDiskStatus {
    NORMAL = 1,
    ABNORMAL = 2
};

struct CNwareDiskUserList {
    std::string m_domainName;
    std::string m_domainId;
    std::string m_bus;
    std::string m_dev;
    std::string m_hide;

    BEGIN_SERIAL_MEMEBER;
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_domainName, domainName);
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_domainId, domainId);
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_bus, bus);
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_dev, dev);
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_hide, hide);
    END_SERIAL_MEMEBER;
};

struct CNwareDiskInfo {
    std::string m_id = "";
    std::string m_name;
    std::string m_hostId;
    std::string m_path;
    uint32_t m_status = static_cast<int>(CNwareDiskStatus::ABNORMAL);
    uint64_t m_capacity = 0;
    std::string mStoragePoolId;
    std::vector<CNwareDiskUserList> m_userList;

    BEGIN_SERIAL_MEMEBER;
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_id, id);
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_name, name);
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_hostId, hostId);
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_path, path);
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_status, status);
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_capacity, capacity);
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_userList, userList);
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mStoragePoolId, storagePoolId);
    END_SERIAL_MEMEBER;
};

struct CNwareDiskData {
    CNwareDiskInfo m_data;

    BEGIN_SERIAL_MEMEBER;
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_data, data);
    END_SERIAL_MEMEBER;
};

struct BridgeInterfaces {
    std::string m_bridge;
    bool m_isEnabled;
    uint32_t m_bootOrder;
    std::string m_interfaceId;
    std::string m_portGroupId;
    std::string m_portGroupName;
    std::string m_portGroupType;
    std::string m_ip;
    bool m_isVhostDriver = false;
    uint32_t m_queues = 0;
    std::string m_mac;
    uint32_t m_model = 0;
    uint32_t m_mtu = 0;
    uint32_t m_networkType = 0;
    bool m_noIpMacSpoofing = false;
    std::string m_vfName;
    std::string m_vlanId;
    std::string m_networkStrategy;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_bridge, bridge)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_isEnabled, isEnabled)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_bootOrder, bootOrder)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_interfaceId, interfaceId)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_portGroupId, portGroupId)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_portGroupName, portGroupName)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_portGroupType, portGroupType)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_ip, ip)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_isVhostDriver, isVhostDriver)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_queues, queues)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_mac, mac)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_model, model)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_mtu, mtu)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_networkType, networkType)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_noIpMacSpoofing, noIpMacSpoofing)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_vfName, vfName)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_vlanId, vlanId)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_networkStrategy, networkStrategy)
    END_SERIAL_MEMEBER
};

struct BridgeInterfacesArray {
    std::vector<BridgeInterfaces> m_bridgeInterfaces;

    BEGIN_SERIAL_MEMEBER;
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_bridgeInterfaces, bridgeInterfaces);
    END_SERIAL_MEMEBER;
};

struct VswitchsRes {
    std::string m_id;
    std::string m_hostId;
    uint32_t m_slots;
    std::string m_name;
    std::string m_mode;
    std::string m_pnics;
    std::string m_ip;
    uint32_t m_status;
    std::string m_vswitchType;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_id, id)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_hostId, hostId)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_slots, slots)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_name, name)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_mode, mode)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_pnics, pnics)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_ip, ip)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_status, status)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_vswitchType, vswitchType)
    END_SERIAL_MEMEBER
};

enum class VolSource {
    OLD_DISK = 0,
    CREATE_NEW_DISK
};

struct CNwareError {
    int64_t m_errorCode;
    std::string m_message;
    std::string m_exception;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_errorCode, errorCode)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_message, message)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_exception, exception)
    END_SERIAL_MEMEBER
};

struct HostInfo {
    std::string clusterId;
    std::string clusterName;
    std::string id;
    std::string ip;
    std::string cpuArchitecture; /* 主机CPU架构: x86_64 aarch64 mips64el sw_64 loongarch64 */
    std::string hostname;
    std::string name;
    bool isMaintain {false};
    bool isConnected {false};
    int32_t cpuThreads;
    int32_t cpuCores;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMEBER(clusterId)
    SERIAL_MEMEBER(clusterName)
    SERIAL_MEMEBER(id)
    SERIAL_MEMEBER(ip)
    SERIAL_MEMEBER(cpuArchitecture)
    SERIAL_MEMEBER(hostname)
    SERIAL_MEMEBER(name)
    SERIAL_MEMEBER(isMaintain)
    SERIAL_MEMEBER(isConnected)
    SERIAL_MEMEBER(cpuThreads)
    SERIAL_MEMEBER(cpuCores)
    END_SERIAL_MEMEBER
};
struct HostResource {
    double cpuRate = 0.0;
    double memoryRate = 0.0;
    double storageRate = 0.0;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMEBER(cpuRate)
    SERIAL_MEMEBER(memoryRate)
    SERIAL_MEMEBER(storageRate)
    END_SERIAL_MEMEBER
};
}
#endif