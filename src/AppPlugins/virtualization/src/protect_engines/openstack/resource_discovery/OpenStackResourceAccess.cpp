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
#include "OpenStackResourceAccess.h"
#include <common/JsonHelper.h>
#include <thread>
#include <algorithm>
#include "json/json.h"
#include "log/Log.h"
#include "protect_engines/openstack/api/nova/NovaClient.h"
#include "protect_engines/openstack/api/cinder/CinderClient.h"
#include "protect_engines/openstack/api/keystone/KeyStoneClient.h"
#include "protect_engines/openstack/api/neutron/NeutronClient.h"
#include "OpenStackMessage.h"
#include "config_reader/ConfigIniReader.h"
#include "common/utils/Utils.h"


OPENSTACK_PLUGIN_NAMESPACE_BEGIN

OpenStackResourceAccess::OpenStackResourceAccess(ApplicationEnvironment appEnv) : m_appEnv(appEnv)
{}

OpenStackResourceAccess::OpenStackResourceAccess(ApplicationEnvironment appEnv, QueryByPage pageInfo)
    : m_appEnv(appEnv), m_condition(pageInfo) {}

OpenStackResourceAccess::~OpenStackResourceAccess()
{}

void OpenStackResourceAccess::SetApplication(Application application)
{
    DBGLOG("%s", application.id.c_str());
    m_application = application;
    return;
}

void OpenStackResourceAccess::SetCertMgr(std::shared_ptr<CertManger> certMgr)
{
    m_certMgr = certMgr;
}

/**
 * @brief application扩展字段中存放domain name和domain id.
 *
 * @param request
 * @param user
 * @param pwd
 * @return int32_t
 */
void OpenStackResourceAccess::SetRequestCommonInfo(ModelBase &request, const std::string &user,
    const std::string &pwd)
{
    AuthObj authObj;
    authObj.name = user;
    authObj.passwd = pwd;
    authObj.certVerifyEnable = m_certMgr->IsVerifyCert();
    authObj.cert = m_certMgr->GetCertPath();
    authObj.revocationList = m_certMgr->GetRevocationListPath();
    request.SetUserInfo(authObj);
    request.SetEndpoint(m_appEnv.endpoint); // keystone address
    request.SetEnvAddress(m_appEnv.endpoint);
    return;
}

int32_t OpenStackResourceAccess::SetDomainInfo(ModelBase &request)
{
    Json::Value envExtendInfo;
    if (!Module::JsonHelper::JsonStringToJsonValue(m_application.extendInfo, envExtendInfo)) {
        ERRLOG("Failed to convert extend info.");
        return FAILED;
    }
    std::string domainName = envExtendInfo["domainName"].asString();
    DBGLOG("Domain name %s.", domainName.c_str());
    request.SetDomain(domainName);
    std::string domainId = envExtendInfo["domainId"].asString();
    DBGLOG("Domain id %s.", domainId.c_str());
    request.SetDomainId(domainId);
    if (domainName.empty() || domainId.empty()) {
        return FAILED;
    }
    return SUCCESS;
}

/**
 * @brief 1. 云平台连通性检查   2. Domain帐号正确性检查
 * 云平台连通性检查时application中auth填云平台帐号, 添加Domain帐号时填写Domain管理员帐号
 * @param errCode
 * @return int32_t
 */
int32_t OpenStackResourceAccess::VerifyDomainUser(int32_t& errCode)
{
    GetTokenRequest request;
    request.SetNeedRetry(false);
    request.SetScopeValue(m_application.id);  // domain id
    request.SetScopeType(Scope::NONE);
    SetRequestCommonInfo(request, m_application.auth.authkey, m_application.auth.authPwd);
    Json::Value appExtendInfo;
    if (!Module::JsonHelper::JsonStringToJsonValue(m_application.extendInfo, appExtendInfo)) {
        ERRLOG("Failed to convert extend info.");
        return FAILED;
    }
    request.SetDomain(appExtendInfo["domainName"].asString());
    KeyStoneClient client;
    std::shared_ptr<GetTokenResponse> response = client.GetToken(request);
    if (response == nullptr) {
        errCode = CHECK_OPENSTACK_CONNECT_FAILED;
        ERRLOG("Failed to get token detail.");
        return FAILED;
    }
    if (response->GetStatusCode() != Module::SC_CREATED) {
        ERRLOG("Verify user(%s) failed.", m_application.auth.authkey.c_str());
        errCode = CHECK_OPENSTACK_CONNECT_FAILED;
        return FAILED;
    }
    INFOLOG("Verify user(%s) success.", m_application.auth.authkey.c_str());
    errCode = 0;
    return SUCCESS;
}

