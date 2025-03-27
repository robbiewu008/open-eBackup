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
#include <utility>
#include <boost/algorithm/string.hpp>
#include <config_reader/ConfigIniReader.h>
#include "json/json.h"
#include "log/Log.h"
#include "common/utils/Utils.h"
#include "protect_engines/nutanix/common/NutanixConstants.h"
#include "NutanixResourceManager.h"

using AppProtect::ResourceResultByPage;
using namespace VirtPlugin;

namespace NutanixPlugin {
namespace {
std::unordered_set<std::string> resourceTypeSet = {
    {"NutanixHost"},
    {"NutanixCluster"},
    {"NutanixVm"},
    {"NutanixDisk"},
    {"NutanixStorageContainer"},
    {"NutanixNetwork"}
};
const std::string MGM_NODE_REMARK = "Management Node VM";
const int32_t MAX_NUM_PER_PAGE = 500;
}

NutanixResourceManager::NutanixResourceManager(const ApplicationEnvironment appEnv, QueryByPage pageInfo,
    std::shared_ptr<NutanixClient> client, std::shared_ptr<CertManger> certManger)
    : m_appEnv(appEnv), m_condition(pageInfo), m_nutanixClient(std::move(client)), m_certMgr(std::move(certManger))
{
    m_resourceFuncMap["NutanixCluster"] = std::bind(&NutanixResourceManager::GetClusterList, this,
        std::placeholders::_1);
    m_resourceFuncMap["NutanixHost"] = std::bind(&NutanixResourceManager::GetHostList, this,
        std::placeholders::_1);
    m_resourceFuncMap["NutanixVm"] = std::bind(&NutanixResourceManager::GetVMList, this,
        std::placeholders::_1);
    m_resourceFuncMap["NutanixDisk"] = std::bind(&NutanixResourceManager::GetVMDiskList, this,
        std::placeholders::_1);
    m_resourceFuncMap["NutanixStorageContainer"] = std::bind(&NutanixResourceManager::GetStorageContainerList, this,
        std::placeholders::_1);
    m_resourceFuncMap["NutanixNetwork"] = std::bind(&NutanixResourceManager::GetNetworkList, this,
        std::placeholders::_1);
}

void NutanixResourceManager::SetCommonInfo(NutanixRequest& req)
{
    if (m_certMgr == nullptr) {
        ERRLOG("SetCommonInfo m_certMgr nullptr! ");
        return;
    }
    AuthObj authObj;
    authObj.name = m_appEnv.auth.authkey;
    authObj.passwd = m_appEnv.auth.authPwd;
    authObj.certVerifyEnable = m_certMgr->IsVerifyCert();
    authObj.cert = m_certMgr->GetCertPath();
    authObj.revocationList = m_certMgr->GetRevocationListPath();
    req.SetEndpoint(m_appEnv.endpoint);
    req.SetEnvAddress(m_appEnv.endpoint);
    req.SetIpPort(std::to_string(m_appEnv.port));
    req.SetUserInfo(authObj);
    return;
}

int32_t NutanixResourceManager::GetTargetResource(ResourceResultByPage& page)
{
    if (m_nutanixClient == nullptr) {
        ERRLOG("m_nutanixClient nullptr! Ip: %s", m_appEnv.endpoint.c_str());
        return FAILED;
    }
    NutanixType parentType = typeMap.at(m_appEnv.subType);
    std::string subType;
    if (!GetResourceType(subType)) {
        ERRLOG("Get Resource type error! Type: %s", subType.c_str());
        return FAILED;
    }

    int offset = m_condition.pageNo * m_condition.pageSize;
    if (offset < 0 || m_condition.pageSize < 0) {
        ERRLOG("page param error! pageNo: %d, pageSize: %d", m_condition.pageNo, m_condition.pageSize);
        return FAILED;
    }
    return m_resourceFuncMap[subType](page);
}

int32_t NutanixResourceManager::GetClusterList(ResourceResultByPage& page)
{
    GetClusterListRequest req(m_condition.pageNo * m_condition.pageSize, m_condition.pageSize);
    SetCommonInfo(req);
    std::shared_ptr<NutanixResponse<struct ClusterListDataResponse>> resp =
        m_nutanixClient->ExecuteAPI<NutanixResponse<struct ClusterListDataResponse>, GetClusterListRequest>(req);
    if (resp == nullptr) {
        ERRLOG("GetResource failed! Ip: %s", req.GetEnvAddress().c_str());
        return FAILED;
    }
    ClusterListDataResponse clusterList = resp->GetResult();

    std::vector<ApplicationResource> resourceResults;

    if (clusterList.entities.size() == 0) {
        ERRLOG("No cluster founded!");
    }

    for (const auto& cluster : clusterList.entities) {
        ApplicationResource result;
        result.__set_type("Nutanix");
        result.__set_subType("NutanixCluster");
        result.__set_name(cluster.name);
        result.__set_id(cluster.uuid);
        Json::Value clusterDetails;
        clusterDetails["version"] = cluster.version;
        Json::FastWriter jsonWriter;
        std::string detail = jsonWriter.write(clusterDetails);
        Utils::RemoveSpecialSymbols(detail);
        Json::Value clusterExtendInfo;
        clusterExtendInfo["details"] = detail;
        result.__set_extendInfo(jsonWriter.write(clusterExtendInfo));
        result.__set_parentId(m_appEnv.id);
        result.__set_parentName(m_appEnv.name);
        resourceResults.emplace_back(result);
    }
    page.__set_items(resourceResults);
    page.__set_pageNo(clusterList.metadata.count == 0 ? 0 :
        clusterList.metadata.startIndex / clusterList.metadata.count);
    page.__set_total(clusterList.metadata.totalEntities);
    page.__set_pageSize(clusterList.metadata.count);
    return SUCCESS;
}

int32_t NutanixResourceManager::GetHostList(ResourceResultByPage& page)
{
    NutanixType parentType = typeMap.at(m_appEnv.subType);

    GetHostListRequest req(m_condition.pageNo * m_condition.pageSize, m_condition.pageSize);
    SetCommonInfo(req);
    std::shared_ptr<NutanixResponse<struct HostListDataResponse>> resp =
        m_nutanixClient->ExecuteAPI<NutanixResponse<struct HostListDataResponse>, GetHostListRequest>(req);
    if (resp == nullptr) {
        ERRLOG("GetResource failed! Ip: %s", req.GetEnvAddress().c_str());
        return FAILED;
    }
    const HostListDataResponse &hostList = resp->GetResult();

    std::vector<ApplicationResource> resourceResults;

    if (hostList.entities.size() == 0) {
        WARNLOG("No host founded!");
    }

    for (const auto& host : hostList.entities) {
        ApplicationResource result;
        result.__set_type("Nutanix");
        result.__set_subType("NutanixHost");
        result.__set_name(host.name);
        result.__set_id(host.uuid);
        Json::Value hostDetails;
        hostDetails["state"] = host.state;
        hostDetails["cpuModel"] = host.cpuModel;
        hostDetails["numVms"] = host.numVms;
        Json::FastWriter jsonWriter;
        std::string detail = jsonWriter.write(hostDetails);
        Utils::RemoveSpecialSymbols(detail);
        Json::Value hostExtendInfo;
        hostExtendInfo["status"] = host.state;
        hostExtendInfo["details"] = detail;
        result.__set_extendInfo(jsonWriter.write(hostExtendInfo));
        result.__set_parentId(host.clusterUuid);
        resourceResults.emplace_back(result);
    }
    page.__set_items(resourceResults);
    page.__set_pageNo(hostList.metadata.count == 0 ? 0 : hostList.metadata.startIndex / hostList.metadata.count);
    page.__set_total(hostList.metadata.totalEntities);
    page.__set_pageSize(hostList.metadata.count);
    return SUCCESS;
}

void NutanixResourceManager::SetVMInfo(ApplicationResource &result, const VMListResponse &vm)
{
    result.__set_type("Nutanix");
    result.__set_parentId(vm.hostUuid);
    result.__set_subType("NutanixVm");
    result.__set_name(vm.name);
    result.__set_id(vm.uuid);
    Json::Value vmDetails;
    vmDetails["power_state"] = vm.powerState;
    vmDetails["guest_os"] = vm.guestOs;
    vmDetails["description"] = vm.description;
    vmDetails["host_uuid"] = vm.hostUuid;
    std::string ips;
    for (const auto& nic : vm.vmNics) {
        ips = ips + nic.ipAddress + ",";
    }
    if (!ips.empty()) {
        ips.pop_back();
    }
    vmDetails["ip"] = ips;
    Json::FastWriter jsonWriter;
    std::string detail = jsonWriter.write(vmDetails);
    Utils::RemoveSpecialSymbols(detail);
    std::string protectable = "true";
    for (const auto& diskInfo : vm.vmDiskInfo) {
        if (!diskInfo.diskAddress.volumeGroupUuid.empty()) {
            protectable = "false";
            break;
        }
    }
    Json::Value vmExtendInfo;
    vmExtendInfo["details"] = detail;
    vmExtendInfo["status"] = vm.powerState;
    vmExtendInfo["description"] = vm.description;
    vmExtendInfo["protectable"] = protectable;
    result.__set_extendInfo(jsonWriter.write(vmExtendInfo));
}

int32_t NutanixResourceManager::GetVMList(ResourceResultByPage &page)
{
    NutanixType parentType = typeMap.at(m_appEnv.subType);
    GetVMListRequest req(m_condition.pageNo * m_condition.pageSize, m_condition.pageSize);
    SetCommonInfo(req);
    std::shared_ptr<NutanixResponse<struct VMListDataResponse>> resp =
        m_nutanixClient->ExecuteAPI<NutanixResponse<struct VMListDataResponse>, GetVMListRequest>(req);
    if (resp == nullptr) {
        ERRLOG("GetResource failed! Ip: %s", req.GetEnvAddress().c_str());
        return FAILED;
    }
    const VMListDataResponse &vmList = resp->GetResult();
    std::vector<ApplicationResource> resourceResults;

    if (vmList.entities.size() == 0) {
        WARNLOG("No vm founded!");
    }

    for (const auto& vm : vmList.entities) {
        ApplicationResource result;
        SetVMInfo(result, vm);
        std::string parentId = vm.hostUuid == "" ? m_appEnv.id : vm.hostUuid;
        result.__set_parentId(parentId);
        resourceResults.emplace_back(result);
    }
    page.__set_items(resourceResults);
    page.__set_pageNo(vmList.metadata.count == 0 ? 0 : vmList.metadata.startIndex / vmList.metadata.count);
    page.__set_total(vmList.metadata.totalEntities);
    page.__set_pageSize(vmList.metadata.count);
    return SUCCESS;
}

void NutanixResourceManager::SetDiskAddress(Json::Value &diskDetails, const DiskAddress &diskAddress)
{
    Json::Value address;
    address["device_bus"] = diskAddress.deviceBus;
    address["device_index"] = diskAddress.deviceIndex;
    address["device_uuid"] = diskAddress.deviceUuid;
    address["disk_label"] = diskAddress.diskLabel;
    address["ndfs_filepath"] = diskAddress.ndfsFilepath;
    address["vmdisk_uuid"] = diskAddress.vmdiskUuid;
    address["volume_group_uuid"] = diskAddress.volumeGroupUuid;
    diskDetails["disk_address"] = address;
}

void NutanixResourceManager::SetDiskInfo(Json::Value &diskDetails, const VmDiskInfo &diskInfo)
{
    diskDetails["storage_container_uuid"] = diskInfo.storageContainerUuid;
    diskDetails["datasource_uuid"] = diskInfo.datasourceUuid;
    diskDetails["shared"] = diskInfo.shared;
    diskDetails["size"] = diskInfo.size;
    diskDetails["is_thin_provisioned"] = diskInfo.isThinProvisioned;
    diskDetails["is_cdrom"] = diskInfo.isCdrom;
    diskDetails["is_scsi_passthrough"] = diskInfo.isScsiPassthrough;
    diskDetails["is_hot_remove_enabled"] = diskInfo.isHotRemoveEnabled;
    diskDetails["is_empty"] = diskInfo.isEmpty;
    diskDetails["flash_mode_enabled"] = diskInfo.flashModeEnabled;
    SetDiskAddress(diskDetails, diskInfo.diskAddress);
}

void NutanixResourceManager::GetContainerNameFromFilePath(const std::string &filePath, std::string &name)
{
    size_t pos1 = filePath.find("/");
    size_t pos2 = filePath.find("/", pos1 + 1);
    if (pos1 == std::string::npos || pos2 == std::string::npos || pos2 - pos1 <= 1) {
        ERRLOG("Invalid file path: %s", filePath.c_str());
        return;
    }
    name = filePath;
    name = name.substr(pos1 + 1, pos2 - pos1 - 1);
    return;
}

int32_t NutanixResourceManager::GetVMDiskList(ResourceResultByPage& page)
{
    GetVMInfoRequest req(m_uuId);
    SetCommonInfo(req);
    if (m_nutanixClient == nullptr) {
        ERRLOG("Get VM info m_nutanixClient nullptr");
        return false;
    }
    std::shared_ptr<NutanixResponse<struct NutanixVMInfo>> resp =
        m_nutanixClient->ExecuteAPI<NutanixResponse<struct NutanixVMInfo>, GetVMInfoRequest>(req);
    if (resp == nullptr) {
        ERRLOG("Get VM info failed");
        return false;
    }
    NutanixVMInfo vmInfo = resp->GetResult();
    std::vector<ApplicationResource> resourceResults;

    for (const auto& diskInfo : vmInfo.vmDiskInfo) {
        if (diskInfo.isCdrom) {
            continue;
        }
        ApplicationResource result;
        result.__set_type("Nutanix");
        result.__set_parentId(m_uuId);
        result.__set_parentName(vmInfo.name);
        result.__set_subType("NutanixDisk");
        result.__set_name(diskInfo.diskAddress.diskLabel);
        result.__set_id(diskInfo.diskAddress.vmdiskUuid);
        Json::Value diskDetails;
        SetDiskInfo(diskDetails, diskInfo);
        Json::FastWriter jsonWriter;
        std::string detail = jsonWriter.write(diskDetails);
        Utils::RemoveSpecialSymbols(detail);
        Json::Value diskExtendInfo;
        std::string containerName;
        GetContainerNameFromFilePath(diskInfo.diskAddress.ndfsFilepath, containerName);
        diskExtendInfo["details"] = detail;
        diskExtendInfo["storage_name"] = containerName;
        diskExtendInfo["bus"] = diskInfo.diskAddress.deviceBus;
        diskExtendInfo["is_volume_group"] = diskInfo.diskAddress.volumeGroupUuid.empty() ? "false" : "true";
        result.__set_extendInfo(jsonWriter.write(diskExtendInfo));
        resourceResults.emplace_back(result);
    }
    page.__set_total(vmInfo.vmDiskInfo.size());
    page.__set_items(resourceResults);
    return SUCCESS;
}

void NutanixResourceManager::SetStorageContainerInfo(ApplicationResource &result, const ContainerInfo &container)
{
    Json::Value detail;
    Json::Value extendInfo;
    detail["capacity"] = container.maxCapacity;
    detail["available"] = container.usageStats.userFreeBytes;
    detail["storagePoolId"] = container.storagePoolId;
    Json::FastWriter jsonWriter;
    std::string detailStr = jsonWriter.write(detail);
    Utils::RemoveSpecialSymbols(detailStr);
    extendInfo["details"] = detailStr;
    result.__set_extendInfo(jsonWriter.write(extendInfo));
}

int32_t NutanixResourceManager::GetStorageContainerList(ResourceResultByPage &page)
{
    GetContainerRequest req(m_condition.pageNo * m_condition.pageSize, m_condition.pageSize);
    SetCommonInfo(req);
    std::shared_ptr<GetContainerResponse> resp =
        m_nutanixClient->ExecuteAPI<GetContainerResponse, GetContainerRequest>(req);
    if (resp == nullptr) {
        ERRLOG("GetResource failed! Ip: %s", req.GetEnvAddress().c_str());
        return FAILED;
    }
    ContainerListResponse containerList = resp->GetList();
    std::vector<ApplicationResource> resourceResults;

    if (containerList.entities.size() == 0) {
        WARNLOG("No container founded!");
    }

    for (const auto& container : containerList.entities) {
        if (container.isNutanixManaged) { // 过滤掉NutanixManagementShare
            continue;
        }
        ApplicationResource result;
        SetStorageContainerInfo(result, container);
        result.__set_type("Nutanix");
        result.__set_subType("NutanixStorageContainer");
        result.__set_name(container.name);
        result.__set_id(container.containerUuid);
        result.__set_parentId(m_appEnv.id);
        resourceResults.emplace_back(result);
    }
    page.__set_items(resourceResults);
    page.__set_pageNo(containerList.metadata.count == 0 ? 0 : containerList.metadata.startIndex
        / containerList.metadata.count);
    page.__set_total(containerList.metadata.totalEntities);
    page.__set_pageSize(containerList.metadata.count);
    return SUCCESS;
}

int32_t NutanixResourceManager::GetNetworkList(ResourceResultByPage& page)
{
    GetNetworkRequest req(m_condition.pageNo * m_condition.pageSize, m_condition.pageSize);
    SetCommonInfo(req);
    std::shared_ptr<GetNetworkResponse> resp =
        m_nutanixClient->ExecuteAPI<GetNetworkResponse, GetNetworkRequest>(req);
    if (resp == nullptr) {
        ERRLOG("GetResource failed! Ip: %s", req.GetEnvAddress().c_str());
        return FAILED;
    }
    NetworkListResponse networkList = resp->GetList();
    std::vector<ApplicationResource> resourceResults;

    if (networkList.entities.size() == 0) {
        WARNLOG("No network founded!");
    }

    for (const auto& network : networkList.entities) {
        ApplicationResource result;
        result.__set_type("Nutanix");
        result.__set_subType("NutanixNetwork");
        result.__set_name(network.name);
        result.__set_id(network.uuid);
        std::string parentId = m_appEnv.id;
        result.__set_parentId(parentId);
        resourceResults.emplace_back(result);
    }
    page.__set_items(resourceResults);
    page.__set_total(networkList.metadata.totalEntities);
    return SUCCESS;
}

bool NutanixResourceManager::GetResourceType(std::string &subType)
{
    Json::Value conditionJson;
    if (!Module::JsonHelper::JsonStringToJsonValue(m_condition.conditions, conditionJson)) {
        ERRLOG("JsonStringToJsonValue error, str: %s", m_condition.conditions.c_str());
        return false;
    }
    if (conditionJson.isMember("resourceType") &&
        resourceTypeSet.find(conditionJson["resourceType"].asString()) != resourceTypeSet.end()) {
        subType = conditionJson["resourceType"].asString();
        if (conditionJson.isMember("uuid")) {
            m_uuId = conditionJson["uuid"].asString();
        }
    } else {
        ERRLOG("Resource type not found! Type: %s", subType.c_str());
        return false;
    }
    return true;
}
}