/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file VMwareNativeBackupCloseDiskTask.h
 * @brief  Contains function declarations VMwareNativeBackupCloseDiskTask
 * @version 1.0.0
 * @date 2022-10-22
 * @author qingmin 00414872
 */
#ifndef AGENT_VMWARENATIVE_BACKUP_CLOSEDISK_TASK
#define AGENT_VMWARENATIVE_BACKUP_CLOSEDISK_TASK

#include "apps/vmwarenative/VMwareNativeTask.h"

class VMwareNativeBackupCloseDiskTask : public VMwareNativeTask {
public:
    VMwareNativeBackupCloseDiskTask(const mp_string &taskID);
    virtual ~VMwareNativeBackupCloseDiskTask();

    mp_int32 InitTaskStep(const Json::Value &param);

private:
    mp_void CreateTaskStep();
};

#endif
