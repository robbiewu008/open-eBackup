/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 *
 * @file ITimer.h
 * @brief  Base for Service ITimer
 * @version 1.1.0
 * @date 2021-11-29
 * @author caomin 00511255
 */

#ifndef ITIMER_SERIVICE_H_
#define ITIMER_SERIVICE_H_
#include "servicecenter/servicefactory/include/IService.h"
#include "servicecenter/timerservice/include/ITimer.h"

namespace timerservice {
class ITimerService : public servicecenter::IService {
public:
    virtual ~ITimerService() = default;
    virtual std::shared_ptr<ITimer> CreateTimer() = 0;
    virtual void DeleteTimer(std::shared_ptr<ITimer> timer) = 0;
};
}
#endif