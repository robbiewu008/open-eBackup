/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 *
 * @file IJobServer.h
 * @brief  Factory for Service Center
 * @version 1.1.0
 * @date 2021-08-31
 * @author caomin 00511255
 */

#ifndef I_JOB_SERVER_H_
#define I_JOB_SERVER_H_
#include <servicecenter/servicefactory/include/IService.h>

namespace jobservice {
class IJobServer : public servicecenter::IService {
public:
    virtual ~IJobServer(){};
};
}
#endif