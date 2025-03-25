/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 *
 * @file IMessageService.h
 * @brief  Base for Message Service
 * @version 1.1.0
 * @date 2021-08-31
 * @author caomin 00511255
 */

#ifndef IMESSAGESERVICE_H_
#define IMESSAGESERVICE_H_

#include "servicecenter/servicefactory/include/IService.h"
#include "servicecenter/messageservice/include/ISubject.h"

namespace messageservice {
class IMessageService : public servicecenter::IService {
public:
    virtual ~IMessageService() = default;
    virtual std::shared_ptr<ISubject> GetSubject() = 0;
};
}
#endif