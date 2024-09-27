/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file TaskStepPreSufScript.h
 * @brief  Contains function declarations for TaskStepPreSufScript
 * @version 1.0.0
 * @date 2020-08-01
 * @author wangguitao 00510599
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
