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
#include "agent/TaskPool.h"
#include "common/Log.h"
#include "common/File.h"
#include "common/Utils.h"
#include "common/Path.h"
#include "securec.h"
using namespace std;

mp_int32 TaskPool::m_workerThreadCount = 30;
namespace {
constexpr mp_uint32 TASKPOOL_MAX_NUM = 50;
}

TaskPool::TaskPool()
{
    m_pDispatchWorker = NULL;
    m_plugCfgParse = NULL;
    m_plugMgr = NULL;
#ifdef WIN32
    m_pVssWorker = NULL;
#endif
}

TaskPool::~TaskPool()
{
    Exit();
}

/* ------------------------------------------------------------
Description  : 初始化线程池
               如下方法中调用顺序不能随便调整，有依赖关系
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    : yangwenjun 00275736
------------------------------------------------------------- */
EXTER_ATTACK mp_int32 TaskPool::Init()
{
    mp_int32 iRet;
    COMMLOG(OS_LOG_DEBUG, "Begin init task pool.");
    // parse task pool worker thread count
    iRet = CConfigXmlParser::GetInstance().GetValueInt32(
        CFG_SYSTEM_SECTION, CFG_TASKPOOL_WORKER_COUNT, m_workerThreadCount);
    if (iRet != MP_SUCCESS || m_workerThreadCount > TASKPOOL_MAX_NUM) {
        COMMLOG(OS_LOG_WARN, "Obtain worker thread count of task pool from config file failed, "
            "will use default value %d.", m_workerThreadCount);
        m_workerThreadCount = TASKPOOL_MAX_NUM;
    }

    iRet = CreatePlgConfParse();
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Create plg conf parse failed.");
        return iRet;
    }

    iRet = CreatePlugMgr();
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Create plg manager failed.");
        return iRet;
    }

    iRet = CreateWorkers();
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Create task workers failed.");
        return iRet;
    }

#ifdef WIN32
    iRet = CreateVssWorker();
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Create vss workers failed.");
        return iRet;
    }
#endif

    iRet = CreateDispatchWorker();
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Create dispatch worker failed.");
        return iRet;
    }

    COMMLOG(OS_LOG_DEBUG, "End init task pool.");

    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Description  : 退出线程池
Create By    : yangwenjun 00275736
------------------------------------------------------------- */
mp_void TaskPool::Exit()
{
    TaskWorker* pWorker = NULL;

    for (mp_uint32 i = 0; i < m_pWorkers.size(); i++) {
        pWorker = m_pWorkers[i];
        if (pWorker != NULL) {
            pWorker->Exit();
            delete pWorker;
        }
        m_pWorkers[i] = NULL;
    }

    if (m_pDispatchWorker != NULL) {
        delete m_pDispatchWorker;
        m_pDispatchWorker = NULL;
    }

    if (m_plugCfgParse != NULL) {
        delete m_plugCfgParse;
        m_plugCfgParse = NULL;
    }

    if (m_plugMgr != NULL) {
        delete m_plugMgr;
        m_plugMgr = NULL;
    }

#ifdef WIN32
    if (m_pVssWorker != NULL) {
        m_pVssWorker->Exit();
        delete m_pVssWorker;
        m_pVssWorker = NULL;
    }
#endif
}

/* ------------------------------------------------------------
Description  : 创建task worker线程
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    : yangwenjun 00275736
------------------------------------------------------------- */
mp_int32 TaskPool::CreateWorkers()
{
    mp_int32 iRet;

    COMMLOG(OS_LOG_DEBUG, "Begin create task workers.");
    for (mp_uint32 i = 0; i < m_workerThreadCount; i++) {
        TaskWorker *pWorker = new TaskWorker();
        m_pWorkers.push_back(pWorker);
        iRet = m_pWorkers[i]->Init(*m_plugCfgParse, *m_plugMgr);
        if (iRet != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "Init task worker[%d] failed, iRet %d.", i, iRet);
            return iRet;
        }
        pWorker = NULL;
        COMMLOG(OS_LOG_INFO, "Init task worker[%d] succ.", i + 1);
    }

    COMMLOG(OS_LOG_DEBUG, "Create task workers succ.");
    return MP_SUCCESS;
}

#ifdef WIN32
/* ------------------------------------------------------------
Description  : 创建vss worker线程，仅windows下存在该线程
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    : yangwenjun 00275736
------------------------------------------------------------- */
mp_int32 TaskPool::CreateVssWorker()
{
    mp_int32 i = 0;
    mp_int32 iRet;

    COMMLOG(OS_LOG_DEBUG, "Begin create task vss worker.");
    NEW_CATCH_RETURN_FAILED(m_pVssWorker, TaskVssWorker);
    iRet = m_pVssWorker->Init(*m_plugCfgParse, *m_plugMgr);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Init task vss worker failed, iRet %d.", iRet);
        return iRet;
    }
    COMMLOG(OS_LOG_INFO, "Init task vss worker succ.");
    COMMLOG(OS_LOG_DEBUG, "Create task vss worker succ.");
    return MP_SUCCESS;
}
#endif  // WIN32

