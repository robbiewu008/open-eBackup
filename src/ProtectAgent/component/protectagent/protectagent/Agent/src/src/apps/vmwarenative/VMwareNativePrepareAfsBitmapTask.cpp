/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 * Description: 无效数据识别功能实现
 * Author:hw
 * Create:2024-01-13
 * Notes:无
 */
#include "apps/vmwarenative/VMwareNativePrepareAfsBitmapTask.h"
#include "apps/vmwarenative/TaskStepVMwareNative.h"
#include "apps/vmwarenative/TaskStepPrepareAfsBitmap.h"
using namespace std;

VMwareNativePrepareAfsBitmapTask::VMwareNativePrepareAfsBitmapTask(const mp_string &taskID) : VMwareNativeTask(taskID)
{
    m_taskName = "VMwareNativePrepareAfsBitmapTask";
    CreateTaskStep();
}

VMwareNativePrepareAfsBitmapTask::~VMwareNativePrepareAfsBitmapTask()
{}

mp_int32 VMwareNativePrepareAfsBitmapTask::InitTaskStep(const Json::Value &param)
{
    // trigger vm opendisk backup task step
    if (MP_SUCCESS != InitTaskStepParam(param, "", STEPNAME_BACKUP_AFSBITMAP)) {
        ERRLOG("Unable to init afs bitmap, task id '%s'.", m_taskId.c_str());
        return MP_FAILED;
    }
    INFOLOG("Init afs bitmap parameters successfully, task id '%s'.", m_taskId.c_str());
    return MP_SUCCESS;
}

mp_void VMwareNativePrepareAfsBitmapTask::CreateTaskStep()
{
    ADD_TASKSTEP(TaskStepPrepareAfsBitmap, STEPNAME_BACKUP_AFSBITMAP, TASK_STEP_INTERVAL_HUNDERED, m_steps);
}
