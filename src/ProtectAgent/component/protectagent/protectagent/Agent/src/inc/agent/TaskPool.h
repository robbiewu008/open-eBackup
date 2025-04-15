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
