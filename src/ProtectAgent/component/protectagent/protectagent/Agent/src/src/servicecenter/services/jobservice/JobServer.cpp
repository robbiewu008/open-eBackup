#include "pluginfx/ExternalPluginManager.h"
#include "servicecenter/services/jobservice/detail/JobServerRpcPublishObserver.h"
#include "servicecenter/services/jobservice/detail/JobServiceFactory.h"
#include "servicecenter/servicefactory/include/ServiceFactory.h"
#include "servicecenter/services/jobservice/detail/JobServer.h"

namespace jobservice {
bool JobServer::Initailize()
{
    RegisterRpcObserver();
    return true;
}

bool JobServer::Uninitailize()
{
    return true;
}

void JobServer::RegisterRpcObserver()
{
    auto observer = JobServiceFactory::GetInstance()->CreateObserver();
    ExternalPluginManager::GetInstance().RegisterObserver(messageservice::EVENT_TYPE::RPC_PUBLISH_TYPE,
        std::dynamic_pointer_cast<messageservice::IObserver>(observer));
}
}