/**
 * @brief 获取云平台的所有domain
 *
 * @return int32_t
 */
int32_t OpenStackResourceAccess::GetDomains(ResourceResultByPage &page)
{
    GetDomainsRequest request;
    SetRequestCommonInfo(request, m_appEnv.auth.authkey, m_appEnv.auth.authPwd);
    KeyStoneClient client;
    std::shared_ptr<GetDomainsResponse> response = client.GetDomains(request);
    if (response == nullptr) {
        ERRLOG("Failed to get domains.");
        return FAILED;
    }
    if (response->GetStatusCode() != static_cast<uint32_t>(Module::SC_OK)) {
        ERRLOG("Failed to get domains, status: %d, resp body: %s.", response->GetStatusCode(),
            response->GetBody().c_str());
        return FAILED;
    }
    if (!response->Serial()) {
        ERRLOG("Failed to serial get domains.");
        return FAILED;
    }
    ApplicationResource returnValue;
    for (const auto &domain : response->GetDomainList().m_domainlist) {
        returnValue.__set_type("StackDomain");
        returnValue.__set_subType("OpenStackDomain");
        returnValue.__set_id(domain.m_id);
        returnValue.__set_name(domain.m_name);
        returnValue.__set_parentId(m_appEnv.id);
        returnValue.__set_parentName(m_appEnv.name);
        returnValue.__set_extendInfo("{}");
        page.items.push_back(returnValue);
        DBGLOG("Get domain %s.", domain.m_name.c_str());
    }
    page.__set_total(page.items.size());
    DBGLOG("Get domain list success.");
    return SUCCESS;
}

int32_t OpenStackResourceAccess::GetProjectLists(ResourceResultByPage &page)
{
    GetDomainProjectsRequest request;
    SetRequestCommonInfo(request, m_appEnv.auth.authkey, m_appEnv.auth.authPwd);
    request.SetDomain(m_application.name);
    request.SetDomainId(m_application.id);
    DBGLOG("Domain name %s, id(%s).", m_application.name.c_str(), m_application.id.c_str());
    if (request.GetDomainId().empty()) {
        ERRLOG("Domain id is empty, Failed to get project list.");
        return FAILED;
    }
    KeyStoneClient client;
    std::shared_ptr<GetDomainProjectsResponse> response = client.GetDomainProjects(request);
    if (response == nullptr) {
        ERRLOG("Failed to get projects.");
        return FAILED;
    }
    if (response->GetStatusCode() != static_cast<uint32_t>(Module::SC_OK)) {
        ERRLOG("Failed to get projects, status: %d, resp body: %s.", response->GetStatusCode(),
            response->GetBody().c_str());
        return FAILED;
    }
    if (!response->Serial()) {
        ERRLOG("Failed to serial get projects body.");
        return FAILED;
    }
    
    ApplicationResource returnValue;
    for (const auto &project : response->GetProjectList().m_projects) {
        returnValue.__set_type("StackProject");
        returnValue.__set_subType("OpenStackProject");
        returnValue.__set_id(project.m_id);
        returnValue.__set_name(project.m_name);
        returnValue.__set_parentId(m_application.id);
        returnValue.__set_parentName(m_application.name);
        returnValue.__set_extendInfo("{}");
        page.items.push_back(returnValue);
        DBGLOG("Get domain %s project name %s.", request.GetDomain().c_str(), project.m_name.c_str());
    }
    page.__set_total(page.items.size());
    INFOLOG("Get project(%s) list success.", m_application.name.c_str());
    return SUCCESS;
}

