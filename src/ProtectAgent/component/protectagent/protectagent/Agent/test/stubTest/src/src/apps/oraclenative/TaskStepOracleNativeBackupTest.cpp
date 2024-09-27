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
#include "apps/oraclenative/TaskStepOracleNativeBackupTest.h"



TEST_F(TaskStepOracleNativeBackupTest, Init)
{
    Json::Value param;
    const mp_string id;
    const mp_string taskId;
    const mp_string name;
    mp_int32 ratio;
    mp_int32 order;
    mp_int32 iRet;
    TaskStepOracleNativeBackup work(id, taskId, name, ratio, order);

    iRet = work.Init(param);
    EXPECT_EQ(ERROR_COMMON_INVALID_PARAM, iRet);


    stub.set(ADDR(CJsonUtils,GetJsonString), StubCJsonUtilsGetJsonString);
    stub.set(ADDR(CJsonUtils,GetJsonInt32), StubCJsonUtilsGetJsonInt32);
    stub.set((bool (Json::Value::*)(const char *) const)ADDR(Json::Value, isMember), stub_return_bool_true);
    iRet = work.Init(param);
    EXPECT_EQ(ERROR_COMMON_INVALID_PARAM, iRet);

    stub.set(ADDR(TaskStepOracleNative,InitialDBInfo),stub_return_success);
    iRet = work.Init(param);
    EXPECT_EQ(MP_FAILED, iRet);
}

TEST_F(TaskStepOracleNativeBackupTest, Run)
{
    const mp_string id;
    const mp_string taskId;
    const mp_string name;
    mp_int32 ratio;
    mp_int32 order;
    mp_int32 iRet;
    TaskStepOracleNativeBackup work(id, taskId, name, ratio, order);
    stub.set(ADDR(TaskStep,RemoveParam), StubRemoveParam);

    iRet = work.Run();
    EXPECT_EQ(MP_FAILED, iRet);

    // stub.set(ADDR(TaskContext, GetValueInt32), StubCJsonUtilsGetJsonInt32);
    // iRet = work.Run();
    // EXPECT_EQ(MP_FAILED, iRet);

    // stub.set(ADDR(TaskStepOracleNativeBackup, BuildRmanBackupScriptParam), stub_return_success);
    // iRet = work.Run();
    // EXPECT_EQ(ERROR_AGENT_INTERNAL_ERROR, iRet);

    // stub.set(ADDR(TaskStepOracleNativeBackup, RunExecScript), stub_return_success);
    // iRet = work.Run();
    // EXPECT_EQ(MP_FAILED, iRet);

    // stub.set(ADDR(TaskStepOracleNativeBackup, AnalyseOracleBackupRst), stub_return_success);
    // iRet = work.Run();
    // EXPECT_EQ(MP_SUCCESS, iRet);
}

TEST_F(TaskStepOracleNativeBackupTest, RunExecScript)
{
    const mp_string id;
    const mp_string taskId;
    const mp_string name;
    mp_int32 ratio;
    mp_int32 order;
    mp_int32 iRet;
    int bakcupMode;
    mp_string param;
    std::vector<mp_string> vecResult;
    TaskStepOracleNativeBackup work(id, taskId, name, ratio, order);
    stub.set(ADDR(TaskStep,RemoveParam), StubRemoveParam);
    stub.set(ADDR(TaskContext, GetValueInt32), StubCJsonUtilsGetJsonInt32);

    bakcupMode = 1;
    iRet = work.RunExecScript(bakcupMode, param, vecResult);
    EXPECT_EQ(ERROR_AGENT_INTERNAL_ERROR, iRet);

    bakcupMode = 0;
    iRet = work.RunExecScript(bakcupMode, param, vecResult);
    EXPECT_EQ(ERROR_AGENT_INTERNAL_ERROR, iRet);
}

TEST_F(TaskStepOracleNativeBackupTest, Redo)
{
    const mp_string id;
    const mp_string taskId;
    const mp_string name;
    mp_int32 ratio;
    mp_int32 order;
    mp_int32 iRet;
    mp_string innerPID = "65535";
    TaskStepOracleNativeBackup work(id, taskId, name, ratio, order);

    iRet = work.Redo(innerPID);
    EXPECT_EQ(MP_FAILED, iRet);
}

