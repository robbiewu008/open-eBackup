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
#include "taskmanager/TaskStepLinkTargetTest.h"
#include "taskmanager/TaskStepLinkTarget.h"
#include "host/host.h"

static mp_void StubCLoggerLog(mp_void){
    return;
}

mp_int32 LinkiScsiTargetTest(const Json::Value& scsiTargets)
{
    return MP_SUCCESS;
}

TEST_F(TaskStepLinkTargetTest, InitTest)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    mp_string id;
    mp_string taskId;
    mp_string name;
    mp_int32 ratio;
    mp_int32 order;
    TaskStepLinkTarget target(id, taskId, name, ratio ,order);
    Json::Value param;
    target.Init(param);
    param["aaa"] = 1;
    param["bbb"] = 2;
    target.Init(param);
}

TEST_F(TaskStepLinkTargetTest, RunTest)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    mp_string id;
    mp_string taskId;
    mp_string name;
    mp_int32 ratio;
    mp_int32 order;
    TaskStepLinkTarget target(id, taskId, name, ratio ,order);
    Json::Value param;
    target.Init(param);
    target.Run();
    stub.set(ADDR(CHost, LinkiScsiTarget), LinkiScsiTargetTest);
    target.Run();
    target.m_stepStatus = STATUS_NO_EXISTS;
    target.Run();
}

TEST_F(TaskStepLinkTargetTest, Cancel1Test)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    mp_string id;
    mp_string taskId;
    mp_string name;
    mp_int32 ratio;
    mp_int32 order;
    TaskStepLinkTarget target(id, taskId, name, ratio ,order);
    Json::Value param;
    target.Cancel();
}

TEST_F(TaskStepLinkTargetTest, RedoTest)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    mp_string id;
    mp_string taskId;
    mp_string name;
    mp_int32 ratio;
    mp_int32 order;
    TaskStepLinkTarget target(id, taskId, name, ratio ,order);
    mp_string innerPID;
    target.Redo(innerPID);
}

TEST_F(TaskStepLinkTargetTest, StopTest)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    mp_string id;
    mp_string taskId;
    mp_string name;
    mp_int32 ratio;
    mp_int32 order;
    TaskStepLinkTarget target(id, taskId, name, ratio ,order);
    Json::Value param;
    target.Stop(param);
}

TEST_F(TaskStepLinkTargetTest, Update1Test)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    mp_string id;
    mp_string taskId;
    mp_string name;
    mp_int32 ratio;
    mp_int32 order;
    TaskStepLinkTarget target(id, taskId, name, ratio ,order);
    Json::Value param;
    target.Update(param);
}

TEST_F(TaskStepLinkTargetTest, Finish1Test)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    mp_string id;
    mp_string taskId;
    mp_string name;
    mp_int32 ratio;
    mp_int32 order;
    TaskStepLinkTarget target(id, taskId, name, ratio ,order);
    Json::Value param;
    target.Finish(param);
}
