/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 *
 * @file IObserver.h
 * @brief  Base for Observer
 * @version 1.1.0
 * @date 2021-08-31
 * @author caomin 00511255
 */

#ifndef IOBSERVER_H_
#define IOBSERVER_H_

#include <memory>

namespace messageservice {
class IEvent;
class IObserver {
public:
    virtual ~IObserver() = default;
    virtual void Update(std::shared_ptr<IEvent> event) = 0;
};
}

#endif