TEST_F(TaskStepOracleNativeBackupTest, RefreshStepInfo)
{
    const mp_string id;
    const mp_string taskId;
    const mp_string name;
    mp_int32 ratio;
    mp_int32 order;
    mp_int32 iRet;
    TaskStepOracleNativeBackup work(id, taskId, name, ratio, order);

    stub.set(&CRootCaller::Exec, stub_return_success);
    iRet = work.RefreshStepInfo();
}

TEST_F(TaskStepOracleNativeBackupTest, BuildRmanBackupScriptParam)
{
    const mp_string id;
    const mp_string taskId;
    const mp_string name;
    mp_int32 ratio;
    mp_int32 order;
    mp_int32 iRet;
    mp_string param;

    TaskStepOracleNativeBackup work(id, taskId, name, ratio, order);

    iRet = work.BuildRmanBackupScriptParam(param);
    EXPECT_EQ(MP_FAILED, iRet);
}

TEST_F(TaskStepOracleNativeBackupTest, DataBackupHandler)
{
    const mp_string id;
    const mp_string taskId;
    const mp_string name;
    mp_int32 ratio;
    mp_int32 order;
    mp_int32 iRet;
    std::vector<mp_string> param;
    Json::Value backupRst;

    TaskStepOracleNativeBackup work(id, taskId, name, ratio, order);
    iRet = work.DataBackupHandler(param, backupRst);
    EXPECT_EQ(ERROR_COMMON_OPER_FAILED, iRet);

    param.push_back("test1");
    param.push_back("test2");
    param.push_back("test3");
    param.push_back("test4");
    iRet = work.DataBackupHandler(param, backupRst);
    EXPECT_EQ(MP_SUCCESS, iRet);
}

TEST_F(TaskStepOracleNativeBackupTest, LogBackupHandler)
{
    const mp_string id;
    const mp_string taskId;
    const mp_string name;
    mp_int32 ratio;
    mp_int32 order;
    mp_int32 iRet;
    std::vector<mp_string> param;
    Json::Value backupRst;

    param.push_back("test1");
    param.push_back("test2");
    TaskStepOracleNativeBackup work(id, taskId, name, ratio, order);
    iRet = work.LogBackupHandler(param, backupRst);
    EXPECT_EQ(ERROR_COMMON_OPER_FAILED, iRet);


    param.push_back("test3");
    param.push_back("test4");
    param.push_back("test5");
    param.push_back("test6");
    iRet = work.LogBackupHandler(param, backupRst);
    EXPECT_EQ(MP_SUCCESS, iRet);
}

TEST_F(TaskStepOracleNativeBackupTest, HistorySpeedBackupHandler)
{
    const mp_string id;
    const mp_string taskId;
    const mp_string name;
    mp_int32 ratio;
    mp_int32 order;
    mp_int32 iRet;
    std::vector<mp_string> param;
    Json::Value backupRst;

    param.push_back("test1");
    TaskStepOracleNativeBackup work(id, taskId, name, ratio, order);
    iRet = work.HistorySpeedBackupHandler(param, backupRst);
    EXPECT_EQ(ERROR_COMMON_OPER_FAILED, iRet);

    param.push_back("test3");
    iRet = work.HistorySpeedBackupHandler(param, backupRst);
    EXPECT_EQ(MP_SUCCESS, iRet);
}

TEST_F(TaskStepOracleNativeBackupTest, PfileParamBackupHandler)
{
    const mp_string id;
    const mp_string taskId;
    const mp_string name;
    mp_int32 ratio;
    mp_int32 order;
    mp_int32 iRet;
    std::vector<mp_string> param;
    Json::Value backupRst;

    param.push_back("test1");
    TaskStepOracleNativeBackup work(id, taskId, name, ratio, order);
    iRet = work.PfileParamBackupHandler(param, backupRst);
    EXPECT_EQ(ERROR_COMMON_OPER_FAILED, iRet);

    param.push_back("test2");
    iRet = work.PfileParamBackupHandler(param, backupRst);
    EXPECT_EQ(MP_SUCCESS, iRet);

    stub.set(ADDR(CIPCFile,ReadResult),StubCIPCFileReadResult);
    iRet = work.PfileParamBackupHandler(param, backupRst);
    EXPECT_EQ(MP_SUCCESS, iRet);
}

