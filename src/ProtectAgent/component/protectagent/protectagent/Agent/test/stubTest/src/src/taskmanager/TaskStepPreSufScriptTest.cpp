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
#include "taskmanager/TaskStepPreSufScript.h"
#include "taskmanager/TaskStepPreSufScriptTest.h"

static mp_void StubCLoggerLog(mp_void){
    return;
}

TEST_F(TaskStepPreScriptTest, TaskStepPreScriptTestInitTest)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    mp_string id;
    mp_string taskId;
    mp_string name;
    mp_int32 ratio;
    mp_int32 order;
    TaskStepPreScript script(id, taskId, name, ratio, order);
    Json::Value param;
    script.Init(param);
    param["preScript"] = "preScript";
    script.Init(param);
}

TEST_F(TaskStepPreScriptTest, TaskStepPostScriptInitTest)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    mp_string id;
    mp_string taskId;
    mp_string name;
    mp_int32 ratio;
    mp_int32 order;
    TaskStepPostScript script(id, taskId, name, ratio, order);
    Json::Value param;
    script.Init(param);
    param["postScript"] = "postScript";
    script.Init(param);
}

TEST_F(TaskStepPreScriptTest, TaskStepFailPostScriptInitTest)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    mp_string id;
    mp_string taskId;
    mp_string name;
    mp_int32 ratio;
    mp_int32 order;
    TaskStepFailPostScript script(id, taskId, name, ratio, order);
    Json::Value param;
    script.Init(param);
    param["failPostScript"] = "failPostScript";
    script.Init(param);
}