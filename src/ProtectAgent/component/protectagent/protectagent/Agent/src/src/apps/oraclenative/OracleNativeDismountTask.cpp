/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file OracleNativeDismountTask.h
 * @brief  Contains function declarations for OracleNativeDismountTask
 * @version 1.0.0
 * @date 2020-08-01
 * @author wangguitao 00510599
 */
#include "apps/oraclenative/OracleNativeDismountTask.h"

#include "common/Utils.h"
#include "apps/oraclenative/TaskStepOracleNativeDismount.h"

OracleNativeDismountTask::OracleNativeDismountTask(const mp_string& taskID) : OracleNativeTask(taskID)
{
    m_taskName = "OracleNativeDismountTask";
    statusFlag = MP_FALSE;
    CreateTaskStep();
    taskMode = SYNCHRONOUS_TASK;
}

OracleNativeDismountTask::~OracleNativeDismountTask()
{}

mp_int32 OracleNativeDismountTask::InitTaskStep(const Json::Value& param)
{
    return InitTaskStepParam(param, "", STEPNAME_ORACLE_NATIVEDISMOUNT);
}

mp_void OracleNativeDismountTask::CreateTaskStep()
{
    LOGGUARD("");
    mp_int32 iNum = 100;
    ADD_TASKSTEP(TaskStepOracleNativeDismount, STEPNAME_ORACLE_NATIVEDISMOUNT, iNum, m_steps);
}
