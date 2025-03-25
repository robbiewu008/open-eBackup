/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 *
 * @file JobServiceFactory.h
 * @brief  Factory for Job Service
 * @version 1.1.0
 * @date 2021-11-22
 * @author caomin 00511255
 */

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