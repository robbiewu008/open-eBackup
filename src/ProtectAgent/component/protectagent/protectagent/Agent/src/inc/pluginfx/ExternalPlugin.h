/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 *
 * @file PluginRunController.h
 * @brief  The implemention about ExternalPlugin.h
 * @version 1.0.0.0
 * @date 2021-10-13
 * @author jwx966562
 */
#ifndef _EXTERNAL_PLUGIN_H
#define _EXTERNAL_PLUGIN_H
#include <vector>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <chrono>
#include "pluginfx/ExternalPluginRunState.h"
#include "pluginfx/iplugin.h"
#include "apps/appprotect/plugininterface/ApplicationProtectPlugin_types.h"
#include "apps/appprotect/plugininterface/PluginService.h"
#include "apps/appprotect/plugininterface/ProtectService.h"
#include "apps/appprotect/plugininterface/ApplicationService.h"
#include "common/CMpThread.h"
#include "servicecenter/thriftservice/include/IThriftClient.h"

namespace {
    const mp_int32 START_EXTERNAL_PLUGIN_TIMEOUT = 60;
}

class ExternalPlugin : public IPlugin {
public:
    ExternalPlugin(const std::string &pluginName, const mp_string &startUser, bool ssl, mp_int32 serverPort = 59570);
    ExternalPlugin(const ExternalPlugin& plugin);
    ~ExternalPlugin();
    /* 继承接口 */
    mp_int32 Initialize(std::vector<mp_uint32> &cmds) override
    {
        return MP_SUCCESS;
    }
    mp_int32 Destroy() override
    {
        return MP_SUCCESS;
    }
    mp_void SetOption(mp_string pszName, mp_string pvVal) override
    {}
    mp_bool GetOption(mp_string pszName, mp_string &pvVal) override
    {
        return MP_TRUE;
    }
    mp_void *CreateObject(mp_string pszName) override
    {
        return nullptr;
    }
    mp_int32 GetClasses(IPlugin::DYN_CLASS &pClasses, mp_int32 sz) override
    {
        return MP_SUCCESS;
    }
    mp_string GetName() override
    {
        return m_pluginInfo.name;
    }
    mp_string GetVersion() override
    {
        return "";
    }
    std::size_t GetSCN() override
    {
        return 0;
    }
    ApplicationPlugin GetPluginInfo()
    {
        return m_pluginInfo;
    }
    void SetPluginInfo(const ApplicationPlugin &pluginInfo);
    mp_void SavePid(const ApplicationPlugin &pluginInfo);

    std::shared_ptr<thriftservice::IThriftClient> GetPluginClient(
        thriftservice::ClientSocketOpt opt = thriftservice::ClientSocketOpt());

    std::shared_ptr<ExternalPlugin> Clone();

    /* 对外接口 */
    mp_uint32 StopPlugin();
    mp_uint32 StartPlugin();
    mp_uint32 PluginRegistered();
    mp_uint32 PluginUnregistered();
    EX_PLUGIN_STATUS GetRunningStatus()
    {
        return m_runningStatus;
    }
    mp_void SetProcessId(const mp_string &processId)
    {
        m_pluginInfo.processId = processId;
    }
    bool GetRconnectStatus();
    void AddTaskCounter();
    void ReduceTaskCounter();
    void AddRestCounter();
    void ReduceRestCounter();
    /* 状态机接口 */
    mp_uint32 ExecStopPlugin();
    EXTER_ATTACK mp_uint32 ExecStartPlugin();
    mp_uint32 KillProcess();
    void ChangeStatus(EX_PLUGIN_STATUS status);
    bool IsPluginRunning();
    bool IsWaitPluginRegister();
    bool CheckPluginRegister(bool &timeout);
    bool IsPluginProcessExist();
    bool IsPluginResponding();
    bool IsReload()
    {
        return m_reloading;
    }
    /* 当前插件没有任务和rest请求是否已经超过超时时间 */
    bool IsNoUseTimeout();
    mp_void SetCgroupInfo(mp_int32 cpuLimit, mp_int32 memoryLimit, mp_int32 blkioWeight)
    {
        m_cpuLimit = cpuLimit;
        m_memoryLimit = memoryLimit;
        m_blkioWeight = blkioWeight;
    }
private:
    mp_int32 m_cpuLimit = -1;            // 用于设置 cpu.cfs_quotas_us
    mp_int32 m_memoryLimit = -1;     // 用于设置 memory.limit_in_bytes
    mp_int32 m_blkioWeight = -1;      // 用于设置 blkio.weight
    std::vector<mp_string> m_jobList;  // 插件执行的任务ID
    EX_PLUGIN_STATUS m_runningStatus;  // 插件运行状态
    std::mutex m_statusMutex;          // 锁运行状态
    ApplicationPlugin m_pluginInfo;    // 插件信息
    mp_int32 m_thriftServerPort = 0;
    std::unique_ptr<ExPluginStateBase> m_currentState;
    mp_string m_startUser;
    std::atomic<bool> m_taskCounter { false }; // for task lock
    std::atomic<mp_int32> m_restCounter {0};  // for rest 2 plguin lock
    mp_int32 m_waitTimeout {START_EXTERNAL_PLUGIN_TIMEOUT};
    std::condition_variable m_waitCond;
    bool m_ssl {false};
    bool m_reloading {false};
    mp_int32 m_heartbeatNum = 0;
    mp_int32 m_noRespondNum = 0;

    /* 当前插件没有任务和rest请求时开始计时时间点 */
    std::chrono::time_point<std::chrono::steady_clock> m_noUsePoint = std::chrono::steady_clock::now();
private:
    void SetThriftClientConfig(thriftservice::ClientSocketOpt& opt);
    bool IsUsing();
    mp_string GetExternalScriptPath(const mp_string &pluginName);
    mp_uint32 GenerateParam(mp_string &params);  // 获取执行脚本的参数
    std::shared_ptr<thriftservice::IThriftClient> GetThriftClientFromService(
        const thriftservice::ClientSocketOpt& opt);
};

#endif