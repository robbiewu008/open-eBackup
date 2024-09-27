/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 *
 * @file ServiceFactory.h
 * @brief  Factory for Service Center
 * @version 1.1.0
 * @date 2021-08-31
 * @author caomin 00511255
 */

#ifndef SERVICE_FACTORY_H_
#define SERVICE_FACTORY_H_

#include <memory>
#include "servicecenter/servicefactory/include/IServiceCenter.h"
#include "servicecenter/servicefactory/include/IService.h"

namespace servicecenter {
class ServiceFactory {
public:
    ServiceFactory();
    ~ServiceFactory() = default;

    static ServiceFactory* GetInstance();
    template<typename T>
    bool Register(const std::string& name)
    {
        return IServiceCenter::GetInstance()->Register(name,
            []() -> std::shared_ptr<IService> { return std::dynamic_pointer_cast<IService>(std::make_shared<T>()); });
    }

    bool Unregister(const std::string& name)
    {
        return IServiceCenter::GetInstance()->Unregister(name);
    }

    template<typename T>
    std::shared_ptr<T> GetService(const std::string& name)
    {
        return std::dynamic_pointer_cast<T>(GetIService(name));
    }

private:
    std::shared_ptr<IService> GetIService(const std::string& name)
    {
        return IServiceCenter::GetInstance()->GetService(name);
    }

private:
    static std::shared_ptr<IServiceCenter> GetServiceCenter();
};
}  // namespace servicecenter

#endif