int32_t OpenStackResourceAccess::GetProjectServers(ResourceResultByPage& page)
{
    GetProjectServersRequest request;
    SetRequestCommonInfo(request, m_application.auth.authkey, m_application.auth.authPwd);
    if (SetDomainInfo(request) != SUCCESS) {
        ERRLOG("Get Project servers failed by set domain failed.");
        return FAILED;
    }
    if (request.GetDomain().empty() || request.GetDomainId().empty()) {
        ERRLOG("Domain name or id is empty.");
        return FAILED;
    }
    request.SetScopeValue(m_application.id);  // project id
    request.SetServerLimit(m_condition.pageSize);
    Json::Value extendInfoJson;
    if (!Module::JsonHelper::JsonStringToJsonValue(m_condition.conditions, extendInfoJson)) {
        ERRLOG("Failed to convert extend info.");
        return FAILED;
    }
    if (extendInfoJson.isMember("marker")) {
        request.SetServerMarker(extendInfoJson["marker"].asString());
    }
    NovaClient novaClient;
    std::shared_ptr<GetProjectServersResponse> response = novaClient.GetProjectServers(request);
    if (response == nullptr) {
        ERRLOG("Failed to get server list.");
        return FAILED;
    }
    if (response->GetStatusCode() != static_cast<uint32_t>(Module::SC_OK)) {
        ERRLOG("Failed to get server list, status %d, body: %s.", response->GetStatusCode(),
            response->GetBody().c_str());
        return FAILED;
    }
    if (!response->Serial()) {
        ERRLOG("Failed to serial get projects body.");
        return FAILED;
    }
    for (const auto &server : response->GetProjectServerList().m_serverList) {
        ApplicationResource returnValue;
        if (ComposeServerDetail(returnValue, server, request.GetDomainId(), request.GetDomain()) != SUCCESS) {
            ERRLOG("Failed to compose server.");
            continue;
        }
        page.items.push_back(returnValue);
    }
    page.__set_total(page.items.size());
    INFOLOG("Get project(%s) servers success.", m_application.name.c_str());
    return SUCCESS;
}

int32_t OpenStackResourceAccess::ComposeServerDetail(ApplicationResource& serverResource,
    const OpenStackServerInfo& serverInfo, const std::string &domainId, const std::string &domainName)
{
    CloudServerResource serverRes;
    serverRes.id = serverInfo.m_uuid;
    serverRes.name = serverInfo.m_name;
    serverRes.projectId = serverInfo.m_tenantId;
    serverRes.status = serverInfo.m_oSEXTSTSvmState;
    serverRes.m_domainId = domainId;
    serverRes.m_domainName = domainName;
    serverRes.m_flavorId = serverInfo.m_flavor.m_id;
    serverRes.m_networks = serverInfo.m_networks;
    std::vector<std::string> addresses = serverInfo.m_addresses;
    serverRes.vmIp = boost::algorithm::join(addresses, ",");

    std::string serverExtendInfoStr = "";
    if (!Module::JsonHelper::StructToJsonString(serverRes, serverExtendInfoStr)) {
        ERRLOG("Server resource format conversion failed.");
        return FAILED;
    }
    serverResource.__set_type("CloudServer");
    serverResource.__set_subType("OpenStackCloudServer");
    serverResource.__set_parentId(m_application.id);  // project id
    serverResource.__set_parentName(m_application.name);
    serverResource.__set_id(serverInfo.m_uuid);
    serverResource.__set_name(serverInfo.m_name);
    serverResource.__set_extendInfo(serverExtendInfoStr);
    DBGLOG("serverExtendInfoStr is %s.", serverExtendInfoStr.c_str());
    DBGLOG("Server id %s, name %s, project id %s.", serverInfo.m_uuid.c_str(), serverInfo.m_name.c_str(),
        m_application.id.c_str());
    return SUCCESS;
}

