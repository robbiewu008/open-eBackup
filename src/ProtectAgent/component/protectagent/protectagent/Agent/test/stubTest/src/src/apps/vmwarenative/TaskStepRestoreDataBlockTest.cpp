/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 *
 * @file TaskStepRestoreDataBlockTest.cpp
 * @author w00558987
 * @brief 提供VMwareNative高级备份磁盘数据恢复功能
 * @version 0.1
 * @date 2021-07-21
 *
 */

#include "apps/vmwarenative/TaskStepRestoreDataBlockTest.h"
#include "apps/vmwarenative/TaskStepRestoreDataBlock.h"
#include "apps/vmwarenative/TaskStepVMwareNative.h"
#include "common/ConfigXmlParse.h"
#include "common/Types.h"

namespace {
    const mp_string id = "1";
    const mp_string taskId = "885ef1e6-9eb8-4148-8116-558aa3bf46e7";
    const mp_string name = "taskStepBackupDataBlockTest";
    const mp_int32 ratio = 1;
    const mp_int32 order = 1;
}  // namespace


mp_int32 InvokeDataProcessLogicSuccess(Json::Value& param, Json::Value& respParam, mp_uint32 reqCmd, mp_uint32 rspCmd)
{
    return 0;
}

mp_int32 InvokeDataProcessLogicFailure(Json::Value& param, Json::Value& respParam, mp_uint32 reqCmd, mp_uint32 rspCmd)
{
    return -1;
}

// Run interface testing
TEST_F(TaskStepRestoreDataBlockTest, InvokeDataProcessLogic_success)
{
    Stub stub;
    stub.set(ADDR(TaskStepVMwareNative, DataProcessLogic), InvokeDataProcessLogicSuccess);

    TaskStepRestoreDataBlock ts(id, taskId, name, ratio, order);
    EXPECT_EQ(0, ts.Run());
}

// Run interface testing
TEST_F(TaskStepRestoreDataBlockTest, InvokeDataProcessLogic_failure)
{
    Stub stub;
    stub.set(ADDR(TaskStepVMwareNative, DataProcessLogic), InvokeDataProcessLogicFailure);

    TaskStepRestoreDataBlock ts(id, taskId, name, ratio, order);
    EXPECT_EQ(-1, ts.Run());
}
