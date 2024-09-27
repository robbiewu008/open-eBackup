#ifndef JOB_SERVER_RPC_PUBLISH_OBSERVER_H_
#define JOB_SERVER_RPC_PUBLISH_OBSERVER_H_
#include <servicecenter/messageservice/include/RpcPublishObserver.h>

namespace jobservice {
class JobServerRpcPublishObserver : public messageservice::RpcPublishObserver {
public:
    JobServerRpcPublishObserver(){};
    virtual ~JobServerRpcPublishObserver()
    {}
    void SetProcessor(std::shared_ptr<TProcessor> processor);

protected:
    void Update(std::shared_ptr<messageservice::RpcPublishEvent> event);
};
}  // namespace jobservice
#endif