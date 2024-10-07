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
#ifndef AGENT_ISERVICECENTER_H
#define AGENT_ISERVICECENTER_H

#include <memory>
#include <string>
#include <functional>
#include "IService.h"
#ifdef WIN32
#include "common/Thread.h"
#endif

namespace servicecenter {
using ServiceCreator = std::function<std::shared_ptr<IService>()>;

class IServiceCenter : public std::enable_shared_from_this<IServiceCenter> {
public:
    virtual ~IServiceCenter(){};
    virtual std::shared_ptr<IService> GetService(const std::string &name) = 0;
    virtual bool Initailize() = 0;
    virtual bool Register(const std::string &name, ServiceCreator creator) = 0;
    virtual bool Unregister(const std::string &name) = 0;

#ifdef WIN32
    AGENT_EXPORT static std::shared_ptr<IServiceCenter> GetInstance();
#else
    static std::shared_ptr<IServiceCenter> GetInstance();
#endif
protected:
    static std::shared_ptr<IServiceCenter> gInstance;
};
}

#endif