#ifndef AGENT_ISERVICE_H_
#define AGENT_ISERVICE_H_
#include <memory>

namespace servicecenter {
class IService : public std::enable_shared_from_this<IService> {
public:
    virtual ~IService(){};
    virtual bool Initailize() = 0;
    virtual bool Uninitailize() = 0;
};
}

#endif