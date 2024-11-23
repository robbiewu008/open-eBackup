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
#ifndef APS_RESOURCE_ACCESS_H
#define APS_RESOURCE_ACCESS_H

#include <boost/algorithm/string.hpp>
#include "curl_http/HttpStatus.h"
#include "common/execute_python/ExecutePython.h"
#include "common/cert_mgr/CertMgr.h"
#include "protect_engines/apsara_stack/common/Structs.h"

using AppProtect::Application;
using AppProtect::ApplicationEnvironment;
using AppProtect::QueryByPage;
using AppProtect::ResourceResultByPage;

using namespace VirtPlugin;
namespace ApsaraStackPlugin {

enum class ResourceType {
    TYPE_REGION = 0,
    TYPE_ZONE,
    TYPE_RESOURCESET,
    TYPE_INSTANCE,
    TYPE_DISK
};

class ApsaraStackResourceAccess {
public:
    explicit ApsaraStackResourceAccess(ApplicationEnvironment appEnv);
    ApsaraStackResourceAccess(ApplicationEnvironment appEnv, Application application);
    ApsaraStackResourceAccess(ApplicationEnvironment appEnv, QueryByPage pageInfo);
    ApsaraStackResourceAccess(ApplicationEnvironment appEnv, Application application, QueryByPage pageInfo);
    ~ApsaraStackResourceAccess();
    void SetApplication(Application application);

    int32_t ListResource(ResourceResultByPage &page, const ListResourcePara &conditionPara);
    int32_t CheckAppConnect(ActionResult& returnValue);
    
protected:
    int32_t ParseResourceResponse(ResourceResultByPage &page,
        const std::string &response, const ListResourcePara &conditionPara);
    int32_t GetResource(const ListResourcePara &conditionPara, std::string &response);

    bool SetRegion(const std::string &response, ResourceResultByPage &page);
    bool SetZone(const std::string &response, ResourceResultByPage &page,
        const ListResourcePara &conditionPara);
    bool SetInstance(const std::string &response, ResourceResultByPage &page,
        const ListResourcePara &conditionPara);
    bool SetDisk(const std::string &response, ResourceResultByPage &page);
    bool SetResourceSet(const std::string &response, ResourceResultByPage &page);

    Application m_application;
    ApplicationEnvironment m_appEnv;
    QueryByPage m_condition;
    std::shared_ptr<ExecutePython> m_pyPtr;
};

}
#endif
