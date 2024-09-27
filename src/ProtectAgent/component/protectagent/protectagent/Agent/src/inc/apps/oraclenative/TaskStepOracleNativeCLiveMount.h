/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file TaskStepOracleNativeCLiveMount.h
 * @brief  Contains function declarations for TaskStepOracleNativeCLiveMount
 * @version 1.0.0
 * @date 2020-08-01
 * @author wangguitao 00510599
 */
#ifndef AGENT_BACKUP_STEP_CANCEL_LIVEMOUNT_ORACLE
#define AGENT_BACKUP_STEP_CANCEL_LIVEMOUNT_ORACLE
#include "apps/oraclenative/TaskStepOracleNative.h"
#include "common/Types.h"

static const mp_string STEPNAME_CANCEL_LIVEMOUNT = "TaskStepCancelLiveMount";
class TaskStepOracleNativeCLiveMount : public TaskStepOracleNative {
public:
    TaskStepOracleNativeCLiveMount(
        const mp_string& id, const mp_string& taskId, const mp_string& name, mp_int32 ratio, mp_int32 order);
    ~TaskStepOracleNativeCLiveMount();
    mp_int32 Init(const Json::Value& param);
    mp_int32 Run();

private:
    mp_void BuildScriptParam(mp_string& strParam);
};

#endif
