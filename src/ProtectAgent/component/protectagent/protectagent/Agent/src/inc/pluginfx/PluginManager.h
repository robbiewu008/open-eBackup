/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file PluginManager.h
 * @brief  The implemention about PluginManager.h
 * @version 1.0.0.0
 * @date 2020-08-01
 * @author wangguitao 00510599
 */
#ifndef AGENT_PLUGIN_MANAGER_IMPL_H
#define AGENT_PLUGIN_MANAGER_IMPL_H

#include <map>
#include "pluginfx/iplugin.h"
#include "common/CMpThread.h"
#include "common/FileSearcher.h"

typedef IPlugin* (*QUERY_INTERFACE)();

class CModule {
public:
    CModule(CFileSearcher& pSearcher, const mp_string& pszName, const mp_string& pszVersion);
    virtual ~CModule();

    IPlugin* GetPlugin();
    mp_string Name() const;
    mp_string Version() const;

    mp_int32 Load();
    mp_int32 Unload();

    mp_time LoadTime() const
    {
        return m_LoadTime;
    }

private:
    mp_time m_LoadTime;
    mp_handle_t m_hLib;
    IPlugin* m_Plugin;

    CFileSearcher* m_FileSearcher;
    mp_string m_Name;
    mp_string m_Version;

    static const mp_int32 MODULE_NUM_256 = 256;
};

class CPluginManager {
public:
    CPluginManager();
    virtual ~CPluginManager();

    mp_int32 Initialize(IPluginCallback& pCallback);
    mp_uint64 GetSCN();

    IPlugin* GetPlugin(const mp_string& pszPlg);
    IPlugin* GetPlugin(const mp_uint32& manageCmd);
    IPlugin* LoadPlugin(const mp_string& pszPlg);
    mp_void LoadPreLoadPlugins(const std::vector<mp_string>& vecPlgs);
    mp_void UnloadPlugin(const mp_string& pszPlg);
    mp_int32 Upgrade();

    mp_void SetPluginPath(const mp_string& pszPath)
    {
        m_FileSearcher->SetPath(pszPath);
    }

    const mp_string& GetPluginPath() const
    {
        return m_FileSearcher->GetPath();
    }

protected:
    mp_uint64 m_scn;
    // 查询锁，GetModule时使用，在更新m_Modules
    // 时必须加m_QueryX锁，保证查询时的数据一致性
    thread_lock_t m_QueryX;
    thread_lock_t m_LoadX;     // 更新锁
    thread_lock_t m_UpgradeX;  // 保证upgrade可以并发

    virtual mp_string GetModuleVersion(const mp_string& pszModule, mp_string& pszVer);
    CModule* Load(const mp_string& pszName);
    CModule* Load(const mp_string& pszName, const mp_string& szVersion);
    CModule* GetModule(const mp_string& pszName);
    CModule* GetModule(const mp_uint32& manageCmd);
    mp_void UnloadOldModules();
    mp_bool NeedReload(CModule& module);
    mp_bool InitModule(CModule& module);
    mp_bool Reload(CModule& pModule);

    mp_void Clear();

    typedef std::map<mp_string, CModule*> MODULES_MAP;
    MODULES_MAP m_Modules;
    MODULES_MAP m_OldModules;

    CFileSearcher* m_FileSearcher;
    IPluginCallback* m_Callback;

    typedef std::map<mp_uint32, CModule*> CMD_MODULES_MAP;
    CMD_MODULES_MAP m_CmdModulesMap;

private:
    mp_void PrintModules();
    mp_void PrintCmdModules();
    static const mp_int32 PLUGINMANAGER_NUM_256 = 256;
    static const mp_uchar PLUGINMANAGER_NUM_128 = 128;
    static const mp_uchar PLUGINMANAGER_NUM_64  = 64;
};

#endif  // _AGENT_PLUGIN_MANAGER_IMPL_H