int32_t OpenStackResourceAccess::GetFlavors(ResourceResultByPage& page)
{
    GetFlavorsDetailRequest request;
    SetRequestCommonInfo(request, m_application.auth.authkey, m_application.auth.authPwd);
    if (SetDomainInfo(request) != SUCCESS) {
        ERRLOG("Get flavors failed by set domain failed.");
        return FAILED;
    }
    request.SetScopeValue(m_application.id);
    NovaClient novaClient;
    std::shared_ptr<GetFlavorsDetailResponse> response = novaClient.GetFlavorsDetail(request);
    if (response == nullptr) {
        ERRLOG("Get flavors failed, nullptr.");
        return FAILED;
    }
    if (response->GetStatusCode() != static_cast<uint32_t>(Module::SC_OK)) {
        ERRLOG("Get flavors error failed, statusCode: %u.",
            response->GetStatusCode());
        return FAILED;
    }
    if (!response->Serial()) {
        ERRLOG("Failed to serial get flavors body.");
        return FAILED;
    }
    for (const auto &flavor : response->GetFlavorsDetail().m_flavors) {
        ApplicationResource returnValue;
        if (ComposeFlavorInfo(returnValue, flavor) != SUCCESS) {
            ERRLOG("Failed to compose flavor, %s.", flavor.m_name.c_str());
            continue;
        }
        page.items.push_back(returnValue);
    }
    page.__set_total(page.items.size());
    INFOLOG("Get project(%s) flavors success.", m_application.name.c_str());
    return SUCCESS;
}
 
int32_t OpenStackResourceAccess::ComposeFlavorInfo(ApplicationResource& flavorResource, const ServerFlavor &flavor)
{
    std::string flavorInfoString;
    FlavorResouce flavorInfo;
    flavorInfo.m_name = flavor.m_name;
    flavorInfo.m_id = flavor.m_id;
    flavorInfo.m_public = flavor.m_osFlvDisabled;
    flavorInfo.m_disabled = flavor.m_osFlavorAccessIsPublic;
    if (!Module::JsonHelper::StructToJsonString(flavorInfo, flavorInfoString)) {
        ERRLOG("Flavor info format conversion failed.");
        return FAILED;
    }
    flavorResource.__set_type("Flavor");
    flavorResource.__set_subType("OpenStackFlavor");
    flavorResource.__set_parentId(m_application.id);    // 项目id
    flavorResource.__set_id(flavorInfo.m_id);
    flavorResource.__set_name(flavorInfo.m_name);
    flavorResource.__set_extendInfo(flavorInfoString);
    DBGLOG("FlavorInfoString is %s.", flavorInfoString.c_str());
    return SUCCESS;
}

int32_t OpenStackResourceAccess::GetProjectVolumes(ResourceResultByPage& page)
{
    GetProjectVolumesRequest request;
    SetRequestCommonInfo(request, m_application.auth.authkey, m_application.auth.authPwd);
    if (SetDomainInfo(request) != SUCCESS) {
        ERRLOG("Get Project volumes failed by set domain failed.");
        return FAILED;
    }
    request.SetScopeValue(m_application.id);
    request.SetVolumeOffset(m_condition.pageNo * m_condition.pageSize);
    request.SetVolumeLimit(m_condition.pageSize);
    request.SetVolumeStatus(VOLUME_STATUS_IN_USE);
    CinderClient cinderClient;
    std::shared_ptr<GetProjectVolumesResponse> response = cinderClient.GetProjectVolumes(request);
    if (response == nullptr) {
        ERRLOG("Failed to get project volumes.");
        return FAILED;
    }
    if (response->GetStatusCode() != static_cast<uint32_t>(Module::SC_OK)) {
        ERRLOG("Failed to get project volumes, status: %d, body: %s.", response->GetStatusCode(),
            response->GetBody().c_str());
        return FAILED;
    }
    if (!response->Serial()) {
        ERRLOG("Failed to serial get projects body.");
        return FAILED;
    }
    for (const auto &vol : response->GetProjectVolumes().m_volumeList) {
        ApplicationResource returnValue;
        if (ComposeVolumeInfo(returnValue, vol) != SUCCESS) {
            ERRLOG("Failed to compose volume, %s.", vol.m_id.c_str());
            continue;
        }
        page.items.push_back(returnValue);
    }
    page.__set_total(page.items.size());
    INFOLOG("Get project(%s) volumes success.", m_application.name.c_str());
    return SUCCESS;
}


