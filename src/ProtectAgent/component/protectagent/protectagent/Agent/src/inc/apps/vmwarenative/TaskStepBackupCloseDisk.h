/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file TaskStepBackupOpenDisk.h
 * @brief  Contains function declarations TaskStepBackupCloseDisk
 * @version 1.0.0
 */
#ifndef __AGENT_TASKSTEP_BACKUP_CLOSEDISK_H__
#define __AGENT_TASKSTEP_BACKUP_CLOSEDISK_H__

#include "apps/vmwarenative/TaskStepVMwareNative.h"

static const mp_string STEPNAME_BACKUP_CLOSEDISK = "TaskStepBackupCloseDisk";
class TaskStepBackupCloseDisk : public TaskStepVMwareNative {
public:
    TaskStepBackupCloseDisk(
        const mp_string &id, const mp_string &taskId, const mp_string &name, mp_int32 ratio, mp_int32 order);
    virtual ~TaskStepBackupCloseDisk();

    mp_int32 Init(const Json::Value &param);
    mp_int32 Run();

private:
    mp_long m_invokedTime;
    mp_int32 m_timeInterval;
};

#endif
