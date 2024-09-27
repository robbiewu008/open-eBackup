/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 *
 * @file VMwareNativeInitVddkLibTask.cpp
 * @author w00558987
 * @brief 备份/恢复任务初始化VDDK步骤
 * @version 0.1
 * @date 2021-01-16
 *
 */

#include "apps/vmwarenative/VMwareNativeInitVddkLibTaskTest.h"

TEST_F(VMwareNativeInitVddkLibTaskTest, InitTaskStepStub)
{
    mp_string taskID = "1";
    VMwareNativeInitVddkLibTask task(taskID);
    Json::Value param;
    // task.InitTaskStep(param);
}

TEST_F(VMwareNativeInitVddkLibTaskTest, CreateTaskStepStub)
{
    mp_string taskID = "1";
    VMwareNativeInitVddkLibTask task(taskID);
    task.CreateTaskStep();
}