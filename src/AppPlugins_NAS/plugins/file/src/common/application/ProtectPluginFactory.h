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
#ifndef APPPLUGIN_NAS_PROTECTPLUGINFACTORY_H
#define APPPLUGIN_NAS_PROTECTPLUGINFACTORY_H
#include <map>
#include <functional>
#include <memory>
#include <algorithm>
#include "ApplicationManager.h"
namespace FilePlugin {
enum class ResourceType {
    NONE = -1,
    NFS = 0,
    CIFS,
    UNIX,
    WINDOWS
};

class ProtectPluginFactory {
public:
    ProtectPluginFactory(const ProtectPluginFactory&) = delete;
    ProtectPluginFactory& operator=(const ProtectPluginFactory&) = delete;
    ProtectPluginFactory(ProtectPluginFactory&&) = delete;
    ProtectPluginFactory& operator=(ProtectPluginFactory&&) = delete;
    static ProtectPluginFactory& GetInstance()
    {
        static ProtectPluginFactory instance;
        return instance;
    }
    void RegFunction(ResourceType type, std::function<AppManagerPtr()> func)
    {
        functions.insert(std::make_pair(type, func));
    }
    AppManagerPtr Create(ResourceType type)
    {
        auto iter = functions.find(type);
        if (iter == functions.end()) {
            return nullptr;
        }
        return iter->second();
    }
    AppManagerPtr Create(const std::string &resourceType)
    {
        return Create(ConvertToEnum(resourceType));
    }
    
private:
    ProtectPluginFactory() = default;
    ~ProtectPluginFactory() noexcept {};
    ResourceType ConvertToEnum(const std::string& resourceType)
    {
        std::string srcResourceType = resourceType;
        transform(srcResourceType.begin(), srcResourceType.end(), srcResourceType.begin(), ::tolower);
        if (srcResourceType == "linux" || srcResourceType == "unix" || srcResourceType == "aix"
            || srcResourceType == "solaris") {
            return ResourceType::UNIX;
        }
        if (srcResourceType == "cifs") {
            return ResourceType::CIFS;
        }
        if (srcResourceType == "nfs") {
            return ResourceType::NFS;
        }
        if (srcResourceType == "windows") {
            return ResourceType::WINDOWS;
        }
        return ResourceType::NONE;
    }
private:
    std::map<ResourceType, std::function<AppManagerPtr()>> functions;
};
// 使用方法: 在不同的资源处理类实现的cpp中使用static AutoRegAppManager<T> g_autoReg{ResourceType}
template<typename AppManagerT>
class AutoRegAppManager {
public:
    explicit AutoRegAppManager(ResourceType type)
    {
        ProtectPluginFactory::GetInstance().RegFunction(type, [] { return std::make_unique<AppManagerT>(); });
    }
};
}
#endif // APPPLUGIN_NAS_PROTECTPLUGINFACTORY_H
