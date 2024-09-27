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
#include "apps/oraclenative/OracleNativeLiveMTaskTest.h"
#include "apps/oraclenative/OracleNativeBackupTask.h"
namespace {
mp_int32 InitTaskStepParamTest(const Json::Value& param, const mp_string& paramKey, const mp_string& stepName)
{
    return MP_SUCCESS;
}
};

TEST_F(OracleNativeLiveMTaskTest, InitTaskStepStub)
{
    mp_string taskID = "1";
    OracleNativeLiveMTask task(taskID);
    Json::Value param;
    task.InitTaskStep(param);
    stub.set(ADDR(Task, InitTaskStepParam), InitTaskStepParamTest);
    task.InitTaskStep(param);
    stub.reset(ADDR(Task, InitTaskStepParam));
}

TEST_F(OracleNativeLiveMTaskTest, CreateTaskStepStub)
{
    mp_string taskID = "1";
    OracleNativeLiveMTask task(taskID);
    task.CreateTaskStep();
}

// TEST_F(OracleNativeLiveMTaskTest, RunTaskBeforeTest)
// {
//     mp_string taskID = "1";
//     OracleNativeLiveMTask task(taskID);
//     task.RunTaskBefore();
// }

// TEST_F(OracleNativeLiveMTaskTest, RunTaskAfterTest)
// {
//     mp_string taskID = "1";
//     OracleNativeLiveMTask task(taskID);
//     // try {
//     //     task.RunTaskAfter();
//     // }
//     // catch (std::exception &ec) {

//     // }
// }

TEST_F(OracleNativeLiveMTaskTest, RunGetProgressTaskTest)
{
    mp_string taskID = "1";
    // OracleNativeBackupTask taska(taskID);
    OracleNativeLiveMTask task(taskID);
    mp_void* pThis = nullptr;
    task.RunGetProgressTask(pThis);
    // pThis = &taska;
    // task.RunGetProgressTask(pThis);
}