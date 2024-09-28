#include <common/Log.h>
#include "servicecenter/services/jobservice/detail/JobServerRpcPublishObserver.h"
namespace jobservice {
void JobServerRpcPublishObserver::SetProcessor(std::shared_ptr<TProcessor> processor)
{
    m_processor = processor;
}

void JobServerRpcPublishObserver::Update(std::shared_ptr<messageservice::RpcPublishEvent> event)
{
    if (!event->GetThriftServer()->RegisterProcessor("JobService", m_processor)) {
        ERRLOG("JobService register processor failed.");
    }
}
}
