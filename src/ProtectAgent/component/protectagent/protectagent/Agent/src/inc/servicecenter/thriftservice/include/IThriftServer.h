/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 *
 * @file IThriftServer.h
 * @brief  Base for thrift sever
 * @version 1.1.0
 * @date 2021-08-31
 * @author caomin 00511255
 */

#ifndef ITHRIFTSERVER_H_
#define ITHRIFTSERVER_H_

#include <thrift/TProcessor.h>

using namespace apache::thrift;

namespace thriftservice {
class IThriftServer {
public:
    virtual ~IThriftServer(){};
    virtual bool Start() = 0;
    virtual bool Stop() = 0;
    virtual bool RegisterProcessor(const std::string& name, std::shared_ptr<TProcessor> processor) = 0;
};
}
#endif