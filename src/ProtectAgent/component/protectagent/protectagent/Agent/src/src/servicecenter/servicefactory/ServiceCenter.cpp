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
#include "servicecenter/servicefactory/detail/ServiceCenter.h"
#include "servicecenter/servicefactory/include/IService.h"

namespace servicecenter {
namespace detail {
ServiceCenter::~ServiceCenter()
{
}

std::shared_ptr<IService> ServiceCenter::GetService(const std::string& name)
{
    auto service = GetIServiceFromInstances(name);
    if (service == nullptr) {
        auto creator = GetCreator(name);
        if (creator) {
            return CreateIService(name, creator);
        }
    }
    return service;
}

std::shared_ptr<IService> ServiceCenter::GetIServiceFromInstances(const std::string& name)
{
    std::lock_guard<std::mutex> lock(m_lock);
    auto it = m_serviceInstances.find(name);
    if (it != m_serviceInstances.end()) {
        return it->second;
    }
    return nullptr;
}

std::shared_ptr<IService> ServiceCenter::CreateIService(const std::string& name, const ServiceCreator& creator)
{
    auto ret = creator();
    if (ret->Initailize()) {
        std::lock_guard<std::mutex> lock(m_lock);
        m_serviceInstances.emplace(name, ret);
        return ret;
    }
    return nullptr;
}

ServiceCreator ServiceCenter::GetCreator(const std::string& name)
{
    ServiceCreator ret;
    std::lock_guard<std::mutex> lock(m_lock);
    auto creatorIt = m_creaters.find(name);
    if (creatorIt != m_creaters.end()) {
        ret = creatorIt->second;
    }
    return ret;
}

bool ServiceCenter::Initailize()
{
    if (IServiceCenter::g_instance != nullptr) {
        return false;
    }
    IServiceCenter::g_instance = shared_from_this();
    return true;
}

bool ServiceCenter::Register(const std::string& name, const ServiceCreator& creator)
{
    std::lock_guard<std::mutex> lock(m_lock);
    if (m_creaters.find(name) != m_creaters.end()) {
        return false;
    }
    m_creaters[name] = creator;
    return true;
}

bool ServiceCenter::Unregister(const std::string& name)
{
    std::shared_ptr<IService> service;
    {
        std::lock_guard<std::mutex> lock(m_lock);
        m_creaters.erase(m_creaters.find(name));
        auto instance = m_serviceInstances.find(name);
        if (instance != m_serviceInstances.end()) {
            service = instance->second;
            m_serviceInstances.erase(instance);
        }
    }
    if (service) {
        service->Uninitailize();
    }
    return true;
}
}
}