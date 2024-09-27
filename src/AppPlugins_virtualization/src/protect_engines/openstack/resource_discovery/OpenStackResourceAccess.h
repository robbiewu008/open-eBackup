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
#ifndef OPENSTACK_RESOURCE_ACCESS_H
#define OPENSTACK_RESOURCE_ACCESS_H
#include <vector>
#include <map>
#include <boost/algorithm/string.hpp>
#include "common/Structs.h"
#include "common/cert_mgr/CertMgr.h"
#include "thrift_interface/ApplicationProtectBaseDataType_types.h"
#include "thrift_interface/ApplicationProtectPlugin_types.h"
#include "thrift_interface/ApplicationProtectFramework_types.h"
#include "protect_engines/openstack/common/OpenStackMacros.h"
#include "protect_engines/openstack/api/nova/NovaClient.h"
#include "protect_engines/openstack/api/cinder/CinderClient.h"
#include "protect_engines/openstack/api/keystone/KeyStoneClient.h"

using AppProtect::Application;
using AppProtect::ApplicationEnvironment;
using AppProtect::QueryByPage;
using AppProtect::ResourceResultByPage;

OPENSTACK_PLUGIN_NAMESPACE_BEGIN

const int32_t CHECK_OPENSTACK_PARAM_ERROR = 1677929220;
const int32_t CHECK_OPENSTACK_CERT_FAILED = 1677931024;
const int32_t CHECK_OPENSTACK_CONNECT_FAILED = 1577210035;
const int32_t CHECK_OPENSTACK_USER_IS_LOCKED = 1577210067;
const std::string SYMBOL_USER_IS_LOCKED = "lock";
const std::string OPENSTACK_SERVICE_NAME_KEYSTONE_V3 = "keystonev3";
const std::string OPENSTACK_SERVICE_NAME_KEYSTONE = "keystone";
const std::string SUDO_DISK_TOOL_PATH = VirtPlugin::VIRT_PLUGIN_PATH + "bin/security_sudo_disk.sh";
const std::string VOLUME_STATUS_IN_USE = "in-use";

class OpenStackResourceAccess {
public:
    explicit OpenStackResourceAccess(ApplicationEnvironment appEnv);
    OpenStackResourceAccess(ApplicationEnvironment appEnv, QueryByPage pageInfo);
    ~OpenStackResourceAccess();
    void SetApplication(Application application);
    void SetCertMgr(std::shared_ptr<VirtPlugin::CertManger> certMgr);
    int32_t VerifyDomainUser(int32_t& errCode);
    int32_t VerifyEnvUser(int32_t& errCode);
    int32_t GetDomains(ResourceResultByPage &page);
    int32_t GetProjectServers(ResourceResultByPage& page);
    int32_t GetProjectLists(ResourceResultByPage &page);
    int32_t GetFlavors(ResourceResultByPage& page);
    int32_t GetNetworks(ResourceResultByPage& page);
    int32_t GetProjectVolumes(ResourceResultByPage& page);
    int32_t GetVolumeTypes(ResourceResultByPage& page);
    int32_t GetService(const std::string& name, ServiceInfo& Services);
    int32_t CheckAppConnect(ActionResult& returnValue);
    int32_t GetAppCluster(ApplicationEnvironment& returnEnv);
    int32_t GetAvailabilityZones(ResourceResultByPage& page);
protected:
    int32_t ComposeServerDetail(ApplicationResource& serverResource, const OpenStackServerInfo& serverInfo,
        const std::string &domainId, const std::string &domainName);
    int32_t ComposeFlavorInfo(ApplicationResource& volumeTypeResource, const ServerFlavor &flavor);
    void SetRequestCommonInfo(ModelBase &request, const std::string &auth, const std::string &pwd);
    int32_t ComposeVolumeTypeInfo(ApplicationResource& storagePoolResource, const VolumeType &volumeType);
    int32_t ComposeVolumeInfo(ApplicationResource& volumeResource, const Volume& volumeInfo);
    int32_t ComposeNetworkInfo(ApplicationResource& networkResource, const Network& network);
    int32_t SetDomainInfo(ModelBase &request);
    int32_t ComposeAvailabilityZone(ApplicationResource& networkResource,
        const AvailabilityZone& availabilityZone);
protected:
    Application m_application;
    ApplicationEnvironment m_appEnv;
    QueryByPage m_condition;
    Json::Value m_appExtendInfo;
    std::shared_ptr<VirtPlugin::CertManger> m_certMgr;
};

OPENSTACK_PLUGIN_NAMESPACE_END

#endif