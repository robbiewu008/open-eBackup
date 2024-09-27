/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file OracleNativeDismountTask.h
 * @brief  Contains function declarations OracleNativeDismountTask
 * @version 1.0.0
 * @date 2020-06-27
 * @author wangguitao 00510599
 */
#ifndef AGENT_ORACLE_NATIVE_DISMOUNT_TASK
#define AGENT_ORACLE_NATIVE_DISMOUNT_TASK

#include "common/Types.h"
#include "apps/oraclenative/OracleNativeTask.h"

class OracleNativeDismountTask : public OracleNativeTask {
public:
    OracleNativeDismountTask(const mp_string& taskID);
    virtual ~OracleNativeDismountTask();

    mp_int32 InitTaskStep(const Json::Value& param);
private:
    mp_void CreateTaskStep();
};

#endif

