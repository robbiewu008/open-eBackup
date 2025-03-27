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
#ifndef __HYPERV_RESOURCE_ACCESS_H__
#define __HYPERV_RESOURCE_ACCESS_H__

#include <vector>
#include <map>
#include <json/json.h>
#include "common/JsonHelper.h"
#include "common/MpString.h"
#include "thrift_interface/ApplicationProtectBaseDataType_types.h"
#include "thrift_interface/ApplicationProtectPlugin_types.h"
#include "thrift_interface/ApplicationProtectFramework_types.h"

using namespace AppProtect;

namespace HyperVPlugin {
const int32_t SUCCESS = 0;
const int32_t FAILED = -1;

struct HyperVClusterInfo {
    std::string m_name;
    std::string m_id;
    std::string m_ipAddresses;
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_name, Name)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_id, ID)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_ipAddresses, IPAddresses)
    END_SERIAL_MEMEBER
};

struct HyperVClusterList {
    std::list<HyperVClusterInfo> m_clusterList;
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_clusterList, result)
    END_SERIAL_MEMEBER
};

/* {
    "Name":  "hyperv03.hyper.com",
    "ID":  "578896ad-ade9-4822-b413-d558bd7a369b",
    "OverallState":  0,
    "VirtualizationPlatform":  2,
    "HyperVVersion":  "10.0.14393.0"
} */
struct HyperVHostInfo {
    std::string m_name;
    std::string m_id;
    std::string m_overallState;
    std::string m_virtualizationPlatform;
    std::string m_hyperVVersion;
    std::string m_hostCluster;
    std::string m_fQDN;
    std::string m_ips;
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_name, Name)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_id, ID)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_overallState, OverallState)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_virtualizationPlatform, VirtualizationPlatform)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_hyperVVersion, HyperVVersion)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_hostCluster, HostCluster)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_fQDN, FQDN)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_ips, IPAddress)
    END_SERIAL_MEMEBER
};

struct HyperVHostList {
    std::list<HyperVHostInfo> m_hostList;
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_hostList, result)
    END_SERIAL_MEMEBER
};

struct HyperVHostExtendInfo {
    std::string m_state;
    std::string m_status;
    std::string m_agentSN;
    std::string m_hostCluster;
    std::string m_fQDN;
    std::string m_ipList;
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_state, State)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_status, status)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_agentSN, AgentSN)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_hostCluster, HostCluster)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_fQDN, FQDN)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_ipList, ipList)
    END_SERIAL_MEMEBER
};

struct HyperVNodeInfo {
    std::string m_name;
    std::string m_id;
    std::string m_overallState;
    std::string m_virtualizationPlatform;
    std::string m_hyperVVersion;
    std::string m_hostCluster;
    std::string m_fQDN;
    std::string m_ips;
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_name, NodeName)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_id, SerialNumber)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_overallState, State)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_virtualizationPlatform, SerialNumber)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_hyperVVersion, MajorVersion)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_hostCluster, Cluster)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_fQDN, FQDN)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_ips, IPAddress)
    END_SERIAL_MEMEBER
};

struct HyperVNodeList {
    std::list<HyperVNodeInfo> m_hostList;
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_hostList, result)
    END_SERIAL_MEMEBER
};

struct HyperVIpInfo {
    std::string m_address;
    std::string m_addressFamily;
    std::string m_isIPv6LinkLocal;
    std::string m_scopeId;
    std::string m_iPAddressToString;
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_address, Address)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_addressFamily, AddressFamily)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_isIPv6LinkLocal, IsIPv6LinkLocal)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_scopeId, ScopeId)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_iPAddressToString, IPAddressToString)
    END_SERIAL_MEMEBER
};

struct HyperVIpList {
    std::list<HyperVIpInfo> m_ipList;
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_ipList, result)
    END_SERIAL_MEMEBER
};

struct HyperVVMIpList {
    std::list<std::string> m_ipList;
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_ipList, result)
    END_SERIAL_MEMEBER
};

/* {
    "VMName":  "测试虚拟机1",
    "VMId":  "4f27bc34-0d01-4057-bc93-a4c556d61644",
    "State":  3,
    "Version":  "5.0",
    "Generation":  2
} */
struct HyperVVMInfo {
    std::string m_vmName;
    std::string m_vmId;
    std::string m_state;
    std::string m_version;
    std::string m_generation;
    std::string m_configurationLocation;
    std::string m_checkpointFileLocation;
    std::string m_ips;
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_vmName, Name)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_vmId, Id)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_state, State)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_version, Version)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_generation, Generation)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_configurationLocation, ConfigurationLocation)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_checkpointFileLocation, CheckpointFileLocation)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_ips, IPAddress)
    END_SERIAL_MEMEBER
};

