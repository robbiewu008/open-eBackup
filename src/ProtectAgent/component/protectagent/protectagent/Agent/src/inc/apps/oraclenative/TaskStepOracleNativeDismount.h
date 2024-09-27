/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file TaskStepOracleNativeDismount.h
 * @brief  Contains function declarations for TaskStepOracleNativeDismount
 * @version 1.0.0
 * @date 2020-08-01
 * @author wangguitao 00510599
 */
#ifndef AGENT_BACKUP_STEP_ORACLENATIVE_DISMOUNT_H
#define AGENT_BACKUP_STEP_ORACLENATIVE_DISMOUNT_H
#include <vector>
#include "common/Types.h"
#include "apps/oraclenative/TaskStepOracleNative.h"

static const mp_string STEPNAME_ORACLE_NATIVEDISMOUNT = "TaskStepOracleNativeDismount";
class TaskStepOracleNativeDismount : public TaskStepOracleNative {
public:
    TaskStepOracleNativeDismount(
        const mp_string& id, const mp_string& taskId, const mp_string& name, mp_int32 ratio, mp_int32 order);
    ~TaskStepOracleNativeDismount();
    mp_int32 Init(const Json::Value& param);
    mp_int32 Run();

private:
    // 0-clean, 1-dismount
    mp_int32 taskType;
    mp_int32 storType;
    mp_string recoverPath;

    // user and key are used in iscsi CHAP or cifs authentication
    mp_string authUser;
    mp_string authKey;
    Json::Value storageInfo;
    std::vector<mp_string> ipList;
    mp_string logSharePath;
    mp_string dataSharePath;
    
    mp_int32 BuildScriptParam(mp_string& strParam);
    mp_int32 ParseVolumeParameter(const Json::Value& param);
};

#endif
