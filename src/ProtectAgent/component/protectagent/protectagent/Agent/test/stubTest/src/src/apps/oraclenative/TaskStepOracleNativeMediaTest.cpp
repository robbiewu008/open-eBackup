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
#include "apps/oraclenative/TaskStepOracleNativeMediaTest.h"

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
};


TEST_F(TaskStepOracleNativeMediaTest, mediaInit)
{
    DoGetJsonStringTest();
    Json::Value param;
    mp_string id;
    mp_string taskId;
    mp_string name;
    mp_int32 ratio;
    mp_int32 order;

    TaskStepOracleNativeMedia mediaData(id, taskId, name, ratio, order);
    mediaData.Init(param);
    param["taskType"] = 1;
    param["params"] = "21";
    param["storage"] = "11";
    mediaData.Init(param);
}

TEST_F(TaskStepOracleNativeMediaTest, mediaRun)
{
    DoGetJsonStringTest();
    Json::Value param;
    mp_string id;
    mp_string taskId;
    mp_string name;
    mp_int32 ratio;
    mp_int32 order;

    TaskStepOracleNativeMedia mediaData(id, taskId, name, ratio, order);
    mediaData.Run();
}


TEST_F(TaskStepOracleNativeMediaTest, mediaPrepareMediumInfo)
{
    DoGetJsonStringTest();
    Json::Value param;
    mp_string id;
    mp_string taskId;
    mp_string name;
    mp_int32 ratio;
    mp_int32 order;

    TaskStepOracleNativeMedia mediaData(id, taskId, name, ratio, order);
    mediaData.PrepareMediumInfo();
}

TEST_F(TaskStepOracleNativeMediaTest, mediaInitialVolumes)
{
    DoGetJsonStringTest();
    Json::Value param;
    mp_string id;
    mp_string taskId;
    mp_string name;
    mp_int32 ratio;
    mp_int32 order;

    TaskStepOracleNativeMedia mediaData(id, taskId, name, ratio, order);
    mp_string diskList;
    mediaData.InitialVolumes(diskList);
}

TEST_F(TaskStepOracleNativeMediaTest, mediaPrepareBackupMedium)
{
    DoGetJsonStringTest();
    Json::Value param;
    mp_string id;
    mp_string taskId;
    mp_string name;
    mp_int32 ratio;
    mp_int32 order;

    TaskStepOracleNativeMedia mediaData(id, taskId, name, ratio, order);
    mp_string diskList;
    mediaData.PrepareBackupMedium(diskList);
}
