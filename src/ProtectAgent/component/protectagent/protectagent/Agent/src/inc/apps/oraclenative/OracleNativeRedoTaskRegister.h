/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file OracleNativeRedoTaskRegister.h
 * @brief  Contains function declarations for TaskStepOracleNativeLiveMount
 * @version 1.0.0
 * @date 2020-08-01
 * @author wangguitao 00510599
 */
#ifndef AGENT_ORACLE_NATIVE_REDO_TASK_REGISTER
#define AGENT_ORACLE_NATIVE_REDO_TASK_REGISTER

#include "taskmanager/TaskRedoFuncContainer.h"
#include "apps/oraclenative/OracleNativeBackupTask.h"
#include "apps/oraclenative/OracleNativeExpireCopyTask.h"

class OracleNativeRedoTaskRegister {
public:
    OracleNativeRedoTaskRegister()
    {
        TaskRedoFuncContainer::GetInstance().RegisterNewFunc(
            "OracleNativeBackupTask", OracleNativeBackupTask::CreateRedoTask);
        TaskRedoFuncContainer::GetInstance().RegisterNewFunc(
            "OracleNativeExpireCopyTask", OracleNativeExpireCopyTask::CreateRedoTask);
    }
    ~OracleNativeRedoTaskRegister()
    {}
};

#endif