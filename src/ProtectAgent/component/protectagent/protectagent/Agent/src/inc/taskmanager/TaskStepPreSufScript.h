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
#ifndef AGENT_BACKUP_TASK_STEP_SCRIPT
#define AGENT_BACKUP_TASK_STEP_SCRIPT

#include "common/Types.h"
#include "taskmanager/TaskStepScript.h"

static const mp_string STEPNAME_PERSCRIPT = "preScriptTaskStep";
static const mp_string STEPNAME_SUFSCRIPT = "sufScriptTaskStep";
static const mp_string STEPNAME_FAILEDSCRIPT = "failScriptTaskStep";

class TaskStepPreScript : public TaskStepScript {
public:
    TaskStepPreScript(
        const mp_string& id, const mp_string& taskId, const mp_string& name, mp_int32 ratio, mp_int32 order);
    ~TaskStepPreScript();
    // initialize task step param by tcp channel
    mp_int32 Init(const Json::Value& param);

private:
    mp_string scriptName;
    mp_string scriptNameParam;
};

class TaskStepPostScript : public TaskStepScript {
public:
    TaskStepPostScript(
        const mp_string& id, const mp_string& taskId, const mp_string& name, mp_int32 ratio, mp_int32 order);
    ~TaskStepPostScript();
    // initialize task step param by tcp channel
    mp_int32 Init(const Json::Value& param);

private:
    mp_string scriptName;
    mp_string scriptNameParam;
};

class TaskStepFailPostScript : public TaskStepScript {
public:
    TaskStepFailPostScript(
        const mp_string& id, const mp_string& taskId, const mp_string& name, mp_int32 ratio, mp_int32 order);
    ~TaskStepFailPostScript();
    // initialize task step param by tcp channel
    mp_int32 Init(const Json::Value& param);

private:
    mp_string scriptName;
    mp_string scriptNameParam;
};

#endif
