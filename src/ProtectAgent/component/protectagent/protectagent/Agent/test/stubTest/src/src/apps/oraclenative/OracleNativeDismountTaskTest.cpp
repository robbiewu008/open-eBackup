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
#include "apps/oraclenative/OracleNativeDismountTaskTest.h"


TEST_F(OracleNativeDismountTaskTest, InitTaskStepStub)
{
    mp_string taskID = "1";
    OracleNativeDismountTask task(taskID);
    Json::Value param;
    task.InitTaskStep(param);
}

TEST_F(OracleNativeDismountTaskTest, CreateTaskStepStub)
{
    mp_string taskID = "1";
    OracleNativeDismountTask task(taskID);
    task.CreateTaskStep();
}
