/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file OracleNativeRestoreTask.h
 * @brief  Contains function declarations OracleNativeRestoreTask
 * @version 1.0.0
 * @date 2020-06-27
 * @author wangguitao 00510599
 */
#ifndef AGENT_ORACLE_NATIVE_RESTORE_TASK
#define AGENT_ORACLE_NATIVE_RESTORE_TASK

#include "common/Types.h"
#include "apps/oraclenative/OracleNativeTask.h"

class OracleNativeRestoreTask : public OracleNativeTask {
public:
    OracleNativeRestoreTask(const mp_string& taskID);
    virtual ~OracleNativeRestoreTask();
    // initialize task step param by tcp channel
    mp_int32 InitTaskStep(const Json::Value& param);

private:
    mp_void CreateTaskStep();
};

#endif
