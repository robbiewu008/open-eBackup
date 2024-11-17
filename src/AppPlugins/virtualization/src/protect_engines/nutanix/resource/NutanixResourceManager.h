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
#ifndef APPPLUGINS_VIRTUALIZATION_NUTANIXRESOURCEMANAGER_H
#define APPPLUGINS_VIRTUALIZATION_NUTANIXRESOURCEMANAGER_H

#include <vector>
#include <map>
#include "protect_engines/nutanix/api/client/NutanixClient.h"

using AppProtect::Application;
using AppProtect::ApplicationEnvironment;
using AppProtect::QueryByPage;
using AppProtect::ResourceResultByPage;

namespace NutanixPlugin {

class NutanixResourceManager {
public:
    NutanixResourceManager(const ApplicationEnvironment appEnv, QueryByPage pageInfo,
        std::shared_ptr<NutanixClient> Client, std::shared_ptr<CertManger> certManger);
    ~NutanixResourceManager() = default;

    int32_t GetTargetResource(ResourceResultByPage& page);

protected:
    void SetCommonInfo(NutanixRequest& req);
    bool GetResourceType(std::string &subType);
    int32_t GetClusterList(ResourceResultByPage& page);
    int32_t GetHostList(ResourceResultByPage& page);
    int32_t GetVMList(ResourceResultByPage& page);
    int32_t GetVMDiskList(ResourceResultByPage& page);
    int32_t GetStorageContainerList(ResourceResultByPage& page);
    int32_t GetNetworkList(ResourceResultByPage& page);
    void SetDiskAddress(Json::Value &diskDetails, const DiskAddress &diskAddress);
    void SetVMInfo(ApplicationResource &result, const VMListResponse &vm);
    void SetStorageContainerInfo(ApplicationResource &result, const ContainerInfo &container);
    void SetDiskInfo(Json::Value &diskDetails, const VmDiskInfo &diskInfo);
    void GetContainerNameFromFilePath(const std::string &filePath, std::string &name);

protected:
    std::map<std::string, std::function<int32_t(
        ResourceResultByPage &page)>> m_resourceFuncMap;
    Application m_application;
    ApplicationEnvironment m_appEnv;
    QueryByPage m_condition;
    Json::Value m_appExtendInfo;
    std::shared_ptr<NutanixClient> m_nutanixClient;
    std::shared_ptr<CertManger> m_certMgr;
    int m_total = 0;
    std::string m_uuId;
};
}
#endif // APPPLUGINS_VIRTUALIZATION_NUTANIXRESOURCEMANAGER_H
