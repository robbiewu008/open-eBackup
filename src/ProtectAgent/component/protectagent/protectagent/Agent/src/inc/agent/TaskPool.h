/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file TaskPool.h
 * @brief  Contains function declarations TaskPool
 * @version 1.0.0
 * @date 2020-06-27
 * @author wangguitao 00510599
 */
#ifndef _AGENT_TASK_POOL_H_
#define _AGENT_TASK_POOL_H_

#include <vector>
#include "pluginfx/iplugin.h"
#include "pluginfx/PluginCfgParse.h"
#include "pluginfx/PluginManager.h"
#include "agent/TaskWorker.h"
#include "agent/TaskVssWorker.h"
#include "agent/TaskDispatchWorker.h"
#include "common/Types.h"
#include "common/CMpThread.h"
#include "common/ConfigXmlParse.h"

class TaskPool : public IPluginCallback {
public:
    TaskPool();
    ~TaskPool();

    EXTER_ATTACK mp_int32 Init();
    mp_void Exit();

    // IPluginCallback 虚方法实现
    virtual mp_bool CanUnload(IPlugin& pOldPlg);
    virtual mp_void OnUpgraded(IPlugin& pOldPlg, IPlugin& pNewPlg);
    virtual mp_void SetOptions(IPlugin& plg);
    virtual mp_string GetReleaseVersion(const mp_string& pszLib, mp_string& pszVer);

private:
    // 工作线程总数
    static mp_int32 m_workerThreadCount;
    std::vector<TaskWorker*> m_pWorkers;
    TaskDispatchWorker* m_pDispatchWorker;
    PluginCfgParse* m_plugCfgParse;
    CPluginManager* m_plugMgr;
#ifdef WIN32
    TaskVssWorker* m_pVssWorker;
#endif

private:
    mp_int32 CreateWorkers();
    mp_int32 CreateDispatchWorker();
    mp_int32 CreatePlgConfParse();
    mp_int32 CreatePlugMgr();
#ifdef WIN32
    mp_int32 CreateVssWorker();
#endif
};

#endif  // _AGENT_TASK_POOL_H_
