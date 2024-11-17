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
#ifndef SERVICE_FACTORY_H
#define SERVICE_FACTORY_H

#include <memory>
#include "IServiceCenter.h"
#include "IService.h"

namespace servicecenter {

class ServiceFactory {
public:
    ServiceFactory();
    ~ServiceFactory() = default;

    static ServiceFactory* GetInstance();
    template<typename T>
    bool Register(std::string name)
    {
        return IServiceCenter::GetInstance()->Register(name, []() -> std::shared_ptr<IService> {
            return std::dynamic_pointer_cast<IService>(std::make_shared<T>());
        });
    }

    bool Unregister(std::string name)
    {
        return IServiceCenter::GetInstance()->Unregister(name);
    }

    template<typename T>
    std::shared_ptr<T> GetService(std::string name)
    {
        return std::dynamic_pointer_cast<T>(GetIService(name));
    }

private:
    std::shared_ptr<IService> GetIService(std::string name)
    {
        return IServiceCenter::GetInstance()->GetService(name);
    }

private:
    static std::shared_ptr<IServiceCenter> GetServiceCenter();
};
}

#endif