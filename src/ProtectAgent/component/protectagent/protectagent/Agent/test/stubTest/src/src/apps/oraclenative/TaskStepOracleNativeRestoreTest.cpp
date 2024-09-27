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
#include "apps/oraclenative/TaskStepOracleNativeRestoreTest.h"

using namespace std;
namespace {
mp_int32 ExecTesta(mp_void* pThis, mp_int32 iCommandID, mp_string strParam, vector<mp_string> pvecResult[],
    mp_int32 (*cb)(void*, const mp_string&), void* pTaskStep)
{
    pvecResult->push_back("111");
    return MP_SUCCESS;
}

mp_void LogTest(const mp_int32 iLevel, const mp_int32 iFileLine, const mp_string& pszFileName,
    const mp_string& pszFuncction, const mp_string& pszFormat, ...) {}
#define DoGetJsonStringTest() do { \
    stub.set(ADDR(CLogger, Log), LogTest); \
} while (0)

mp_int32 BuildRestoreScriptParamSub(mp_string& strParam)
{
    return MP_SUCCESS;
}

mp_int32 StubSuccess(mp_void* pThis)
{
    return MP_SUCCESS;
}
};


TEST_F(TaskStepOracleNativeRestoreTest, RestoreInitTest)
{
    DoGetJsonStringTest();
    Json::Value param;
    mp_string id;
    mp_string taskId;
    mp_string name;
    mp_int32 ratio;
    mp_int32 order;
    TaskStepOracleNativeRestore dataBase(id, taskId, name, ratio , order);
    dataBase.Init(param);
    param["taskType"] = 1;
    param["params"] = "21";
    param["storage"] = "11";
    dataBase.Init(param);
}

TEST_F(TaskStepOracleNativeRestoreTest, RestoreCancelTest)
{
    DoGetJsonStringTest();
    Json::Value param;
    mp_string id;
    mp_string taskId;
    mp_string name;
    mp_int32 ratio;
    mp_int32 order;
    TaskStepOracleNativeRestore dataBase(id, taskId, name, ratio , order);
    // dataBase.Cancel();
    // stub.set(ADDR(CRootCaller, Exec), ExecTesta);
    // dataBase.Cancel();
    // stub.reset(ADDR(CRootCaller, Exec));
}

TEST_F(TaskStepOracleNativeRestoreTest, RestoreRunTest)
{
    DoGetJsonStringTest();
    Json::Value param;
    mp_string id;
    mp_string taskId;
    mp_string name;
    mp_int32 ratio;
    mp_int32 order;
    TaskStepOracleNativeRestore dataBase(id, taskId, name, ratio , order);
    dataBase.Run();
    stub.set(ADDR(TaskStepOracleNativeRestore, BuildRestoreScriptParam), BuildRestoreScriptParamSub);
    dataBase.Run();
    // stub.set(ADDR(CRootCaller, Exec), ExecTesta);
    // dataBase.Run();
    // stub.reset(ADDR(TaskStepOracleNativeRestore, BuildRestoreScriptParam));
    // stub.reset(ADDR(CRootCaller, Exec));
}

TEST_F(TaskStepOracleNativeRestoreTest, RestoreBuildRestoreScriptParamTest)
{
    DoGetJsonStringTest();
    Json::Value param;
    mp_string id;
    mp_string taskId;
    mp_string name;
    mp_int32 ratio;
    mp_int32 order;
    TaskStepOracleNativeRestore dataBase(id, taskId, name, ratio , order);
    mp_string strParam;
    dataBase.BuildRestoreScriptParam(strParam);
}

TEST_F(TaskStepOracleNativeRestoreTest, RestoreRefreshStepInfoTest)
{
    DoGetJsonStringTest();
    Json::Value param;
    mp_string id;
    mp_string taskId;
    mp_string name;
    mp_int32 ratio;
    mp_int32 order;
    TaskStepOracleNativeRestore dataBase(id, taskId, name, ratio , order);
    stub.set(ADDR(CRootCaller, Exec), ExecTesta);
    stub.set(ADDR(TaskStepOracleNativeRestore, BuildRestoreScriptParam), BuildRestoreScriptParamSub);
    stub.set(ADDR(TaskStepOracleNative, AnalyseQueryRmanStatusScriptRst), StubSuccess);
    stub.set(ADDR(TaskStepOracleNativeRestore, GetHistorySpeed), StubSuccess);
    dataBase.RefreshStepInfo();
}

TEST_F(TaskStepOracleNativeRestoreTest, RestoreGetHistorySpeedTest)
{
    DoGetJsonStringTest();
    Json::Value param;
    mp_string id;
    mp_string taskId;
    mp_string name;
    mp_int32 ratio;
    mp_int32 order;
    TaskStepOracleNativeRestore dataBase(id, taskId, name, ratio , order);
    std::vector<mp_string> vecResult;
    dataBase.GetHistorySpeed(vecResult);
    vecResult.push_back("111");
    dataBase.GetHistorySpeed(vecResult);
}
