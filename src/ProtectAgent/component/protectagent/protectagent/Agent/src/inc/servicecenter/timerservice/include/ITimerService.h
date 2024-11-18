/*
* This file is a part of the open-eBackup project.
* This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this file, You can obtain one at
* http://mozilla.org/MPL/2.0/.
*
* Copyright (c) [2024] Huawei Technologies Co.,Ltd.
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
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