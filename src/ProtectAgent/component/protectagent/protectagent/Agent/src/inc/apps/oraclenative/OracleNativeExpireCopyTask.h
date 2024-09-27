/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file OracleNativeExpireCopyTask.h
 * @brief  Contains function declarations for OracleNativeExpireCopyTask
 * @version 1.0.0
 * @date 2020-08-01
 * @author wangguitao 00510599
 */
#ifndef AGENT_ORACLE_NATIVE_EXPIRE_COPY_TASK
#define AGENT_ORACLE_NATIVE_EXPIRE_COPY_TASK

#include "common/Types.h"
#include "apps/oraclenative/OracleNativeTask.h"

class OracleNativeExpireCopyTask : public OracleNativeTask {
public:
    OracleNativeExpireCopyTask(const mp_string& taskID);
    virtual ~OracleNativeExpireCopyTask();

    mp_int32 InitTaskStep(const Json::Value& param);
    static Task *CreateRedoTask(const mp_string& taskId);
private:
    mp_void CreateTaskStep();
};

#endif
