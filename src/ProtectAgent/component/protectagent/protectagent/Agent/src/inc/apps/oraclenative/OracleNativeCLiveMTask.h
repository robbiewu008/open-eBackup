/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file OracleNativeCLiveMTask.h
 * @brief  no on-premies database cancel live moun task
 * @version 1.0.0
 * @date 2020-02-04
 * @author wangguitao 00510599
 */
#ifndef AGENT_ORACLE_NATIVE_CANCEL_LIVEMOUNT_TASK
#define AGENT_ORACLE_NATIVE_CANCEL_LIVEMOUNT_TASK

#include "common/Types.h"
#include "apps/oraclenative/OracleNativeTask.h"

class OracleNativeCLiveMTask : public OracleNativeTask {
public:
    OracleNativeCLiveMTask(const mp_string& taskID);
    virtual ~OracleNativeCLiveMTask();
    // initialize task step param by tcp channel
    mp_int32 InitTaskStep(const Json::Value& param);

private:
    mp_void CreateTaskStep();
};

#endif
