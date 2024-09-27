/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file OracleNativeLiveMTask.h
 * @brief  no on-premies database live moun task
 * @version 1.0.0
 * @date 2020-02-04
 * @author wangguitao 00510599
 */
#ifndef AGENT_ORACLE_NATIVE_LIVEMOUNT_TASK
#define AGENT_ORACLE_NATIVE_LIVEMOUNT_TASK

#include "common/Types.h"
#include "apps/oraclenative/OracleNativeTask.h"

class OracleNativeLiveMTask : public OracleNativeTask {
public:
    OracleNativeLiveMTask(const mp_string& taskID);
    virtual ~OracleNativeLiveMTask();
    // initialize task step param by tcp channel
    mp_int32 InitTaskStep(const Json::Value& param);

private:
    mp_void CreateTaskStep();
};

#endif
