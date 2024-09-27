/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file VMwareNativeVmRestoreTask.h
 * @brief  The implemention about VMwareNativeVmRestoreTask
 * @version 1.0.0.0
 * @date 2020-08-01
 * @author wangguitao 00510599
 */
#ifndef AGENT_VMWARENATIVE_VMRESTORE_TASK
#define AGENT_VMWARENATIVE_VMRESTORE_TASK

#include "apps/vmwarenative/VMwareNativeTask.h"
#include "common/Types.h"

class VMwareNativeVmRestoreTask : public VMwareNativeTask {
public:
    VMwareNativeVmRestoreTask(const mp_string &taskID);
    virtual ~VMwareNativeVmRestoreTask();

    mp_int32 InitTaskStep(const Json::Value &param);

private:
    mp_void CreateTaskStep();
};

#endif
