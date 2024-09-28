#include "servicecenter/messageservice/include/RpcPublishObserver.h"
#include "common/Log.h"
#include "servicecenter/servicefactory/include/ServiceFactory.h"

namespace messageservice {
void RpcPublishObserver::Update(std::shared_ptr<messageservice::IEvent> event)
{
    std::shared_ptr<RpcPublishEvent> rpcPubEvent(std::dynamic_pointer_cast<RpcPublishEvent>(event));
    if (rpcPubEvent.get() == nullptr) {
        ERRLOG("RpcPublishObserver received a null event.");
        return;
    }
    Update(rpcPubEvent);
}
}
