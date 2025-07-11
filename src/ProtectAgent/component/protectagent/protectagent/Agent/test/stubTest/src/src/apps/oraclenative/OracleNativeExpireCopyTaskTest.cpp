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
#include "apps/oraclenative/OracleNativeExpireCopyTaskTest.h"
#include "taskmanager/TaskManager.h"
using namespace std;


namespace {
mp_int32 ExecTesta(mp_void* pThis, mp_int32 iCommandID, mp_string strParam, vector<mp_string> pvecResult[],
    mp_int32 (*cb)(void*, const mp_string&), void* pTaskStep)
{
    return MP_SUCCESS;
}

mp_void LogTest(const mp_int32 iLevel, const mp_int32 iFileLine, const mp_string& pszFileName,
    const mp_string& pszFuncction, const mp_string& pszFormat, ...) {}
#define DoGetJsonStringTest() do { \
    stub.set(ADDR(CLogger, Log), LogTest); \
} while (0)

mp_int32 InitTaskStepParamTest(const Json::Value& param, const mp_string& paramKey, const mp_string& stepName)
{
    return MP_SUCCESS;
}
mp_int32 QueryTaskInfoTest(const mp_string& taskID, const mp_string& taskStep, taskinfo_t& taskInfo)
{
    return MP_SUCCESS;
}
};

TEST_F(OracleNativeExpireCopyTaskTest, InitTaskStepStub)
{
    mp_string taskID = "1";
    OracleNativeExpireCopyTask task(taskID);
    Json::Value param;
    task.InitTaskStep(param);
    stub.set(ADDR(Task, InitTaskStepParam), InitTaskStepParamTest);
    task.InitTaskStep(param);
    stub.reset(ADDR(Task, InitTaskStepParam));
}

TEST_F(OracleNativeExpireCopyTaskTest, CreateTaskStepStub)
{
    mp_string taskID = "1";
    OracleNativeExpireCopyTask task(taskID);
    task.CreateTaskStep();
}

TEST_F(OracleNativeExpireCopyTaskTest, CreateRedoTaskStub)
{
    mp_string taskID = "1";
    mp_string redoTaskID = "2";
    OracleNativeExpireCopyTask task(taskID);
    task.CreateRedoTask(redoTaskID);
    stub.set(ADDR(TaskManager, QueryTaskInfo), QueryTaskInfoTest);
    task.CreateRedoTask(redoTaskID);
    stub.reset(ADDR(TaskManager, QueryTaskInfo));
}