TEST_F(TaskStepOracleNativeBackupTest, ExtendParamBackupHandler)
{
    const mp_string id;
    const mp_string taskId;
    const mp_string name;
    mp_int32 ratio;
    mp_int32 order;
    mp_int32 iRet;
    std::vector<mp_string> param;
    Json::Value backupRst;

    param.push_back("test1");
    TaskStepOracleNativeBackup work(id, taskId, name, ratio, order);
    iRet = work.ExtendParamBackupHandler(param, backupRst);
    EXPECT_EQ(ERROR_COMMON_OPER_FAILED, iRet);

    param.push_back("test2");
    iRet = work.ExtendParamBackupHandler(param, backupRst);
    EXPECT_EQ(ERROR_COMMON_OPER_FAILED, iRet);
}

TEST_F(TaskStepOracleNativeBackupTest, AnalyseOracleBackupRst)
{
    const mp_string id;
    const mp_string taskId;
    const mp_string name;
    mp_int32 ratio;
    mp_int32 order;
    mp_int32 iRet;
    std::vector<mp_string> param;
    Json::Value backupRst;

    TaskStepOracleNativeBackup work(id, taskId, name, ratio, order);
    iRet = work.AnalyseOracleBackupRst(param, backupRst);
    EXPECT_EQ(ERROR_COMMON_OPER_FAILED, iRet);

    param.push_back("test1");
    param.push_back("test2;");
    iRet = work.AnalyseOracleBackupRst(param, backupRst);
    EXPECT_EQ(ERROR_COMMON_OPER_FAILED, iRet);
}

TEST_F(TaskStepOracleNativeBackupTest, Cancel)
{
    const mp_string id;
    const mp_string taskId;
    const mp_string name;
    mp_int32 ratio;
    mp_int32 order;
    mp_int32 iRet;

    TaskStepOracleNativeBackup work(id, taskId, name, ratio, order);
    iRet = work.Cancel();
    EXPECT_EQ(ERROR_COMMON_SCRIPT_EXEC_FAILED, iRet);
}

TEST_F(TaskStepOracleNativeBackupTest, BackupLevelBackupHandler)
{
    const mp_string id;
    const mp_string taskId;
    const mp_string name;
    mp_int32 ratio;
    mp_int32 order;
    mp_int32 iRet;
    std::vector<mp_string> param;
    Json::Value backupRst;

    TaskStepOracleNativeBackup work(id, taskId, name, ratio, order);
    iRet = work.BackupLevelBackupHandler(param, backupRst);
    EXPECT_EQ(ERROR_COMMON_OPER_FAILED, iRet);

    param.push_back("test1");
    param.push_back("test2;");
    iRet = work.BackupLevelBackupHandler(param, backupRst);
    EXPECT_EQ(MP_SUCCESS, iRet);
}

TEST_F(TaskStepOracleNativeBackupTest, FilelistHandler)
{
    const mp_string id;
    const mp_string taskId;
    const mp_string name;
    mp_int32 ratio;
    mp_int32 order;
    mp_int32 iRet;
    std::vector<mp_string> param;
    Json::Value backupRst;

    TaskStepOracleNativeBackup work(id, taskId, name, ratio, order);
    iRet = work.FilelistHandler(param, backupRst);
    EXPECT_EQ(ERROR_COMMON_OPER_FAILED, iRet);

    param.push_back("test1");
    param.push_back("test2;");
    iRet = work.FilelistHandler(param, backupRst);
    EXPECT_EQ(ERROR_COMMON_OPER_FAILED, iRet);

    stub.set(ADDR(CIPCFile,ReadResult),StubCIPCFileReadResult);
    iRet = work.FilelistHandler(param, backupRst);
    EXPECT_EQ(ERROR_COMMON_OPER_FAILED, iRet);
}

TEST_F(TaskStepOracleNativeBackupTest, UpdateOracleDbInfo)
{
    const mp_string id;
    const mp_string taskId;
    const mp_string name;
    mp_int32 ratio;
    mp_int32 order;
    mp_int32 iRet;
    std::vector<mp_string> param;
    Json::Value backupRst;

    TaskStepOracleNativeBackup work(id, taskId, name, ratio, order);
    stub.set(ADDR(TaskStepOracleNative, InitialDBInfo), StubInitialDBInfo);
    work.UpdateOracleDbInfo();
}