int32_t OpenStackResourceAccess::ComposeVolumeInfo(ApplicationResource& volumeResource, const Volume& volumeInfo)
{
    if (volumeInfo.m_attachPoints.empty()) {
        DBGLOG("Volume(%s) attach points is empty.", volumeInfo.m_id.c_str());
        return FAILED;
    }
    VolumeResource volRes;
    volRes.id = volumeInfo.m_id;
    volRes.name = volumeInfo.m_name;
    volRes.bootable = volumeInfo.m_bootable;
    volRes.shareable = volumeInfo.m_shareable;
    std::ostringstream os;
    os << volumeInfo.m_size;
    volRes.size = os.str();
    volRes.architecture = volumeInfo.m_volImageMetadata.m_arch;
    volRes.m_fullClone = volumeInfo.m_metaData.m_fullClone;
    volRes.m_volumeType = volumeInfo.m_volumeType;
    if (volumeInfo.m_attachPoints.size() != 0) {
        volRes.mDevice = volumeInfo.m_attachPoints[0].m_device;
    }

    std::string volInfoString;
    if (!Module::JsonHelper::StructToJsonString(volRes, volInfoString)) {
        ERRLOG("Volume info format conversion failed.");
        return FAILED;
    }
    volumeResource.__set_type("CloudVolume");
    volumeResource.__set_subType("OpenStackVolume");
    volumeResource.__set_parentId(volumeInfo.m_attachPoints[0].m_serverId); // 非共享盘只能有一个挂载点
    volumeResource.__set_id(volumeInfo.m_id);
    volumeResource.__set_name(volumeInfo.m_name);
    volumeResource.__set_extendInfo(volInfoString);
    DBGLOG("VolInfoString is %s.", volInfoString.c_str());
    DBGLOG("vol id %s, name %s, server id %s.", volumeInfo.m_id.c_str(), volumeInfo.m_name.c_str(),
        volumeInfo.m_attachPoints[0].m_serverId.c_str());
    return SUCCESS;
}

int32_t OpenStackResourceAccess::GetVolumeTypes(ResourceResultByPage& page)
{
    GetVolumeTypesRequest request;
    SetRequestCommonInfo(request, m_application.auth.authkey, m_application.auth.authPwd);
    if (SetDomainInfo(request) != SUCCESS) {
        ERRLOG("Get volume types failed by set domain failed.");
        return FAILED;
    }
    request.SetScopeValue(m_application.id);
    CinderClient cinderClient;
    std::shared_ptr<GetVolumeTypesResponse> response = cinderClient.GetVolumeTypes(request);
    if (response == nullptr) {
        ERRLOG("Get volume types failed, nullptr.");
        return FAILED;
    }
    if (response->GetStatusCode() != static_cast<uint32_t>(Module::SC_OK)) {
        ERRLOG("Get Volume type error failed, statusCode: %u.",
            response->GetStatusCode());
        return FAILED;
    }
    if (!response->Serial()) {
        ERRLOG("Failed to serial get volume types body.");
        return FAILED;
    }
    for (const auto &volumeType : response->GetVolumeTypes().m_volumeTypes) {
        ApplicationResource returnValue;
        if (ComposeVolumeTypeInfo(returnValue, volumeType) != SUCCESS) {
            ERRLOG("Failed to compose volume type, %s.", volumeType.m_name.c_str());
            continue;
        }
        page.items.push_back(returnValue);
    }
    page.__set_total(page.items.size());
    INFOLOG("Get project(%s) volumeTypes success.", m_application.name.c_str());
    return SUCCESS;
}
 
