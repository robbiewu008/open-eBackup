#ifndef AGENT_ISERVICECENTER_H_
#define AGENT_ISERVICECENTER_H_

#include <memory>
#include <string>
#include <functional>
#include "servicecenter/servicefactory/include/IService.h"

namespace servicecenter {
using ServiceCreator = std::function<std::shared_ptr<IService>()>;

class IServiceCenter : public std::enable_shared_from_this<IServiceCenter> {
public:
    virtual ~IServiceCenter(){};
    virtual std::shared_ptr<IService> GetService(const std::string& name) = 0;
    virtual bool Initailize() = 0;
    virtual bool Register(const std::string& name, const ServiceCreator& creator) = 0;
    virtual bool Unregister(const std::string& name) = 0;

    static std::shared_ptr<IServiceCenter> GetInstance();

protected:
    static std::shared_ptr<IServiceCenter> g_instance;
};
}  // namespace servicecenter

#endif