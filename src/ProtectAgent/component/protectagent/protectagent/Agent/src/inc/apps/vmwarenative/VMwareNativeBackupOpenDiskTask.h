/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file VMwareNativeBackupOpenDiskTask.h
 * @brief  Contains function declarations VMwareNativeBackupOpenDiskTask
 * @version 1.0.0
 * @date 2020-08-01
 * @author wangguitao 00510599
 */
#ifndef AGENT_VMWARENATIVE_BACKUP_OPENDISK_TASK
#define AGENT_VMWARENATIVE_BACKUP_OPENDISK_TASK

#include "apps/vmwarenative/VMwareNativeTask.h"

class VMwareNativeBackupOpenDiskTask : public VMwareNativeTask {
public:
    VMwareNativeBackupOpenDiskTask(const mp_string &taskID);
    virtual ~VMwareNativeBackupOpenDiskTask();

    mp_int32 InitTaskStep(const Json::Value &param);

private:
    mp_void CreateTaskStep();
};

#endif
