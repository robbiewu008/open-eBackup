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
#ifndef _PLUGIN_REGISTER_HANDLER_H
#define _PLUGIN_REGISTER_HANDLER_H

#include <iostream>
#include "apps/appprotect/plugininterface/RegisterPluginService.h"
#include "common/CMpThread.h"
#include "servicecenter/messageservice/include/RpcPublishObserver.h"
#include "servicecenter/servicefactory/include/IService.h"

namespace AppProtect {
class PluginRegisterHandler :
    public RegisterPluginServiceIf,
    public messageservice::RpcPublishObserver,
    public servicecenter::IService {
public:
    PluginRegisterHandler();
    ~PluginRegisterHandler();
    EXTER_ATTACK virtual void RegisterPlugin(ActionResult& _return, const ApplicationPlugin& plugin); // 插件注册
    EXTER_ATTACK virtual void UnRegisterPlugin(ActionResult& _return, const ApplicationPlugin& plugin);
    bool Initailize() override;
    bool Uninitailize() override;
protected:
    void Update(std::shared_ptr<messageservice::RpcPublishEvent> event) override;
};
}
#endif // _PLUGIN_REGISTER_HANDLER_H