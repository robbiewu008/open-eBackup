/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 *
 * @file MessageService.h
 * @brief  implement for MessageService
 * @version 1.1.0
 * @date 2021-08-31
 * @author caomin 00511255
 */

#ifndef MESSAGESERVICE_H_
#define MESSAGESERVICE_H_

#include "servicecenter/messageservice/include/IMessageService.h"

namespace messageservice {
namespace detail {
class MessageService : public IMessageService {
public:
    virtual std::shared_ptr<ISubject> GetSubject();
    virtual bool Initailize();
    virtual bool Uninitailize();
};
}
}
#endif