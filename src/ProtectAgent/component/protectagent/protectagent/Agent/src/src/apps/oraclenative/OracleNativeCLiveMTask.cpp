/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file OracleNativeCLiveMTask.cpp
 * @brief  The implemention about OracleNativeCLiveMTask
 * @version 1.0.0.0
 * @date 2020-08-01
 * @author wangguitao 00510599
 */
#include "apps/oraclenative/OracleNativeCLiveMTask.h"
#include "common/Utils.h"
#include "taskmanager/TaskContext.h"
#include "apps/oraclenative/TaskStepOracleNativeCLiveMount.h"
#include "apps/oraclenative/TaskStepOracleNativeDismount.h"

OracleNativeCLiveMTask::OracleNativeCLiveMTask(const mp_string& taskID) : OracleNativeTask(taskID)
{
    m_taskName = "OracleNativeCLiveMTask";
    statusFlag = MP_FALSE;
    CreateTaskStep();
}

OracleNativeCLiveMTask::~OracleNativeCLiveMTask()
{}

mp_int32 OracleNativeCLiveMTask::InitTaskStep(const Json::Value& param)
{
    // stop database
    mp_int32 iRet = InitTaskStepParam(param, "", STEPNAME_CANCEL_LIVEMOUNT);
    if (iRet != MP_SUCCESS) {
        return iRet;
    }

    iRet = InitTaskStepParam(param, "", STEPNAME_ORACLE_NATIVEDISMOUNT);
    if (iRet != MP_SUCCESS) {
        return iRet;
    }

    return MP_SUCCESS;
}

/*
cancel database LiveMount
1.close database
2.cancel database LiveMount backup Media
1)fs
umount fs
deactive VG
export VG
2)ASM
dismount asm diskgroup
remove raw device
*/
mp_void OracleNativeCLiveMTask::CreateTaskStep()
{
    LOGGUARD("");
    mp_int32 iStepFivty = 50;
    ADD_TASKSTEP(TaskStepOracleNativeCLiveMount, STEPNAME_CANCEL_LIVEMOUNT, iStepFivty, m_steps);
    ADD_TASKSTEP(TaskStepOracleNativeDismount, STEPNAME_ORACLE_NATIVEDISMOUNT, iStepFivty, m_steps);
}
