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
#include "protect_engines/cnware/common/CNwareConstants.h"
#include "CNwareResourceManager.h"

using AppProtect::ResourceResultByPage;
using namespace VirtPlugin;

namespace CNwarePlugin {
namespace {
std::map<std::string, std::string> urlMap = {
    {"CNwareHostPool", "pools"},
    {"CNwareHost",     "hosts"},
    {"CNwareCluster",  "clusters"},
    {"CNwareVm",       "domains"},
    {"CNwareDisk",     "domainDiskInfo"},
    {"StoragePool",     ""},
    {"PortGroup",     ""}
};
std::map<int, std::string> osTypeMap = {
    {0, "other"},
    {1, "linux"},
    {2, "windows"}
};
const std::string MGM_NODE_REMARK = "Management Node VM";
const int32_t MAX_NUM_PER_PAGE = 500;
}

CNwareResourceManager::CNwareResourceManager(const ApplicationEnvironment appEnv, QueryByPage pageInfo,
    std::shared_ptr<CNwareClient> m_Client)
    : m_appEnv(appEnv), m_condition(pageInfo), m_cnwareClient(std::move(m_Client))
{
    m_resourceFuncMap["CNware"] = std::bind(&CNwareResourceManager::SetHostPool, this,
        std::placeholders::_1, std::placeholders::_2);
    m_resourceFuncMap["CNwareHostPool"] = std::bind(&CNwareResourceManager::SetHostPool, this,
        std::placeholders::_1, std::placeholders::_2);
    m_resourceFuncMap["CNwareCluster"] = std::bind(&CNwareResourceManager::SetCluster, this,
        std::placeholders::_1, std::placeholders::_2);
    m_resourceFuncMap["CNwareHost"] = std::bind(&CNwareResourceManager::SetHost, this,
        std::placeholders::_1, std::placeholders::_2);
    m_resourceFuncMap["CNwareVm"] = std::bind(&CNwareResourceManager::SetVm, this,
        std::placeholders::_1, std::placeholders::_2);
    m_resourceFuncMap["CNwareDisk"] = std::bind(&CNwareResourceManager::SetDisk, this,
        std::placeholders::_1, std::placeholders::_2);
}

CNwareResourceManager::~CNwareResourceManager()
= default;

int32_t CNwareResourceManager::GetTargetResource(ResourceResultByPage& page, CNwareRequest &req)
{
    if (m_cnwareClient == nullptr) {
        ERRLOG("m_cnwareClient nullptr! Ip: %s", req.GetEnvAddress().c_str());
        return FAILED;
    }
    std::shared_ptr<ResponseModel> response = std::make_shared<ResponseModel>();
    CNwareType parentType = typeMap.at(m_appEnv.subType);
    std::string subType;
    if (!GetResourceType(subType)) {
        ERRLOG("Get Resource type error! Type: %s", subType.c_str());
        return FAILED;
    }
    if (subType == "StoragePool") {
        return GetStoragePool(page, req);
    } else if (subType == "PortGroup") {
        return GetPortGroup(page, req);
    }
    if (SetResourceUrl(req, subType) != SUCCESS) {
        ERRLOG("SetResourceUrl failed! Ip: %s", req.GetEnvAddress().c_str());
        return FAILED;
    }
    if (m_cnwareClient->GetResource(req, response, m_condition, parentType) != SUCCESS) {
        ERRLOG("GetResource failed! Ip: %s", req.GetEnvAddress().c_str());
        return FAILED;
    }
    int32_t iRet = ParseResponse(page, response, subType);
    if (iRet != SUCCESS) {
        ERRLOG("Set Response into ResourceResultByPage failed! Ip: %s", req.GetEnvAddress().c_str());
        return FAILED;
    }
    INFOLOG("Set Response into ResourceResultByPage succeed! Resource type: %s", m_appEnv.type.c_str());
    return SUCCESS;
}

void CNwareResourceManager::SetStorageDetails(const StoragePool &storage, ApplicationResource &result)
{
    Json::Value storageExtendInfo;
    Json::Value storageDetails;
    Json::FastWriter jsonWriter;
    storageDetails["capacity"] = storage.m_capacity;
    storageDetails["available"] = storage.m_available;
    storageDetails["storageSanId"] = storage.m_storageSanId;
    storageDetails["storageSanName"] = storage.m_storageSanName;
    storageDetails["storageType"] = storage.m_storageType;
    storageDetails["storageResourceName"] = storage.m_storageResourceName;
    storageDetails["type"] = storage.m_type;
    storageDetails["useType"] = storage.m_useType;
    storageDetails["status"] = storage.m_status;
    std::string detail = jsonWriter.write(storageDetails);
    Utils::RemoveSpecialSymbols(detail);
    storageExtendInfo["details"] = detail;
    storageExtendInfo["status"] = storage.m_status;
    result.__set_extendInfo(jsonWriter.write(storageExtendInfo));
}

int32_t CNwareResourceManager::GetStoragePool(ResourceResultByPage &page, CNwareRequest &req)
{
    if (m_cnwareClient == nullptr) {
        ERRLOG("GetStoragePool m_cnwareClient nullptr!");
        return FAILED;
    }
    StoragePoolInfo storageInfo;
    int32_t start = 0;
    std::vector<ApplicationResource> resourceResults;
    int32_t pageNum = Module::ConfigReader::getInt("CNwareConfig", "RequestPageNums");
    if (pageNum <= 0) {
        ERRLOG("GetStoragePool pageNum error!");
        return FAILED;
    }
    do {
        start++;
        std::shared_ptr<StoragePoolResponse> response = m_cnwareClient->GetStoragePoolInfo(
            req, m_vmId, "", start, pageNum);
        if (response == nullptr) {
            ERRLOG("GetStoragePoolInfo failed.");
            return FAILED;
        }
        storageInfo = response->GetStoragePoolInfo();
        page.__set_pageNo(storageInfo.m_start);
        page.__set_total(storageInfo.m_total);
        page.__set_pageSize(storageInfo.m_size);
        for (StoragePool &storage : storageInfo.m_data) {
            ApplicationResource result;
            result.__set_type("CNware");
            result.__set_id(storage.m_id);
            result.__set_name(storage.m_name);
            result.__set_parentId(m_vmId);
            result.__set_parentName(m_vmId);
            SetStorageDetails(storage, result);
            resourceResults.emplace_back(result);
        }
    } while (storageInfo.m_total > (start * pageNum));
    page.__set_items(resourceResults);
    return SUCCESS;
}

int32_t CNwareResourceManager::GetPortGroup(ResourceResultByPage &page, CNwareRequest &req)
{
    if (m_cnwareClient == nullptr) {
        ERRLOG("Get PortGroup m_cnwareClient nullptr!");
        return FAILED;
    }
    std::shared_ptr<PortGroupResponse> response = m_cnwareClient->GetPortGroupInfo(req, m_vmId);
    if (response == nullptr) {
        ERRLOG("Get PortGroup Info failed.");
        return FAILED;
    }
    std::vector<PortGroup> portGroupInfo = response->GetPortGroupInfo();
    page.__set_pageNo(0);
    page.__set_total(portGroupInfo.size());
    page.__set_pageSize(0);
    std::vector<ApplicationResource> resourceResults;
    Json::FastWriter jsonWriter;
    for (PortGroup &portGroup : portGroupInfo) {
        ApplicationResource result;
        result.__set_type("CNware");
        result.__set_id(portGroup.m_id);
        result.__set_name(portGroup.m_name);
        result.__set_parentId(portGroup.m_dvswitchId);
        result.__set_parentName(portGroup.m_dvswitchName);
        Json::Value portGroupExtendInfo;
        Json::Value portGroupDetails;
        portGroupDetails["vlanId"] = portGroup.m_vlanId;
        portGroupDetails["dvswitchId"] = portGroup.m_dvswitchId;
        portGroupDetails["dvswitchName"] = portGroup.m_dvswitchName;
        portGroupDetails["networkStrategy"] = portGroup.m_networkStrategy;
        portGroupDetails["portNum"] = portGroup.m_portNum;
        portGroupDetails["distributedFlag"] = portGroup.m_distributedFlag;
        std::string detail = jsonWriter.write(portGroupDetails);
        Utils::RemoveSpecialSymbols(detail);
        portGroupExtendInfo["details"] = detail;
        result.__set_extendInfo(jsonWriter.write(portGroupExtendInfo));
        resourceResults.emplace_back(result);
    }
    page.__set_items(resourceResults);
    return SUCCESS;
}

bool CNwareResourceManager::GetResourceType(std::string &subType)
{
    Json::Value conditionJson;
    if (!Module::JsonHelper::JsonStringToJsonValue(m_condition.conditions, conditionJson)) {
        ERRLOG("JsonStringToJsonValue error, str: %s", m_condition.conditions.c_str());
        return false;
    }
    if (conditionJson.isMember("resourceType") &&
        urlMap.find(conditionJson["resourceType"].asString()) != urlMap.end()) {
        subType = conditionJson["resourceType"].asString();
        if (conditionJson.isMember("uuid")) {
            m_vmId = conditionJson["uuid"].asString();
        }
    } else {
        ERRLOG("Resource type not found! Type: %s", subType.c_str());
        return false;
    }
    return true;
}


int32_t CNwareResourceManager::SetResourceUrl(CNwareRequest &req, const std::string &subType)
{
    if (urlMap.find(subType) == urlMap.end()) {
        ERRLOG("Type not find! Str: %s", subType.c_str());
        return FAILED;
    }
    req.SetDomain("");
    if (subType == "CNwareDisk") {
        req.url = req.url + "api/compute/domains/" + m_vmId + "/" + urlMap[subType];
    } else {
        req.url = req.url + "api/compute/" + urlMap[subType];
    }
    if (subType == "CNwareVm") {
        Json::Value extendJson;
        if (!Module::JsonHelper::JsonStringToJsonValue(m_appEnv.extendInfo, extendJson)) {
            ERRLOG("SetResourceUrl JsonStringToJsonValue error, str: %s", m_appEnv.extendInfo.c_str());
            return false;
        }
        std::string domain = extendJson.isMember("nameLike") ? extendJson["nameLike"].asString() : "";
        req.SetDomain(domain);
    }
    INFOLOG("Get Type url: %s", req.url.c_str());
    return SUCCESS;
}

int32_t CNwareResourceManager::ParseResponse(ResourceResultByPage &page, std::shared_ptr<ResponseModel> response,
    const std::string &subType)
{
    Json::Value jsonValue;
    std::vector<ApplicationResource> resourceResults;
    if (!Module::JsonHelper::JsonStringToJsonValue(response->GetBody(), jsonValue)) {
        ERRLOG("JsonStringToJsonValue response Body error.");
        return FAILED;
    }
    Json::Value conditionJson;
    if (!Module::JsonHelper::JsonStringToJsonValue(m_condition.conditions, conditionJson)) {
        ERRLOG("JsonStringToJsonValue error, str: %s", m_condition.conditions.c_str());
        return FAILED;
    }
    ParseJsonValue(jsonValue, conditionJson, resourceResults, subType);
    page.__set_items(resourceResults);
    if (jsonValue.isMember("start") && jsonValue.isMember("total") && jsonValue.isMember("size")) {
        page.__set_pageNo(jsonValue["start"].asInt());
        page.__set_total(jsonValue["total"].asInt());
        page.__set_pageSize(jsonValue["size"].asInt());
    }
    return SUCCESS;
}

void CNwareResourceManager::ParseJsonValue(const Json::Value &jsonValue, const Json::Value &conditionJson,
    std::vector<ApplicationResource> &resourceResults, const std::string &subType)
{
    if (jsonValue.isMember("data") && jsonValue["data"].isArray()) {
        for (auto &items : jsonValue["data"]) {
            if (!items.isObject()) {
                ERRLOG("Get data item error, item str: %s", WIPE_SENSITIVE(items.toStyledString()).c_str());
            }
            if (conditionJson.isMember("isTree") && (conditionJson["isTree"].asString() == "1") && items.isMember(
                "clusterId") && (!items["clusterId"].asString().empty())) {
                continue;
            }
            ApplicationResource result;
            if (SetResourceResult(result, items, subType) != SUCCESS) {
                ERRLOG("Set resource result error, item str: %s",
                    WIPE_SENSITIVE(items.toStyledString()).c_str());
            } else {
                resourceResults.emplace_back(result);
            }
        }
    } else if (jsonValue.isMember("diskDevices") && jsonValue["diskDevices"].isArray()) {
        for (auto &items : jsonValue["diskDevices"]) {
            if (!items.isObject()) {
                ERRLOG("Get data item error, item str: %s",
                    WIPE_SENSITIVE(items.toStyledString()).c_str());
            }
            ApplicationResource result;
            if (SetResourceResult(result, items, subType) != SUCCESS) {
                ERRLOG("Set resource result error, item str: %s",
                    WIPE_SENSITIVE(items.toStyledString()).c_str());
            } else {
                resourceResults.emplace_back(result);
            }
        }
    }
}

int32_t CNwareResourceManager::SetResourceResult(ApplicationResource &result, const Json::Value &items,
    const std::string &subType)
{
    result.__set_type("CNware");
    if (m_resourceFuncMap.find(subType) == m_resourceFuncMap.end()) {
        ERRLOG("Subtype not find! Str: %s", subType.c_str());
        return FAILED;
    }
    auto func = m_resourceFuncMap[subType];
    if (func(result, items) != SUCCESS) {
        ERRLOG("Set resource failed! Items: %s", WIPE_SENSITIVE(items.toStyledString()).c_str());
        return FAILED;
    }
    return SUCCESS;
}

int32_t CNwareResourceManager::SetHostPool(ApplicationResource &result, const Json::Value &items)
{
    result.__set_subType("CNwareHostPool");
    INFOLOG("Set resource Items: %s, %s", items["name"].asString().c_str(), items["id"].asString().c_str());
    if (items.isMember("name") && items.isMember("id") && items.isMember("remark")) {
        INFOLOG("Set resource Items: %s, %s", items["name"].asString().c_str(), items["id"].asString().c_str());
        result.__set_name(items["name"].asString());
        result.__set_id(items["id"].asString());
        result.__set_parentId(m_appEnv.id);
        result.__set_parentName(m_appEnv.name);
        Json::Value hostExtendInfo;
        Json::Value hostDetails;
        hostDetails["remark"] = items["remark"].asString();
        Json::FastWriter jsonWriter;
        std::string detail = jsonWriter.write(hostDetails);
        Utils::RemoveSpecialSymbols(detail);
        hostExtendInfo["details"] = detail;
        hostExtendInfo["remark"] = items["remark"].asString();
        result.__set_extendInfo(jsonWriter.write(hostExtendInfo));
        return SUCCESS;
    }
    return FAILED;
}

int32_t CNwareResourceManager::SetHost(ApplicationResource &result, const Json::Value &items)
{
    result.__set_subType("CNwareHost");
    if (items.isMember("name") && items.isMember("id")) {
        result.__set_name(items["name"].asString());
        result.__set_id(items["id"].asString());
    }
    if (items.isMember("isMaintain") && items.isMember("isConnected") && items.isMember("cpuArchitecture")
        && items.isMember("remark") && items.isMember("ip")) {
        Json::Value hostExtendInfo;
        Json::Value hostDetails;
        hostDetails["isMaintain"] = items["isMaintain"].asBool() ? "1" : "0";
        hostDetails["status"] = items["isConnected"].asBool() ? "1" : "0";
        hostDetails["cpuArchitecture"] = items["cpuArchitecture"];
        hostDetails["remark"] = items["remark"].asString();
        hostDetails["ip"] = items["ip"].asString();
        Json::FastWriter jsonWriter;
        std::string detail = jsonWriter.write(hostDetails);
        Utils::RemoveSpecialSymbols(detail);
        hostExtendInfo["details"] = detail;
        hostExtendInfo["remark"] = items["remark"].asString();
        hostExtendInfo["status"] = items["isConnected"].asBool() ? "1" : "0";
        result.__set_extendInfo(jsonWriter.write(hostExtendInfo));
    }
    if (items.isMember("clusterName") && items.isMember("clusterId") && !items["clusterId"].empty()) {
        result.__set_parentId(items["clusterId"].asString());
        result.__set_parentName(items["clusterName"].asString());
        return SUCCESS;
    }
    if (items.isMember("poolName") && items.isMember("poolId")) {
        result.__set_parentId(items["poolId"].asString());
        result.__set_parentName(items["poolName"].asString());
        return SUCCESS;
    }
    return FAILED;
}

int32_t CNwareResourceManager::SetCluster(ApplicationResource &result, const Json::Value &items)
{
    result.__set_subType("CNwareCluster");
    if (items.isMember("name") && items.isMember("id")) {
        result.__set_name(items["name"].asString());
        result.__set_id(items["id"].asString());
    }
    if (items.isMember("poolName") && items.isMember("poolId") && items.isMember("remark")) {
        result.__set_parentId(items["poolId"].asString());
        result.__set_parentName(items["poolName"].asString());
        Json::Value hostExtendInfo;
        Json::Value hostDetails;
        hostDetails["remark"] = items["remark"].asString();
        Json::FastWriter jsonWriter;
        std::string detail = jsonWriter.write(hostDetails);
        Utils::RemoveSpecialSymbols(detail);
        hostExtendInfo["details"] = detail;
        hostExtendInfo["remark"] = items["remark"].asString();
        result.__set_extendInfo(jsonWriter.write(hostExtendInfo));
        return SUCCESS;
    }
    return FAILED;
}

int32_t CNwareResourceManager::SetVm(ApplicationResource &result, const Json::Value &items)
{
    result.__set_subType("CNwareVm");
    if (items.isMember("name") && items.isMember("id")) {
        result.__set_name(items["name"].asString());
        result.__set_id(items["id"].asString());
    }
    if (items.isMember("status") && items.isMember("remark") && items.isMember("osType")
        && items.isMember("osVersion") && items.isMember("bridgeInterfaces")) {
        Json::Value hostExtendInfo;
        Json::Value hostDetails;
        if (items["remark"].asString() == MGM_NODE_REMARK) {
            WARNLOG("Find MGM node %s, skip!", items["name"].asString().c_str());
            return FAILED;
        }
        hostDetails["status"] = items["status"].asString();
        hostDetails["remark"] = items["remark"].asString();
        hostDetails["osVersion"] = items["osVersion"].asString();
        hostDetails["osType"] = items["osType"].asInt();
        std::string ip = "";
        for (const auto &inter : items["bridgeInterfaces"]) {
            if (inter.isMember("ip") && !inter["ip"].asString().empty()) {
                ip = ip + inter["ip"].asString() + ",";
            }
        }
        if (!ip.empty()) {
            ip.pop_back();
        }
        hostDetails["ip"] = ip;
        Json::FastWriter jsonWriter;
        std::string detail = jsonWriter.write(hostDetails);
        Utils::RemoveSpecialSymbols(detail);
        hostExtendInfo["os_type"] = "other";
        hostExtendInfo["remark"] = items["remark"].asString();
        hostExtendInfo["status"] = items["status"].asString();
        if (osTypeMap.count(items["osType"].asInt()) != 0) {
            hostExtendInfo["os_type"] = osTypeMap[items["osType"].asInt()];
        }
        hostExtendInfo["details"] = detail;
        result.__set_extendInfo(jsonWriter.write(hostExtendInfo));
    }
    if (items.isMember("hostName") && items.isMember("hostId")) {
        result.__set_parentId(items["hostId"].asString());
        result.__set_parentName(items["hostName"].asString());
        return SUCCESS;
    }
    return FAILED;
}

int32_t CNwareResourceManager::SetDisk(ApplicationResource &result, const Json::Value &items)
{
    result.__set_subType("CNwareDisk");
    result.__set_parentId(m_appEnv.id);
    result.__set_parentName(m_appEnv.name);
    if (items.isMember("volId") && items.isMember("dev")) {
        result.__set_name(items["dev"].asString());
        result.__set_id(items["volId"].asString());
    }
    if (!items["status"]) {
        ERRLOG("Disk status failed! Status: %s", items["status"].asString().c_str());
        return FAILED;
    }
    if (items.isMember("bootOrder") && items.isMember("capacity") && items.isMember("storagePoolId")
        && items.isMember("bus") && items.isMember("storagePoolName") && items.isMember("dev"),
        items.isMember("sourceFile") && items.isMember("preallocation")) {
        std::vector<std::string> sourceFileSpliteStrs;
        (void)boost::split(sourceFileSpliteStrs, items["sourceFile"].asString(),
            boost::is_any_of("/"));
        if (sourceFileSpliteStrs.back().empty()) {
            WARNLOG("Get empty sourceFileSpliteStrs sourcefile: %s",
                items["sourceFile"].asString());
            return FAILED;
        }
        Json::Value hostExtendInfo;
        Json::Value hostDetails;
        hostDetails["name"] = sourceFileSpliteStrs.back();
        hostDetails["bootOrder"] = items["bootOrder"].asString();
        hostDetails["status"] = items["status"].asBool() ? "1" : "0";
        hostDetails["size"] = items["capacity"];
        hostDetails["storageId"] = items["storagePoolId"].asString();
        hostDetails["storage"] = items["storagePoolName"].asString();
        hostDetails["bus"] = items["bus"].asString();
        hostDetails["dev"] = items["dev"].asString();
        hostDetails["preallocation"] = items["preallocation"].asString();
        hostDetails["sourceFile"] = items["sourceFile"].asString();
        Json::FastWriter jsonWriter;
        std::string detail = jsonWriter.write(hostDetails);
        Utils::RemoveSpecialSymbols(detail);
        hostExtendInfo["details"] = detail;
        hostExtendInfo["bus"] = items["bus"].asString();
        hostExtendInfo["storage"] = items["storagePoolName"].asString();
        hostExtendInfo["storageId"] = items["storagePoolId"].asString();
        hostExtendInfo["status"] = items["status"].asBool() ? "1" : "0";
        result.__set_extendInfo(jsonWriter.write(hostExtendInfo));
        return SUCCESS;
    }
    return FAILED;
}
}
