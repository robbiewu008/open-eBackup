#include "pluginfx/PluginManager.h"

#include <cstdlib>
#include <vector>
#include <iostream>
#include "common/Log.h"
#include "common/Utils.h"
#include "common/ErrorCode.h"
#include "securec.h"
using namespace std;

CPluginManager::CPluginManager()
{
    m_scn = 0;
    m_Callback = NULL;
    m_FileSearcher = NULL;
    CMpThread::InitLock(&m_QueryX);
    CMpThread::InitLock(&m_LoadX);
    CMpThread::InitLock(&m_UpgradeX);
}

CPluginManager::~CPluginManager()
{
    Clear();
    m_Callback = NULL;
    if (m_FileSearcher != NULL) {
        delete m_FileSearcher;
        m_FileSearcher = NULL;
    }
    CMpThread::DestroyLock(&m_QueryX);
    CMpThread::DestroyLock(&m_LoadX);
    CMpThread::DestroyLock(&m_UpgradeX);
}

mp_void CPluginManager::Clear()
{
    MODULES_MAP::iterator it = m_OldModules.begin();
    for (; it != m_OldModules.end(); ++it) {
        it->second->Unload();
        delete it->second;
    }

    it = m_Modules.begin();
    for (; it != m_Modules.end(); ++it) {
        it->second->Unload();
        delete it->second;
    }

    m_Modules.clear();
    m_OldModules.clear();
}

mp_int32 CPluginManager::Initialize(IPluginCallback& pCallback)
{
    m_Callback = &pCallback;
    // CodeDex误报，Memory Leak
    NEW_CATCH_RETURN_FAILED(m_FileSearcher, CFileSearcher);
    return MP_SUCCESS;
}

mp_uint64 CPluginManager::GetSCN()
{
    return m_scn;
}

IPlugin* CPluginManager::GetPlugin(const mp_string& pszPlg)
{
    COMMLOG(OS_LOG_DEBUG, "Begin get plugin, plg %s.", pszPlg.c_str());
    CModule* pModule = GetModule(pszPlg);
    if (pModule != NULL) {
        COMMLOG(OS_LOG_DEBUG, "Get plugin succ, this plugin has been loaded.");
        return pModule->GetPlugin();
    }

    COMMLOG(OS_LOG_DEBUG, "Get plugin failed.");
    return NULL;
}

IPlugin* CPluginManager::GetPlugin(const mp_uint32& manageCmd)
{
    COMMLOG(OS_LOG_DEBUG, "Begin get plugin, manage cmd %u.", manageCmd);
    CModule* pModule = GetModule(manageCmd);
    if (pModule != NULL) {
        return pModule->GetPlugin();
    }

    COMMLOG(OS_LOG_ERROR, "Get plugin failed.");
    return NULL;
}

IPlugin* CPluginManager::LoadPlugin(const mp_string& pszPlg)
{
    CThreadAutoLock tlock(&m_LoadX);
    COMMLOG(OS_LOG_DEBUG, "Begin load plugin, plg %s.", pszPlg.c_str());
    CModule* pModule = GetModule(pszPlg);
    if (pModule != NULL) {
        COMMLOG(OS_LOG_DEBUG, "Get plugin from from cache succ.");
        return pModule->GetPlugin();
    }

    pModule = Load(pszPlg);
    if (pModule == NULL) {
        COMMLOG(OS_LOG_ERROR, "Load plugin failed, plg %s.", pszPlg.c_str());
        return NULL;
    }

    ++m_scn;
    // 插件第一次加载需要执行初始化操作，会调用具体插件类的init方法
    mp_bool bRet = InitModule(*pModule);
    if (!bRet) {
        COMMLOG(OS_LOG_ERROR, "Init module failed.");
        // temporary plan: even if load failed, do not clean resource.
        return NULL;
    }

    // 真正变更时需要对m_QueryX加锁
    CThreadAutoLock qlock(&m_QueryX);
    ++m_scn;
    m_Modules[pModule->Name()] = pModule;
    COMMLOG(OS_LOG_DEBUG, "Load plugin succ, scn %d, module name %s.", m_scn,
            pModule->Name().c_str());
    return pModule->GetPlugin();
}

