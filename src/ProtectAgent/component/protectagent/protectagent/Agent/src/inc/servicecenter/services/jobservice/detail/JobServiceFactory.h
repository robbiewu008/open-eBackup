#ifndef JOB_SERVICE_FACTORY_H_
#define JOB_SERVICE_FACTORY_H_

#include <thrift/TProcessor.h>
#include "servicecenter/messageservice/include/RpcPublishObserver.h"
#include "message/curlclient/DmeRestClient.h"

namespace jobservice {
class JobServiceFactory {
public:
    static std::shared_ptr<JobServiceFactory> GetInstance();
    std::shared_ptr<apache::thrift::TProcessor> CreateProcessor();
    std::shared_ptr<messageservice::RpcPublishObserver> CreateObserver();
};
}
#endif