int32_t OpenStackResourceAccess::ComposeVolumeTypeInfo(ApplicationResource& volumeTypeResource,
    const VolumeType &volumeType)
{
    std::string volumeTypeInfoString;
    VolumeTypeResource volumeTypeInfo;
    volumeTypeInfo.m_name = volumeType.m_name;
    volumeTypeInfo.m_id = volumeType.m_id;
    volumeTypeInfo.m_isPublic = volumeType.m_isPublic;
    volumeTypeInfo.m_description = volumeType.m_description;
    volumeTypeInfo.availabilityZone = volumeType.m_extraSpecs.m_availabilityZone;
    volumeTypeInfo.volumeBackEndName = volumeType.m_extraSpecs.m_volumeBackendName;
    if (!Module::JsonHelper::StructToJsonString(volumeTypeInfo, volumeTypeInfoString)) {
        ERRLOG("VolumeType info format conversion failed.");
        return FAILED;
    }
    volumeTypeResource.__set_type("VolumeType");
    volumeTypeResource.__set_subType("OpenStackVolumeType");
    volumeTypeResource.__set_parentId(m_application.id);    // 项目id
    volumeTypeResource.__set_id(volumeType.m_id);
    volumeTypeResource.__set_name(volumeType.m_name);
    volumeTypeResource.__set_extendInfo(volumeTypeInfoString);
    return SUCCESS;
}
 
int32_t OpenStackResourceAccess::GetNetworks(ResourceResultByPage& page)
{
    GetNetworksRequest request;
    SetRequestCommonInfo(request, m_application.auth.authkey, m_application.auth.authPwd);
    if (SetDomainInfo(request) != SUCCESS) {
        ERRLOG("Get networks failed by set domain failed.");
        return FAILED;
    }
    request.SetScopeValue(m_application.id);  // project id
    NeutronClient neutronClient;
    std::shared_ptr<GetNetworksResponse> response = neutronClient.GetNetworks(request);
    if (response == nullptr) {
        ERRLOG("Get networks request failed.");
        return FAILED;
    }
    if (response->GetStatusCode() != static_cast<uint32_t>(Module::SC_OK)) {
        ERRLOG("Get networks request failed, status(%d), body(%s)", response->GetStatusCode(),
            response->GetBody().c_str());
        return FAILED;
    }
    for (const auto &network : response->GetNetworks().m_networks) {
        ApplicationResource returnValue;
        if (ComposeNetworkInfo(returnValue, network) != SUCCESS) {
            ERRLOG("Failed to compose network, %s.", network.m_id.c_str());
            continue;
        }
        page.items.push_back(returnValue);
    }
    page.__set_total(page.items.size());
    INFOLOG("Get project(%s) networks success.", m_application.name.c_str());
    return SUCCESS;
}

int32_t OpenStackResourceAccess::ComposeNetworkInfo(ApplicationResource& networkResource, const Network& network)
{
    NetworkResource networkInfo;
    networkInfo.m_id = network.m_id;
    networkInfo.m_name = network.m_name;
    networkInfo.m_projectId = network.m_projectId;
    networkInfo.m_networkType = network.m_providerNetworkType;
    networkInfo.m_physicalNetwork = network.m_providerPhysicalNetwork;
    networkInfo.m_shared = network.m_shared;
    networkInfo.m_status = network.m_status;
    std::string networkInfoString;
    if (!Module::JsonHelper::StructToJsonString(networkInfo, networkInfoString)) {
        ERRLOG("Network info format conversion failed.");
        return FAILED;
    }
    networkResource.__set_type("Network");
    networkResource.__set_subType("OpenStackNetwork");
    networkResource.__set_parentId(m_application.id);    // 项目id
    networkResource.__set_id(network.m_id);
    networkResource.__set_name(network.m_name);
    networkResource.__set_extendInfo(networkInfoString);
    DBGLOG("NetworkInfoString is %s.", networkInfoString.c_str());
    return SUCCESS;
}

