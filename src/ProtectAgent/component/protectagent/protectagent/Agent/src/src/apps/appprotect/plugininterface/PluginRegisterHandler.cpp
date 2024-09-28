#include "apps/appprotect/plugininterface/PluginRegisterHandler.h"
#include "pluginfx/ExternalPluginRunState.h"
#include "pluginfx/ExternalPluginManager.h"
#include "common/CMpThread.h"
#include "common/Types.h"
#include "common/Log.h"
#include "common/Defines.h"

namespace AppProtect {
PluginRegisterHandler::PluginRegisterHandler()
{}

PluginRegisterHandler::~PluginRegisterHandler()
{}

EXTER_ATTACK void PluginRegisterHandler::RegisterPlugin(ActionResult& _return, const ApplicationPlugin& plugin)
{
    INFOLOG("Receive a register req.PluginName:%s ,endPoing:%s,processId:%s,port:%d.",
        plugin.name.c_str(),
        plugin.endPoint.c_str(),
        plugin.processId.c_str(),
        plugin.port);
    if (plugin.name.empty() || plugin.endPoint.empty() || plugin.processId.empty()) {
        COMMLOG(OS_LOG_ERROR, "Plugin registration parameters are incomplete.");
        _return.code = MP_FAILED;
        return;
    }
    _return.code = MP_SUCCESS;

    mp_int32 iRet = ExternalPluginManager::GetInstance().UpdatePluginInfo(plugin.name, plugin);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Update plugin processId failed. plugin name = %s", plugin.name.c_str());
        _return.code = MP_FAILED;
        return;
    }

    iRet = ExternalPluginManager::GetInstance().UpdatePluginStatus(plugin.name, EX_PLUGIN_STATUS::ISREGISTERED);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Update plugin status failed. plugin uniqueId = %s", plugin.name.c_str());
        _return.code = MP_FAILED;
        return;
    }

    INFOLOG("Update plugin(%s) status to ISREGISTERED.", plugin.name.c_str());
    return;
}

EXTER_ATTACK void PluginRegisterHandler::UnRegisterPlugin(ActionResult& _return, const ApplicationPlugin& plugin)
{
    mp_int32 iRet = ExternalPluginManager::GetInstance().UpdatePluginStatus(plugin.name, EX_PLUGIN_STATUS::CLOSED);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Failed to remove the plugin resource handle.");
        _return.code = MP_FAILED;
        return;
    }
    INFOLOG("Update plugin(%s) status to CLOSED.", plugin.name.c_str());
    _return.code = MP_SUCCESS;
}

void PluginRegisterHandler::Update(std::shared_ptr<messageservice::RpcPublishEvent> event)
{
    m_processorName = "RegisterPluginService";
    std::shared_ptr<servicecenter::IService> handler = shared_from_this();
    std::shared_ptr<PluginRegisterHandler> prHandler = std::dynamic_pointer_cast<PluginRegisterHandler>(handler);
    m_processor = std::make_shared<RegisterPluginServiceProcessor>(prHandler);

    if (event->GetThriftServer().get() == nullptr) {
        COMMLOG(OS_LOG_ERROR, "PluginRegisterHandler receives a null event");
        return;
    }
    if (!event->GetThriftServer()->RegisterProcessor(m_processorName, m_processor)) {
        COMMLOG(OS_LOG_ERROR, "PluginRegisterHandler register processor failed.");
        return;
    }
    INFOLOG("PluginRegisterHandler update success.");
}

bool PluginRegisterHandler::Initailize()
{
    std::shared_ptr<servicecenter::IService> handler = shared_from_this();
    std::shared_ptr<PluginRegisterHandler> prHandler = std::dynamic_pointer_cast<PluginRegisterHandler>(handler);
    ExternalPluginManager::GetInstance().RegisterObserver(messageservice::EVENT_TYPE::RPC_PUBLISH_TYPE, prHandler);
    return true;
}

bool PluginRegisterHandler::Uninitailize()
{
    return true;
}
}  // namespace AppProtect