mp_void CPluginManager::LoadPreLoadPlugins(const vector<mp_string>& vecPlgs)
{
    COMMLOG(OS_LOG_DEBUG, "Begin load and init preload plugins.");
    for (auto iter = vecPlgs.begin(); iter != vecPlgs.end(); ++iter) {
        IPlugin* plg = LoadPlugin(iter->c_str());
        if (plg == NULL) {
            COMMLOG(OS_LOG_ERROR, "Load pre load plugin failed, plg %s.", iter->c_str());
            // temporary plan: even if load failed, do not return failure.
        }
    }

    COMMLOG(OS_LOG_DEBUG, "Load and init preload plugins succ.");
}

mp_void CPluginManager::UnloadPlugin(const mp_string& pszPlg)
{
    CModule* pModule = NULL;
    do {
        CThreadAutoLock tlock(&m_LoadX);
        pModule = GetModule(pszPlg);
        if (pModule == NULL) {
            COMMLOG(OS_LOG_INFO, "Unload plugin '%s' failed, the plugin has not been loaded.",
                    pszPlg.c_str());
            return;
        }

        if (!m_Callback->CanUnload(*pModule->GetPlugin())) {
            COMMLOG(OS_LOG_INFO, "Unload plugin '%s' failed, the plugin can not be unload now.",
                    pszPlg.c_str());
            return;
        }

        do {
            CThreadAutoLock qlock(&m_QueryX);
            ++m_scn;
            m_Modules.erase(pModule->Name());
        } while (0);
    } while (0);

    pModule->Unload();
    delete pModule;
    COMMLOG(OS_LOG_INFO, "Unload plugin '%s' successfully.", pszPlg.c_str());
}

mp_int32 CPluginManager::Upgrade()
{
    mp_int32 iRet = 0;
    vector<CModule*> modules(PLUGINMANAGER_NUM_256);

    // 这里的锁仅仅是为了保证upgrade方法是线程安全的。
    CThreadAutoLock tlock(&m_UpgradeX);
    UnloadOldModules();

    do {
        // 首先将当前加载的库备份到本地vector中
        // 然后再逐个处理，这样可以保证m_LoadX的加锁时间最短
        // 这里主要是减少同load_plugin的锁冲突
        CThreadAutoLock ldLock(&m_LoadX);
        MODULES_MAP::iterator it = m_Modules.begin();
        for (; it != m_Modules.end(); ++it) {
            modules.push_back(it->second);
        }
    } while (0);

    // 依次检查是否需要重新加载
    for (mp_size i = 0; i < modules.size(); ++i) {
        if (!NeedReload(*modules[i])) {
            continue;
        }

        Reload(*modules[i]);
        ++iRet;
    }

    return iRet;
}

mp_bool CPluginManager::Reload(CModule& pModule)
{
    CThreadAutoLock ldLock(&m_LoadX);
    CModule* pNewModule = Load(pModule.Name());
    if (pNewModule == NULL) {
        return MP_FALSE;
    }

    (mp_void) InitModule(*pNewModule);

    m_Callback->OnUpgraded(*pModule.GetPlugin(), *pNewModule->GetPlugin());

    CThreadAutoLock qryLock(&m_QueryX);
    ++m_scn;

    // 这里必须首先删除原来的，因为Map的可以是用const char*做key值
    // 而且这个Key指针是放在second的对象中存放，如果不删除，则在执行
    // 下一行代码时，这个Key值不会被替换，仍然使用旧对象中的指针，这样
    // 当旧对象被删除后，这个Key就成为一个“野指针”，再访问这个Map时
    // 很容易造成非法内存访问
    m_Modules.erase(pModule.Name());
    m_Modules[pNewModule->Name()] = pNewModule;

    // 将旧插件放到另一MAP中，等待后续自动卸载
    m_OldModules[pModule.Name()] = &pModule;

    return MP_TRUE;
}

mp_bool CPluginManager::InitModule(CModule& module)
{
    if (!module.GetPlugin()) {
        return MP_FALSE;
    }

    vector<mp_uint32> cmds;
    mp_int32 iRet = module.GetPlugin()->Initialize(cmds);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Initialize plugin('%s') failed, iRet %d.", module.Name().c_str(), iRet);
        return MP_FALSE;
    }

    for (vector<mp_uint32>::iterator iter = cmds.begin(); iter != cmds.end(); ++iter) {
        m_CmdModulesMap[*iter] = &module;
    }
    PrintCmdModules();
    COMMLOG(OS_LOG_INFO, "Initialize plugin '%s'(%s) completed.", module.Name().c_str(), module.Version().c_str());
    return MP_TRUE;
}

