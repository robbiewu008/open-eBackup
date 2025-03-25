/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file VMwareNativeBackupOpenDiskTask.cpp
 * @brief  提供VMwareNative高级备份打卡磁盘查询已分配数据块功能
 * @version 1.0.0
 * @date 2021-07-20
 * @author jiaohehe wx1039612
 */

#include "apps/vmwarenative/VMwareNativeBackupOpenDiskTask.h"
#include "apps/vmwarenative/TaskStepVMwareNative.h"
#include "taskmanager/TaskStepLinkTarget.h"
#include "taskmanager/TaskStepScanDisk.h"
#include "apps/vmwarenative/TaskStepBackupOpenDisk.h"
#include "apps/vmwarenative/TaskStepPrepareTargetLun.h"
using namespace std;

VMwareNativeBackupOpenDiskTask::VMwareNativeBackupOpenDiskTask(const mp_string &taskID) : VMwareNativeTask(taskID)
{
    m_taskName = "VMwareNativeBackupOpenDiskTask";
    CreateTaskStep();
}

VMwareNativeBackupOpenDiskTask::~VMwareNativeBackupOpenDiskTask()
{}

mp_int32 VMwareNativeBackupOpenDiskTask::InitTaskStep(const Json::Value &param)
{
    // trigger vm opendisk backup task step
    if (MP_SUCCESS != InitTaskStepParam(param, "", STEPNAME_BACKUP_OPENDISK)) {
        ERRLOG("Unable to init open disk backup, task id '%s'.", m_taskId.c_str());
        return MP_FAILED;
    }
    INFOLOG("Init open disk backup parameters successfully, task id '%s'.", m_taskId.c_str());

    return MP_SUCCESS;
}

mp_void VMwareNativeBackupOpenDiskTask::CreateTaskStep()
{
    ADD_TASKSTEP(TaskStepBackupOpenDisk, STEPNAME_BACKUP_OPENDISK, TASK_STEP_INTERVAL_HUNDERED, m_steps);
}
