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