mp_bool CPluginManager::NeedReload(CModule& module)
{
    mp_string szVersion = "";
    szVersion = GetModuleVersion(module.Name(), szVersion);
    // 没有版本定义不需要升级
    if (szVersion.empty()) {
        return MP_FALSE;
    }

    // 版本没有变化不需要升级
    if (szVersion == module.Version()) {
        return MP_FALSE;
    }

    // 动态库仍存在旧版本没有卸载的情况下，不进行版本更新
    // 本函数和m_OldModules只有在upgrade过程中会被访问，所以不需要加锁
    MODULES_MAP::iterator it = m_OldModules.find(module.Name());
    if (it != m_OldModules.end()) {
        COMMLOG(OS_LOG_INFO,
                "Plugin(%s)'s old version(%s) still be running, new version %s reload later.",
                module.Name().c_str(),
                it->second->Version().c_str(),
                szVersion.c_str());
        return MP_FALSE;
    }

    return MP_TRUE;
}

mp_void CPluginManager::UnloadOldModules()
{
    CModule* pModule = NULL;

    COMMLOG(OS_LOG_DEBUG, "Begin unload old modules.");
    MODULES_MAP::iterator iter = m_OldModules.begin();
    while (iter != m_OldModules.end()) {
        pModule = iter->second;
        if (!m_Callback->CanUnload(*pModule->GetPlugin())) {
            COMMLOG(OS_LOG_DEBUG, "Plugin '%s' can not unload.", pModule->Name().c_str());
            ++iter;
            continue;
        }

        pModule->Unload();
        m_OldModules.erase(iter++);
        delete pModule;
    }
    COMMLOG(OS_LOG_DEBUG, "Unload old modules succ.");
}

CModule* CPluginManager::Load(const mp_string& pszName)
{
    mp_string szVersion;
    COMMLOG(OS_LOG_DEBUG, "Begin load module, name %s.", pszName.c_str());
    szVersion = GetModuleVersion(pszName, szVersion);
    if (szVersion.empty()) {
        COMMLOG(OS_LOG_DEBUG, "Can't get moudle version, name %s.", pszName.c_str());
        return Load(pszName, "");
    }

    COMMLOG(OS_LOG_DEBUG, "Load moudle version succ, name %s, version %s.",
            pszName.c_str(), szVersion.c_str());
    return Load(pszName, szVersion);
}

CModule* CPluginManager::Load(const mp_string& pszName, const mp_string& pszVer)
{
    CModule* pModule = NULL;
    try {
        // CodeDex误报，Memory Leak
        pModule = new CModule(*m_FileSearcher, pszName, pszVer);
    } catch (...) {
        COMMLOG(OS_LOG_ERROR, "New CModule failed.");
        pModule = NULL;
    }

    if (!pModule) {
        return NULL;
    }
    mp_int32 iRet = pModule->Load();
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Load plugin '%s' failed, iRet %d.",
                pszName.c_str(), iRet);
        pModule->Unload();
        delete pModule;
        return NULL;
    }

    return pModule;
}

mp_string CPluginManager::GetModuleVersion(const mp_string& pszModule, mp_string& pszVer)
{
    return m_Callback->GetReleaseVersion(pszModule, pszVer);
}

mp_void CPluginManager::PrintModules()
{
    MODULES_MAP::iterator iter;
    COMMLOG(OS_LOG_DEBUG, "Print cached moudles.");
    for (iter = m_Modules.begin(); iter != m_Modules.end(); ++iter) {
    }
}

mp_void CPluginManager::PrintCmdModules()
{
    CMD_MODULES_MAP::iterator iter;
    COMMLOG(OS_LOG_DEBUG, "Print cached cmd moudles.");
    for (iter = m_CmdModulesMap.begin(); iter != m_CmdModulesMap.end(); ++iter) {
        COMMLOG(OS_LOG_DEBUG, "cmd:%d;moduleName:%s;pluginName:%s",
            iter->first, iter->second->Name().c_str(), iter->second->GetPlugin()->GetName().c_str());
    }
}

CModule* CPluginManager::GetModule(const mp_string& pszName)
{
    CThreadAutoLock tlock(&m_QueryX);
    PrintModules();
    MODULES_MAP::iterator it = m_Modules.find(pszName);
    if (m_Modules.end() == it) {
        return NULL;
    }
    return it->second;
}

