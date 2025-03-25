/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file VMwareNativeInitVddkLibTask.h
 * @brief  Contains function declarations VMwareNativeInitVddkLibTask
 * @version 1.0.0
 * @date 2020-06-27
 * @author wangguitao 00510599
 */
#ifndef AGENT_VMWARENATIVE_INIT_VDDKLIB_TASK
#define AGENT_VMWARENATIVE_INIT_VDDKLIB_TASK

#include "apps/vmwarenative/VMwareNativeTask.h"

class VMwareNativeInitVddkLibTask : public VMwareNativeTask {
public:
    VMwareNativeInitVddkLibTask(const mp_string &taskID);
    virtual ~VMwareNativeInitVddkLibTask();

    mp_int32 InitTaskStep(const Json::Value &param);

private:
    mp_void CreateTaskStep();

private:
    mp_int32 m_storageProtocol;
};

#endif