int32_t OpenStackResourceAccess::GetAvailabilityZones(ResourceResultByPage& page)
{
    ServerRequest request;
    SetRequestCommonInfo(request, m_application.auth.authkey, m_application.auth.authPwd);
    if (SetDomainInfo(request) != SUCCESS) {
        ERRLOG("Get availability zones failed by set domain failed.");
        return FAILED;
    }
    request.SetScopeValue(m_application.id);  // project id
    NovaClient novaClient;
    std::shared_ptr<GetAvailabilityZonesResponse> response = novaClient.GetAvailabilityZones(request);
    if (response == nullptr) {
        ERRLOG("Get availability zones request failed.");
        return FAILED;
    }
    if (response->GetStatusCode() != static_cast<uint32_t>(Module::SC_OK)) {
        ERRLOG("Get availability zones request failed, status(%d), body(%s)", response->GetStatusCode(),
            response->GetBody().c_str());
        return FAILED;
    }
    for (const auto &availabilityZone : response->GetAvailabilityZones().m_availabilityZoneInfo) {
        if (availabilityZone.zoneName == "internal") {
            continue;
        }
        ApplicationResource returnValue;
        if (ComposeAvailabilityZone(returnValue, availabilityZone) != SUCCESS) {
            ERRLOG("Failed to compose availability zone, %s.", availabilityZone.zoneName.c_str());
            continue;
        }
        page.items.push_back(returnValue);
    }
    page.__set_total(page.items.size());
    INFOLOG("Get project(%s) availability zone  success.", m_application.name.c_str());
    return SUCCESS;
}

int32_t OpenStackResourceAccess::ComposeAvailabilityZone(ApplicationResource& networkResource,
    const AvailabilityZone& availabilityZone)
{
    AvailabilityZoneResource availabilityZoneInfo;
    availabilityZoneInfo.zoneName = availabilityZone.zoneName;
    availabilityZoneInfo.available = availabilityZone.zoneState.available ? "true" : "false";
    availabilityZoneInfo.hosts = availabilityZone.hosts;
    std::string availabilityZoneString;
    if (!Module::JsonHelper::StructToJsonString(availabilityZoneInfo, availabilityZoneString)) {
        ERRLOG("Availability zone info format conversion failed.");
        return FAILED;
    }
    networkResource.__set_type("AvailabilityZone");
    networkResource.__set_subType("OpenStackAvailabilityZone");
    networkResource.__set_parentId(m_application.id);    // 项目id
    networkResource.__set_id(availabilityZone.zoneName);
    networkResource.__set_name(availabilityZone.zoneName);
    networkResource.__set_extendInfo(availabilityZoneString);
    DBGLOG("Availability zone info string is %s.", availabilityZoneString.c_str());
    return SUCCESS;
}

int32_t OpenStackResourceAccess::GetService(const std::string& name, ServiceInfo& service)
{
    GetServicesRequest request;
    SetRequestCommonInfo(request, m_appEnv.auth.authkey, m_appEnv.auth.authPwd);
    request.SetServiceName(name);
    KeyStoneClient client;
    std::shared_ptr<GetServicesResponse> response = client.GetServices(request);
    if (response == nullptr) {
        ERRLOG("Failed to get services.");
        return FAILED;
    }
    if (response->GetServices().m_services.size() != 1) {
        ERRLOG("No service find, name: %s.", name.c_str());
        return FAILED;
    }
    service = response->GetServices().m_services[0];
    DBGLOG("Get services success.");
    return SUCCESS;
}

int32_t OpenStackResourceAccess::GetAppCluster(ApplicationEnvironment& returnEnv)
{
    ServiceInfo targetService;
    if (GetService(OPENSTACK_SERVICE_NAME_KEYSTONE_V3, targetService) != SUCCESS &&
            GetService(OPENSTACK_SERVICE_NAME_KEYSTONE, targetService) != SUCCESS) {
        ERRLOG("Get service detail failed.");
        return FAILED;
    }

    std::string flag = Module::ConfigReader::getString("OpenStackConfig", "RegisterServiceToOpenStack");
    std::string cpsIp;
    if (flag == "true") {
        std::vector<std::string> cmdOut;
        std::string agentHomedir = Module::EnvVarManager::GetInstance()->GetAgentHomePath();
        std::vector<Module::CmdParam> cmdParam{
            Module::CmdParam(Module::COMMON_CMD_NAME, "sudo"),
            Module::CmdParam(Module::SCRIPT_CMD_NAME, agentHomedir + SUDO_DISK_TOOL_PATH),
            "get_openstack_cps_ip"
        };
        if (Module::RunCommand("sudo", cmdParam, cmdOut) != 0) {
            WARNLOG("Get cps ip failed, ignore fail.");
        }
        if (cmdOut.size() == 1) {
            cpsIp = cmdOut[0];
        }
    }
    DBGLOG("Register flag(%s), cps ip(%s).", flag.c_str(), cpsIp.c_str());
    AppClusterInfo cluster(targetService, flag, cpsIp);
    std::string clusterInfostr;
    if (!Module::JsonHelper::StructToJsonString(cluster, clusterInfostr)) {
        ERRLOG("Failed to convert cluster detail.");
        return FAILED;
    }
    DBGLOG("Cluster info: %s.", clusterInfostr.c_str());
    returnEnv.__set_extendInfo(clusterInfostr);
    return SUCCESS;
}

