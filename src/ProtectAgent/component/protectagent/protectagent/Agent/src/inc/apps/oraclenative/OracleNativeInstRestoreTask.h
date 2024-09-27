/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file CSystemExec.h
 * @brief  Contains function declarations no on-premies database live moun task Operations
 * @version 1.0.0
 * @date 2020-04-02
 * @author wangguitao 00510599
 */
#ifndef AGENT_ORACLE_NATIVE_INST_RESTORE_TASK
#define AGENT_ORACLE_NATIVE_INST_RESTORE_TASK

#include "common/Types.h"
#include "apps/oraclenative/OracleNativeTask.h"

class OracleNativeInstRestoreTask : public OracleNativeTask {
public:
    OracleNativeInstRestoreTask(const mp_string& taskID);
    virtual ~OracleNativeInstRestoreTask();
    // initialize task step param by tcp channel
    mp_int32 InitTaskStep(const Json::Value& param);

private:
    mp_void CreateTaskStep();
};

#endif
