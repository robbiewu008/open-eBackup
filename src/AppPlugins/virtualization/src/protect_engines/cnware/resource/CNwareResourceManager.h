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
#ifndef APPPLUGINS_VIRTUALIZATION_CNWARERESOURCEMANAGER_H
#define APPPLUGINS_VIRTUALIZATION_CNWARERESOURCEMANAGER_H

#include <vector>
#include <map>
#include "protect_engines/cnware/api/client/CNwareClient.h"

using AppProtect::Application;
using AppProtect::ApplicationEnvironment;
using AppProtect::QueryByPage;
using AppProtect::ResourceResultByPage;

namespace CNwarePlugin {

class CNwareResourceManager {
public:
    CNwareResourceManager(const ApplicationEnvironment appEnv, QueryByPage pageInfo,
        std::shared_ptr<CNwareClient> m_Client);
    ~CNwareResourceManager();

    int32_t GetTargetResource(ResourceResultByPage& page, CNwareRequest &req);

protected:
    int32_t SetCluster(ApplicationResource &result, const Json::Value &items);
    int32_t SetHostPool(ApplicationResource &result, const Json::Value &items);
    int32_t SetHost(ApplicationResource &result, const Json::Value &items);
    int32_t SetVm(ApplicationResource &result, const Json::Value &items);
    int32_t SetDisk(ApplicationResource &result, const Json::Value &items);
    int32_t SetResourceUrl(CNwareRequest &req, const std::string &subType);
    int32_t ParseResponse(ResourceResultByPage &page, std::shared_ptr<ResponseModel> response,
        const std::string &type);
    int32_t SetResourceResult(ApplicationResource &result, const Json::Value &items, const std::string &subType);
    void ParseJsonValue(const Json::Value &jsonValue, const Json::Value &conditionJson,
        std::vector<ApplicationResource> &resourceResults, const std::string &subType);
    bool GetResourceType(std::string &subType);
    int32_t GetStoragePool(ResourceResultByPage &page, CNwareRequest &req);
    int32_t GetPortGroup(ResourceResultByPage &page, CNwareRequest &req);
    void SetStorageDetails(const StoragePool &storage, ApplicationResource &result);

protected:
    std::map<std::string, std::function<int32_t(
        ApplicationResource &result, const Json::Value &items)>> m_resourceFuncMap;
    Application m_application;
    ApplicationEnvironment m_appEnv;
    QueryByPage m_condition;
    Json::Value m_appExtendInfo;
    std::shared_ptr<CNwareClient> m_cnwareClient;
    int m_total;
    std::string m_vmId;
};
}
#endif // APPPLUGINS_VIRTUALIZATION_CNWARERESOURCEMANAGER_H