struct HyperVVMList {
    std::list<HyperVVMInfo> m_vmList;
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_vmList, result)
    END_SERIAL_MEMEBER
};

struct HyperVVMExtendInfo {
    std::string m_state;
    std::string m_status;
    std::string m_version;
    std::string m_generation;
    std::string m_configurationLocation;
    std::string m_checkpointFileLocation;
    std::string m_ipList;
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_state, State)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_status, status)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_version, Version)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_generation, Generation)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_configurationLocation, ConfigurationLocation)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_checkpointFileLocation, CheckpointFileLocation)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_ipList, ipList)
    END_SERIAL_MEMEBER
};

/* {
    "DiskIdentifier":  "2C7278D1-4058-4620-9A18-FFE57F90AE41",
    "Size":  136365211648,
    "VhdFormat":  3,
    "VhdType":  4,
    "Path":  "C:\\Users\\Public\\Documents\\Hyper-V\\
              Virtual Hard Disks\\CentOS7_04455719-5F4C-4ED1-AA4B-631F44AB561C.avhdx",
    "ParentPath":  "C:\\Users\\Public\\Documents\\Hyper-V\\Virtual Hard Disks\\CentOS7.vhdx"
} */

struct HyperVDirectoryInfo {
    std::string m_name;
    uint64_t m_size;
    std::string m_lastWriteTime;
    std::string m_type;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_name, Name)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_size, Length)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_lastWriteTime, LastWriteTime)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_type, Mode)
    END_SERIAL_MEMEBER
};

struct HyperVDirectoryList {
    std::list<HyperVDirectoryInfo> m_directoryList;
    uint64_t m_total;
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_directoryList, result)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_total, total)
    END_SERIAL_MEMEBER
};

struct InvalidScanInfo {
    std::string m_errorCode;
    std::string m_errorMsg;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_errorCode, errorCode)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_errorMsg, errorMsg)
    END_SERIAL_MEMEBER
};

struct ActionResultResAcc {
    int32_t m_code = 0;
    int64_t m_bodyErr = 0;
    std::string m_message;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_code, code)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_bodyErr, bodyErr)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_message, message)
    END_SERIAL_MEMEBER
};

extern std::map<std::string, bool> g_listTaskMap;
extern std::mutex g_listLock;
extern std::condition_variable g_listCnv;

class HyperVResourceAccess {
public:
    HyperVResourceAccess(const ApplicationEnvironment &appEnv, const Application &application);
    HyperVResourceAccess(const ApplicationEnvironment &appEnv, const QueryByPage &pageInfo);
    ~HyperVResourceAccess();

    void SetApplication(const Application &application)
    {
        m_application = application;
    }

    void SetRequestId(const std::string &requestId)
    {
        m_requestId = requestId;
    }
    void SetIsParentInfoExist(const bool &isParentInfoExist)
    {
        m_isParentInfoExist = true;
    }
    void SetParentInfo(const std::string &parentId, const std::string &parentName)
    {
        m_parentId = parentId;
        m_parentName = parentName;
    }

    int32_t CheckSCVMMConnection(ActionResult &returnValue);
    int32_t CheckHostConnection(ActionResult &returnValue);
    int32_t CheckClusterConnection(ActionResult &returnValue);
    int32_t GetClusterList(ResourceResultByPage &page);
    int32_t GetClusterHostList(ResourceResultByPage &page);
    int32_t GetHostList(ResourceResultByPage &page);
    int32_t GetVMList(ResourceResultByPage &page);
    int32_t GetDiskList(ResourceResultByPage &page);
    int32_t GetDirectoryList(ResourceResultByPage &page);
    int32_t GetVMIpAddress(ResourceResultByPage &page);
    std::string GetExtendValue(const std::string &key);

private:
    template <typename T>
    ActionResultResAcc Executor(const std::string &command, T &res = T(), const Json::Value &param = Json::Value());
    bool CheckResultValid(const Json::Value &result);
    int32_t GetIpAddress(const std::string &hostName, std::string &ipAddress);
    void GetParentInfo(ApplicationResource &appResource);
    void CheckTask();
    void EndTask();

private:
    ApplicationEnvironment m_appEnv;
    Application m_application;
    QueryByPage m_condition;
    std::string m_requestId;
    int32_t m_pageNo = 0;
    int32_t m_pageSize = 0;
    bool m_isParentInfoExist = false;
    std::string m_parentId;
    std::string m_parentName;
};
}
#endif // __HYPERV_RESOURCE_ACCESS_H__