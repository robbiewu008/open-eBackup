/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file TaskStepOracleNativeInstRestore.h
 * @brief  Contains function declarations for TaskStepOracleNativeInstRestore
 * @version 1.0.0
 * @date 2020-08-01
 * @author wangguitao 00510599
 */
#ifndef AGENT_BACKUP_STEP_INST_RECOVER_ORACLE
#define AGENT_BACKUP_STEP_INST_RECOVER_ORACLE
#include "apps/oraclenative/TaskStepOracleNative.h"
#include "common/Types.h"

static const mp_string STEPNAME_NATIVE_INST_RESTORE = "TaskStepOracleNativeInstRestore";
class TaskStepOracleNativeInstRestore : public TaskStepOracleNative {
public:
    TaskStepOracleNativeInstRestore(
        const mp_string& id, const mp_string& taskId, const mp_string& name, mp_int32 ratio, mp_int32 order);
    ~TaskStepOracleNativeInstRestore();
    mp_int32 Init(const Json::Value& param);
    mp_int32 Run();
    mp_int32 Cancel();
private:
    mp_int32 channel;
    mp_uint64 pitTime;
    mp_uint64 pitSCN;
    mp_string recoverPath;
    mp_int32 recoverOrder;
    mp_int32 recoverNum;
    mp_string encAlgo;
    mp_string encKey;
    mp_int32 m_irecoverTarget;

    mp_int32 BuildScriptParam(mp_string& strParam);
};

#endif
