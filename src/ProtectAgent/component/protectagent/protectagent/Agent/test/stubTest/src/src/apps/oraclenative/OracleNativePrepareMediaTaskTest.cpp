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
#include "apps/oraclenative/OracleNativePrepareMediaTaskTest.h"


TEST_F(OracleNativePrepareMediaTaskTest, InitTaskStepStub)
{
    mp_string taskID = "1";
    OracleNativePrepareMediaTask task(taskID);
    Json::Value param;
    task.InitPrepareMedia(param);
}

TEST_F(OracleNativePrepareMediaTaskTest, CreateTaskStepStub)
{
    mp_string taskID = "1";
    OracleNativePrepareMediaTask task(taskID);
    task.CreateTaskStep();
}

TEST_F(OracleNativePrepareMediaTaskTest, InitTaskStepStubA)
{
    mp_string taskID = "1";
    OracleNativePrepareMediaTask task(taskID);
    Json::Value param;
    task.InitTaskStep(param);
}


TEST_F(OracleNativePrepareMediaTaskTest, InitPrepareMediaStub)
{
    mp_string taskID = "1";
    OracleNativePrepareMediaTask task(taskID);
    Json::Value param;
    task.taskType = 0;
    task.InitPrepareMedia(param);
}
