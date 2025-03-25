#ifndef _EXTERNAL_PLUGIN_MANAGER_H
#define _EXTERNAL_PLUGIN_MANAGER_H

#include <iostream>
#include <map>
#include <vector>
#include <memory>
#include "common/Types.h"
#include "common/Log.h"
#include "common/Defines.h"
#include "common/CMpThread.h"
#include "message/rest/message_process.h"
#include "pluginfx/ExternalPluginParse.h"
#include "apps/appprotect/plugininterface/PluginRegisterHandler.h"
#include "servicecenter/thriftservice/detail/ThriftService.h"
#include "servicecenter/servicefactory/include/ServiceFactory.h"
#include "servicecenter/thriftservice/include/IThriftService.h"
#include "apps/appprotect/plugininterface/ApplicationService.h"
#include "pluginfx/ExternalPlugin.h"
#include "servicecenter/messageservice/detail/MessageService.h"
#include "servicecenter/messageservice/include/IObserver.h"
#include "pluginfx/ExternalPluginAlarmMng.h"

class ExternalPluginManager {
public:
    using ModifyPluginCounter = std::function<void(std::shared_ptr<ExternalPlugin>)>;
    enum class PluginRegisterdState {
        Checking,
        TimeOut,
        Failed,
        Success
    };
public:
    static ExternalPluginManager& GetInstance()
    {
        static ExternalPluginManager m_Instance;
        return m_Instance;
    }
    ~ExternalPluginManager();
    mp_int32 Init();
    std::shared_ptr<ExternalPlugin> GetPlugin(const mp_string &appType);
    void ReleasePlugin(const mp_string &appType);
    std::shared_ptr<ExternalPlugin> GetPluginByRest(const mp_string &appType);
    void ReleasePluginByRest(const mp_string &appType);
    mp_int32 UpdatePluginStatus(const mp_string &pluginName, EX_PLUGIN_STATUS status);
    mp_int32 UpdatePluginInfo(const mp_string &pluginName, const ApplicationPlugin &pluginInfo);
    std::shared_ptr<ExternalPluginParse> GetParseManager()
    {
        return m_pluginParseMng;
    }
    bool GetMonRunStatusFlag();
    void RegisterObserver(messageservice::EVENT_TYPE type, std::shared_ptr<messageservice::IObserver> observer);
    mp_int32 AddHostNameToFile(const mp_string& hostName);
#ifndef WIN32
    static void StopAllPlugsHandle(mp_int32 signum);
#endif
    void StartMonitor();
    mp_void StopPluginEx(const mp_string &appType);
    mp_string GetStartUser(const mp_string &pluginName)
    {
        return m_pluginParseMng->GetStartUser(pluginName);
    }

    mp_int32 GetPluginNameByAppType(const mp_string &appType, mp_string &pluginName)
    {
        return m_pluginParseMng->GetPluginNameByAppType(appType, pluginName);
    }

    // query plugin resource
    mp_int32 QueryPluginResource(const mp_string &strAppType, CRequestMsg &requestMsg, CResponseMsg &responseMsg);
    // query plugin detail
    mp_int32 QueryPluginDetail(const mp_string &strAppType, CRequestMsg &requestMsg, CResponseMsg &responseMsg);
    // query plugin detail2
    mp_int32 QueryPluginDetailV2(const mp_string &strAppType, CRequestMsg &requestMsg, CResponseMsg &responseMsg);
    // query plugin detail
    mp_int32 CheckPlugin(const mp_string &strAppType, CRequestMsg &requestMsg, CResponseMsg &responseMsg);
    // async query plugin resource
    mp_int32 PluginAsyncListApplicationResource(const mp_string &strAppType, CRequestMsg &requestMsg,
        CResponseMsg &responseMsg);
    // plugin finalize clear
    mp_int32 PluginFinalizeClear(const mp_string &strAppType, CRequestMsg &requestMsg, CResponseMsg &responseMsg);
    
    mp_int32 QueryRemoteCluster(const mp_string &strAppType, CRequestMsg& requestMsg, CResponseMsg& responseMsg);
    // query plugin config
    mp_int32 QueryPluginConfig(const mp_string &strAppType, CRequestMsg& requestMsg, CResponseMsg& responseMsg);
    // remove protect
    mp_int32 RemoveProtect(const mp_string &strAppType, CRequestMsg& requestMsg, CResponseMsg& responseMsg);

private:
    ExternalPluginManager();
    mp_int32 ReloadPlugin(std::shared_ptr<ExternalPlugin> plugin);
    bool PluginExisted(const mp_string &pluginName);
    bool IsPluginFolderExist(const std::string &pluginName);
    std::shared_ptr<ExternalPlugin> GetPluginFromPluginMap(const mp_string &pluginName);
    mp_void RemovePlugin(const std::string &pluginName);
    mp_void MonPluginStatus();
    void InitRpcObserver();
    mp_void StopAllPlugs();
    mp_void KillExternalPlugs();
    mp_int32 CheckAndKillPlug(const mp_string& pid, const mp_string& pluginPidPath);
    
