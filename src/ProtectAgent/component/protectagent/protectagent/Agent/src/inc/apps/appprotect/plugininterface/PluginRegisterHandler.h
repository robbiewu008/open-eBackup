/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 *
 * @file ExternalPluginParse.h
 * @brief  The implemention about ExternalPluginParse.h
 * @version 1.1.0
 * @date 2021-10-13
 * @author machenglin mwx1011302
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