/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file OracleNativePrepareMediaTask.h
 * @brief  Contains function declarations for OracleNativePrepareMediaTask
 * @version 1.0.0
 * @date 2020-08-01
 * @author wangguitao 00510599
 */
#ifndef AGENT_ORACLE_NATIVE_PREPARE_MEDIA_TASK
#define AGENT_ORACLE_NATIVE_PREPARE_MEDIA_TASK
#include "common/Types.h"
#include "apps/oraclenative/OracleNativeTask.h"

class OracleNativePrepareMediaTask : public OracleNativeTask {
public:
    OracleNativePrepareMediaTask(const mp_string& taskID);
    virtual ~OracleNativePrepareMediaTask();

    mp_int32 InitTaskStep(const Json::Value& param);
private:
    mp_int32 taskType;
    mp_int32 storType;
    mp_void CreateTaskStep();
    mp_int32 InitPrepareMedia(const Json::Value& param);
};

#endif