    /*
    StartPlugin:启动指定类型的插件
    返回值：返回启动的插件的唯一ID，此ID插件在注册时会带上。如果字符串为空，则启动失败
    */
    mp_int32 StartPlugin(const mp_string &pluginName);
    mp_void QueryPluginState(const mp_string &strAppType, Json::Value &jValueRsp);
    mp_int32 InvokingPlugins(const Json::Value &requestParam, CResponseMsg &responseMsg,
        const std::shared_ptr<AppProtect::ApplicationServiceConcurrentClient> &appServiceClient);
    mp_int32 AsyncListApplicationResource(const Json::Value &requestParam, CResponseMsg &responseMsg,
        const std::shared_ptr<AppProtect::ApplicationServiceConcurrentClient> &appServiceClient, const mp_string id);
    mp_int32 FinalizeClear(const Json::Value &requestParam, CResponseMsg &responseMsg,
        const std::shared_ptr<AppProtect::ApplicationServiceConcurrentClient> &appServiceClient);
    mp_int32 WriteThriftServerPort(const mp_int32 &thriftServerPort);
    mp_int32 QueryPluginDetailV1(const Json::Value &requestParam, CResponseMsg &responseMsg,
        const std::shared_ptr<AppProtect::ApplicationServiceConcurrentClient> &appServiceClient);
    bool CheckIfCNExist(const mp_string &hostName);
    mp_int32 CheckIfCNExistInHostsFile();
#ifdef WIN32
    static DWORD WINAPI MonPluginStatusThread(mp_void *pThis);
#else
    static mp_void* MonPluginStatusThread(mp_void *pThis);
#endif
    std::shared_ptr<AppProtect::ApplicationServiceConcurrentClient> GetApplicationServiceClient(
        const std::shared_ptr<thriftservice::IThriftClient>& pThriftClient);

    void MonitorPluginThread();

    void PauseMonitor();
    void StopMonitor();
    bool WaitMonitor();
    void MonitorPlugin();
    void MonitorPluginCheck(std::vector<mp_string> &erasePlugins,
        std::vector<std::shared_ptr<ExternalPlugin>> &restartPlugins,
        std::vector<std::pair<mp_string, mp_string>> &reloadedPlugins);
    ExternalPluginManager::PluginRegisterdState HandlePlguinWait(
        const mp_string &pluginName, std::shared_ptr<ExternalPlugin> plugin);
    mp_int32 GetPluginName(const mp_string &appType, mp_string &plugin);
    std::shared_ptr<ExternalPlugin> GetPluginImpl(const mp_string &appType, const ModifyPluginCounter& modiftCounter);
    void ReleasePluginImpl(const mp_string &appType, const ModifyPluginCounter& modiftCounter);
    std::shared_ptr<AppProtect::ApplicationServiceConcurrentClient> GetServiceClient(const mp_string &strAppType);

    mp_int32 InitThriftServer();
    EXTER_ATTACK bool InitThriftServerPort();
    void InitMonitorThread();
private:
    std::shared_ptr<ExternalPluginParse> m_pluginParseMng;       // 解析外部插件管理
    std::map<mp_string, std::shared_ptr<ExternalPlugin>> m_pluginMap; // 键值:插件名称
    std::mutex m_pluginMutex;
    thread_id_t m_monRunStatusTid;  // 定时监控插件的运行状态
    volatile bool m_monRunStatusFlag; // 是否要定时监控插件运行状态
    std::shared_ptr<thriftservice::IThriftServer> m_thriftServer; // 发布RPC接口服务端
    mp_int32 m_thriftServerPort = 0;
    std::shared_ptr<messageservice::ISubject> m_subject;
    ExternalPluginAlarmMng m_alarmManage;
    std::mutex m_lock;
    std::condition_variable m_monitorCond;
    bool m_startMonitor {false};
    bool m_stop {false};
    bool m_ssl {true};
    std::shared_ptr<std::thread> m_monitorTh;
};

#endif // _EXTERNAL_PLUGIN_MANAGER_H