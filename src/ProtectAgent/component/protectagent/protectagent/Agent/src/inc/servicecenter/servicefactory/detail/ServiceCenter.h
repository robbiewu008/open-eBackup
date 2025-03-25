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
#ifndef SERVICERCENTER_H_
#define SERVICERCENTER_H_

#include <string>
#include <memory>
#include <map>
#include <mutex>
#include "servicecenter/servicefactory/include/IServiceCenter.h"

namespace servicecenter {
namespace detail {
class ServiceCenter : public IServiceCenter {
public:
    ~ServiceCenter();
    std::shared_ptr<IService> GetService(const std::string& name);
    virtual bool Initailize();
    virtual bool Register(const std::string& name, const ServiceCreator& creator);
    virtual bool Unregister(const std::string& name);

private:
    std::shared_ptr<IService> CreateIService(const std::string& name, const ServiceCreator& creator);
    std::shared_ptr<IService> GetIServiceFromInstances(const std::string& name);
    ServiceCreator GetCreator(const std::string& name);
private:
    std::mutex m_lock;
    std::map<std::string, ServiceCreator> m_creaters;
    std::map<std::string, std::shared_ptr<IService>> m_serviceInstances;
};
}
}
#endif