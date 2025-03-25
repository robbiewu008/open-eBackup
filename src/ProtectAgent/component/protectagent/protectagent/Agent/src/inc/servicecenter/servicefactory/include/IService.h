/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 *
 * @file IService.h
 * @brief  Base for Service Center
 * @version 1.1.0
 * @date 2021-08-31
 * @author caomin 00511255
 */

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