CModule* CPluginManager::GetModule(const mp_uint32& manageCmd)
{
    CThreadAutoLock tlock(&m_QueryX);
    CMD_MODULES_MAP::iterator it = m_CmdModulesMap.find(manageCmd);
    if (m_CmdModulesMap.end() == it) {
        return NULL;
    }
    return it->second;
}

CModule::CModule(CFileSearcher& pSearcher, const mp_string& pszName, const mp_string& pszVersion)
{
    m_Name = pszName;
    if (!pszVersion.empty()) {
        m_Version = pszVersion;
    }
    m_FileSearcher = &pSearcher;
    m_hLib = NULL;
    m_Plugin = NULL;
    time(&m_LoadTime);
}

CModule::~CModule()
{
    Unload();
    m_hLib = NULL;
    m_Plugin = NULL;
}

IPlugin* CModule::GetPlugin()
{
    return m_Plugin;
}

mp_string CModule::Name() const
{
    return m_Name;
}

mp_string CModule::Version() const
{
    return m_Version;
}

mp_int32 CModule::Load()
{
    COMMLOG(OS_LOG_DEBUG, "Begin load plugin.");

    if (m_hLib != NULL) {
        return ERROR_COMMON_DLL_LOAD_FAILED;
    }

    mp_char szName[MODULE_NUM_256] = {0};
    if (m_Version.empty()) {
        CHECK_FAIL(snprintf_s(szName, sizeof(szName), sizeof(szName) - 1, "%s%s", m_Name.c_str(),  LIB_SUFFIX.c_str()));
    } else {
        CHECK_FAIL(snprintf_s(szName, sizeof(szName), sizeof(szName) - 1, "%s-%s%s", m_Name.c_str(),
            m_Version.c_str(), LIB_SUFFIX.c_str()));
    }

    mp_string strPath;
    mp_string strName = szName;
    if (!m_FileSearcher->Search(strName, strPath)) {
        COMMLOG(OS_LOG_ERROR, "Not found file '%s' from path '%s'.", szName, m_FileSearcher->GetPath().c_str());
        return ERROR_COMMON_DLL_LOAD_FAILED;
    }

    COMMLOG(OS_LOG_DEBUG, "Begin load library, plugin='%s'(%s), library='%s'.",
            m_Name.c_str(), m_Version.c_str(), strPath.c_str());
    mp_handle_t hLib = DlibOpenEx(strPath, MP_TRUE);
    if (hLib == NULL) {
        mp_char szErr[MODULE_NUM_256] = {0};
        COMMLOG(OS_LOG_ERROR, "Load library '%s' failed: %s.", strPath.c_str(), DlibError(szErr, sizeof(szErr)));
        return ERROR_COMMON_DLL_LOAD_FAILED;
    }
    COMMLOG(OS_LOG_DEBUG, "Load library '%s' succ.", strPath.c_str());

    COMMLOG(OS_LOG_DEBUG, "Begin get 'QueryInterface' in library '%s'.", strPath.c_str());
    QUERY_INTERFACE fp = (QUERY_INTERFACE)DlibDlsym(hLib, "QueryInterface");
    if (fp == NULL) {
        DlibClose(hLib);
        COMMLOG(OS_LOG_ERROR, "No symbol 'QueryInterface' in library '%s'.", strPath.c_str());
        return ERROR_COMMON_DLL_LOAD_FAILED;
    }
    COMMLOG(OS_LOG_DEBUG, "Get 'QueryInterface' in library '%s' succ.", strPath.c_str());

    m_hLib = hLib;
    m_Plugin = fp();
    m_Plugin->SetOption("PLUGIN_NAME", m_Name);
    m_Plugin->SetOption("PLUGIN_VERSION", m_Version);
    COMMLOG(OS_LOG_DEBUG, "Load plugin succ, plugin='%s'(%s), library='%s'.",
            m_Name.c_str(), m_Version.c_str(), strPath.c_str());
    return MP_SUCCESS;
}

mp_int32 CModule::Unload()
{
    if (m_Plugin != NULL) {
        // CodeDex误报，Dead Code
        mp_int32 iRet = m_Plugin->Destroy();
        if (iRet != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "Unload plugin '%s' failed, iRet %d.", m_Name.c_str(), iRet);
            return iRet;
        }
    }

    DlibClose(m_hLib);

    m_hLib = NULL;
    m_Plugin = NULL;

    return MP_SUCCESS;
}
