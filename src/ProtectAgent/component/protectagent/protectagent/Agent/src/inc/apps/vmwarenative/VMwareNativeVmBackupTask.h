/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file VMwareNativeVmBackupTask.h
 * @brief  Contains function declarations VMwareNativeVmBackupTask
 * @version 1.0.0
 * @date 2020-06-27
 * @author wangguitao 00510599
 */
#ifndef AGENT_VMWARENATIVE_VMBACKUP_TASK
#define AGENT_VMWARENATIVE_VMBACKUP_TASK

#include "apps/vmwarenative/VMwareNativeTask.h"
#include "common/Types.h"

class VMwareNativeVmBackupTask : public VMwareNativeTask {
public:
    VMwareNativeVmBackupTask(const mp_string &taskID);
    virtual ~VMwareNativeVmBackupTask();

    mp_int32 InitTaskStep(const Json::Value &param);

private:
    mp_void CreateTaskStep();
};

#endif