int32_t OpenStackResourceAccess::VerifyEnvUser(int32_t& errCode)
{
    GetTokenRequest request;
    request.SetNeedRetry(false);
    request.SetScopeType(Scope::ADMIN_PROJECT);
    SetRequestCommonInfo(request, m_appEnv.auth.authkey, m_appEnv.auth.authPwd);
    KeyStoneClient client;
    std::shared_ptr<GetTokenResponse> response = client.GetToken(request);
    if (response == nullptr) {
        errCode = CHECK_NETWORK_TIME_OUT;
        ERRLOG("Failed to get services.");
        return FAILED;
    }
    if (response->GetStatusCode() != Module::SC_CREATED) {
        ERRLOG("Failed to get token.");
        if (OpenStackCertErrorCode.count(response->GetErrCode()) != 0) {
            errCode = CHECK_OPENSTACK_CERT_FAILED;
        } else if (response->GetStatusCode() == Module::SC_UNAUTHORIZED) {
            errCode = CHCEK_USER_FAILED;
        } else {
            errCode = CHECK_NETWORK_TIME_OUT;
        }
        std::string errBodyStr = response->GetBody();
        if (errBodyStr.find(SYMBOL_USER_IS_LOCKED) != std::string::npos) {      // if it is locked
            errCode = CHECK_OPENSTACK_USER_IS_LOCKED;
        }
        return FAILED;
    }
    return SUCCESS;
}

/**
 * @brief 添加云平台/添加DOMAIN帐号/周期性连通性检查会调用该接口检查连通性
 *
 * @param returnValue
 * @return int32_t
 */
int32_t OpenStackResourceAccess::CheckAppConnect(ActionResult &returnValue)
{
    Json::Value appExtendInfo;
    if (!Module::JsonHelper::JsonStringToJsonValue(m_application.extendInfo, appExtendInfo)) {
        ERRLOG("Failed to convert extend info.");
        returnValue.__set_code(CHECK_OPENSTACK_CONNECT_FAILED);
        return FAILED;
    }

    int32_t errCode = 0;
    if (appExtendInfo["isDomain"].asString() == "true") {
        if (VerifyDomainUser(errCode) != SUCCESS) {
            returnValue.__set_code(errCode);
            returnValue.__set_bodyErr(errCode);
            return FAILED;
        }
    } else if (appExtendInfo["healthCheck"].asString() == "true") {
        ServiceInfo targetService;
        if (GetService(OPENSTACK_SERVICE_NAME_KEYSTONE_V3, targetService) != SUCCESS &&
                GetService(OPENSTACK_SERVICE_NAME_KEYSTONE, targetService) != SUCCESS) {
            returnValue.__set_bodyErr(CHECK_OPENSTACK_CONNECT_FAILED);
            returnValue.__set_code(CHECK_OPENSTACK_CONNECT_FAILED);
            ERRLOG("Get service detail failed.");
            return FAILED;
        }
    } else if (VerifyEnvUser(errCode) != SUCCESS) {
        returnValue.__set_code(errCode);
        returnValue.__set_bodyErr(errCode);
        return FAILED;
    }
    returnValue.__set_code(0);
    return SUCCESS;
}

OPENSTACK_PLUGIN_NAMESPACE_END
