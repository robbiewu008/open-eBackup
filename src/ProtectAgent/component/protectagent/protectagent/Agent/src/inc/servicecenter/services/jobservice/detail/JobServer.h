/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 *
 * @file IJobService.h
 * @brief  Factory for Service Center
 * @version 1.1.0
 * @date 2021-08-31
 * @author caomin 00511255
 */
#ifndef JOB_SERVICE_H_
#define JOB_SERVICE_H_

#include <servicecenter/servicefactory/include/IService.h>
#include <services/jobservice/include/IJobServer.h>
#include <memory>

namespace jobservice {
class JobServer : public IJobServer {
public:
    virtual ~JobServer(){};
    virtual bool Initailize();
    virtual bool Uninitailize();
private:
    void RegisterRpcObserver();
};
}
#endif