/*
* This file is a part of the open-eBackup project.
* This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this file, You can obtain one at
* http://mozilla.org/MPL/2.0/.
*
* Copyright (c) [2024] Huawei Technologies Co.,Ltd.
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
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
