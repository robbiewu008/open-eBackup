/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file TaskStepOracleNativeRestore.h
 * @brief  Contains function declarations for TaskStepOracleNativeRestore
 * @version 1.0.0
 * @date 2020-08-01
 * @author wangguitao 00510599
 */
#ifndef AGENT_BACKUP_STEP_RESTORE_ORACLE_H
#define AGENT_BACKUP_STEP_RESTORE_ORACLE_H

#include <vector>

#include "apps/oraclenative/TaskStepOracleNative.h"
#include "common/Types.h"

static const mp_string STEPNAME_NATIVERESTORE = "TaskStepOracleNativeRestore";
class TaskStepOracleNativeRestore : public TaskStepOracleNative {
public:
    TaskStepOracleNativeRestore(
        const mp_string& id, const mp_string& taskId, const mp_string& name, mp_int32 ratio, mp_int32 order);
    ~TaskStepOracleNativeRestore();
    mp_int32 Init(const Json::Value& param);
    mp_int32 Run();
    mp_int32 RefreshStepInfo();
    mp_int32 GetHistorySpeed(const std::vector<mp_string>& vecResult);
    mp_int32 Cancel();

private:
    mp_int32 channel;
    mp_uint64 pitTime;
    mp_uint64 pitSCN;
    mp_int32 recoverTarget;
    mp_string recoverPath;
    mp_int32 recoverOrder;
    mp_int32 recoverNum;
    mp_string encAlgo;
    mp_string encKey;
    mp_int32 m_iRestoreBy;
    mp_int32 m_iHistorySpeed;
    
    mp_int32 BuildRestoreScriptParam(mp_string& strParam);
};

#endif