/* ------------------------------------------------------------
Description  : 创建dispatch worker线程
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    : yangwenjun 00275736
------------------------------------------------------------- */
mp_int32 TaskPool::CreateDispatchWorker()
{
    mp_int32 iRet;

    COMMLOG(OS_LOG_DEBUG, "Begin create task dispatch workers.");
    // CodeDex误报，Memory Leak
    NEW_CATCH_RETURN_FAILED(m_pDispatchWorker, TaskDispatchWorker);

#ifdef WIN32
    iRet = m_pDispatchWorker->Init(m_pWorkers[0], m_workerThreadCount, *m_pVssWorker);
#else
    iRet = m_pDispatchWorker->Init(m_pWorkers[0], m_workerThreadCount);
#endif
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Init task dispatch worker[0] failed, iRet %d.", iRet);
        return iRet;
    }
    COMMLOG(OS_LOG_INFO, "Init task dispatch worker succ.");
    COMMLOG(OS_LOG_DEBUG, "Create task dispatch workers succ.");
    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Description  : 创建插件配置文件解析对象
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    : yangwenjun 00275736
------------------------------------------------------------- */
mp_int32 TaskPool::CreatePlgConfParse()
{
    mp_int32 iRet;
    mp_string strPlgCfg;

    COMMLOG(OS_LOG_DEBUG, "Begin create plg conf parse.");
    // CodeDex误报，Memory Leak
    NEW_CATCH_RETURN_FAILED(m_plugCfgParse, PluginCfgParse);
    strPlgCfg = CPath::GetInstance().GetConfFilePath(AGENT_PLG_CONF);

    iRet = m_plugCfgParse->Init(strPlgCfg);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Init plugin conf parse failed, iRet %d.", iRet);
        return iRet;
    }
    m_plugCfgParse->PrintPluginDef();

    COMMLOG(OS_LOG_DEBUG, "Create plg conf parse succ.");
    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Description  : 创建插件管理对象
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    : yangwenjun 00275736
------------------------------------------------------------- */
mp_int32 TaskPool::CreatePlugMgr()
{
    mp_string strPlgPath;
    mp_int32 iRet;

    COMMLOG(OS_LOG_DEBUG, "Begin create plugin manager.");
    strPlgPath = CPath::GetInstance().GetPluginsPath();
    COMMLOG(OS_LOG_DEBUG, "Plugin directory is %s.", strPlgPath.c_str());
    // CodeDex误报，Memory Leak
    NEW_CATCH_RETURN_FAILED(m_plugMgr, CPluginManager);
    iRet = m_plugMgr->Initialize(*this);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Init plugin manager failed.");
        delete m_plugMgr;
        m_plugMgr = NULL;
        return iRet;
    }
    m_plugMgr->SetPluginPath(strPlgPath.c_str());

    // 加载预加载插件并初始化
    std::vector<mp_string> vecPlgNames;
    m_plugCfgParse->GetPreLoadPlugins(vecPlgNames);
    m_plugMgr->LoadPreLoadPlugins(vecPlgNames);

    COMMLOG(OS_LOG_DEBUG, "Create plugin manager succ.");

    return MP_SUCCESS;
}


/* ------------------------------------------------------------
Description  : 判断插件是否可以卸载
               IPluginCallback 虚方法实现
Input        : pOldPlg -- 判断是否可卸载的插件指针
Return       : MP_TRUE -- 可以卸载
               MP_FALSE -- 不可以卸载
Create By    : yangwenjun 00275736
------------------------------------------------------------- */

mp_bool TaskPool::CanUnload(IPlugin& pOldPlg)
{
    mp_uint64 scn;
    mp_string plgName;
    // CodeDex误报，Dead Code
    if (m_plugMgr == NULL) {
        return MP_FALSE;
    }
    scn = m_plugMgr->GetSCN();

    plgName = pOldPlg.GetName();
    if (plgName.empty()) {
        return MP_FALSE;
    }

    for (mp_uint32 i = 0; i < m_pWorkers.size(); i++) {
        if (m_pWorkers[i]->CanUnloadPlugin(scn, plgName) == MP_FALSE) {
            return MP_FALSE;
        }
    }

    return MP_TRUE;
}

/* ------------------------------------------------------------
Description  : 升级事件处理函数(插件框架动态升级预留)
               IPluginCallback 虚方法实现
Create By    : yangwenjun 00275736
------------------------------------------------------------- */
mp_void TaskPool::OnUpgraded(IPlugin& pOldPlg, IPlugin& pNewPlg)
{
    return;
}

/* ------------------------------------------------------------
Description  : 设置插件选项(插件框架动态升级预留)
               IPluginCallback 虚方法实现
Create By    : yangwenjun 00275736
------------------------------------------------------------- */
mp_void TaskPool::SetOptions(IPlugin& plg)
{
    return;
}

/* ------------------------------------------------------------
Description  : 获取插件版本信息
               IPluginCallback 虚方法实
Input        : pszLib -- 插件名称
Output       : pszVer -- 用于保存版本信息的缓冲区指针
               sz -- 用于保存版本信息的缓冲区长度
Return       : 成功返回pszVer，失败返回NULL
Create By    : yangwenjun 00275736
------------------------------------------------------------- */
mp_string TaskPool::GetReleaseVersion(const mp_string& pszLib, mp_string& pszVer)
{
    COMMLOG(OS_LOG_DEBUG, "Begin get release version.");
    if (m_plugCfgParse == NULL) {
        return "";
    }

    mp_string strVersion;
    mp_int32 iRet = m_plugCfgParse->GetPluginVersion(pszLib, strVersion);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get release version failed.");
        return "";
    }

    pszVer = strVersion;
    COMMLOG(OS_LOG_DEBUG, "Get release version succ, name %s, version %s.", pszLib.c_str(), pszVer.c_str());
    return pszVer;
}