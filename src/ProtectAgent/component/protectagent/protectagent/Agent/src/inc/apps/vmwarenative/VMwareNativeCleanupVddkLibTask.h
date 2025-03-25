/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file VMwareNativeCleanupVddkLibTask.h
 * @brief  The implemention about VMwareNativeCleanupVddkLibTask
 * @version 1.0.0.0
 * @date 6/12/2014
 * @author wangguitao 00510599
 */
#ifndef AGENT_VMWARENATIVE_CLEANUP_VDDKLIB_TASK
#define AGENT_VMWARENATIVE_CLEANUP_VDDKLIB_TASK

#include "apps/vmwarenative/VMwareNativeTask.h"

class VMwareNativeCleanupVddkLibTask : public VMwareNativeTask {
public:
    VMwareNativeCleanupVddkLibTask(const mp_string &taskID);
    virtual ~VMwareNativeCleanupVddkLibTask();

    mp_int32 InitTaskStep(const Json::Value &param);

private:
    mp_void CreateTaskStep();

private:
    mp_int32 m_storageProtocol;
};

#endif
