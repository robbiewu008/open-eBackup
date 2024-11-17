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
#ifndef _AGENT_TASK_STEP_SCRIPT_H_
#define _AGENT_TASK_STEP_SCRIPT_H_
#include <vector>

#include "common/CMpThread.h"
#include "common/Types.h"
#include "taskmanager/TaskStep.h"

// script running mode
typedef enum _RunMode_t {
    SCRIPT_RUN_WITH_ROOT = 0,  // execute with root
    SCRIPT_RUN_WITH_RDADMIN,   // execute with rdadmin
} ScriptRunMode;

class TaskStepScript : public TaskStep {
public:
    TaskStepScript(const mp_string& id, const mp_string& taskId, const mp_string& name, mp_int32 ratio, mp_int32 order);
    virtual ~TaskStepScript();

    mp_int32 Init(const Json::Value& param);
    mp_void SetRunMode(ScriptRunMode runMode);
    mp_void InitBuiltInWithRdAdminExec(const mp_string& startScript, const mp_string& startScriptParam,
        const mp_string& stopScript, const mp_string& stopScriptParam);
    mp_void InitBuiltInWithRootExec(mp_int32 startScriptId, const mp_string& startScriptParam, mp_int32 stopScriptId,
        const mp_string& stopScriptParam);
    mp_void Init3rdScript(const mp_string& startScript, const mp_string& startScriptParam, const mp_string& stopScript,
        const mp_string& stopScriptParam, mp_bool affectResult = MP_FALSE);
    mp_int32 Run();
    mp_int32 Cancel();
    mp_int32 Cancel(Json::Value& respParam);
    mp_int32 Stop(const Json::Value& param);
    mp_int32 Redo(mp_string& innerPID);
    mp_int32 Update(const Json::Value& param);
    mp_int32 Update(Json::Value& param, Json::Value& respParam);
    mp_int32 Finish(const Json::Value& param);
    mp_int32 Finish(Json::Value& param, Json::Value& respParam);
    mp_int32 RefreshStepInfo();

private:
    thread_lock_t m_statusFileLocker;
    ScriptRunMode m_runMode;
    mp_bool is3rdScript;
    mp_string m_startScript;
    mp_string m_stopScript;
    mp_int32 m_startScriptId;
    mp_int32 m_stopScriptId;
    mp_string m_startScriptParam;
    mp_string m_stopScriptParam;
    /*
    m_affectResult： indicate that if the user defined script executes failed, the whole task is successful or failed.
    true: the user defined script executes failed, the whole task is failed
    false:the user defined script executes failed, the whole task is successful
    */
    mp_bool m_affectResult;

    mp_int32 RunScriptByRdAdmin(const mp_string& scriptName, const mp_string& param);
    mp_int32 RunScriptByRoot(mp_int32 scriptId, const mp_string& param);
    mp_int32 RunThirdPartyScriptAsRoot(const mp_string& scriptName, const mp_string& param);
    mp_int32 UpdateTaskStatusFile();
};

#endif
