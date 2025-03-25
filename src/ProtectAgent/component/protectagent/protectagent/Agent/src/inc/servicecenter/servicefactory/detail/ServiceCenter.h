/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 *
 * @file ServiceCenter.h
 * @brief  Implement for Service Center
 * @version 1.1.0
 * @date 2021-08-31
 * @author caomin 00511255
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