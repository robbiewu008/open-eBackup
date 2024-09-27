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
#include "taskmanager/TaskStepScriptTest.h"
#include "taskmanager/TaskStepScript.h"
#include "common/CSystemExec.h"
#include "securecom/SecureUtils.h"
namespace {
mp_int32 StubCConfigXmlParserGetValueInt32ReturnSuccess(mp_void* pthis, const mp_string& strSection, const mp_string& strKey, mp_int32& iValue)
{
    return MP_SUCCESS;
}
#define StubClogToVoidLogNullPointReference() do { \
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_int32&))ADDR(CConfigXmlParser,GetValueInt32), StubCConfigXmlParserGetValueInt32ReturnSuccess); \
} while (0)

static mp_void StubCLoggerLog(mp_void){
    return;
}

mp_int32 ReadFileTest(const mp_string& strFilePath, std::vector<mp_string>& vecOutput)
{
    vecOutput.push_back("aaaaa");
    return MP_SUCCESS;
}

mp_void StrTokenTest(const mp_string& strToken, const mp_string& strSeparator, std::list<mp_string>& plstStr)
{
    plstStr.push_back("aaaa");
    plstStr.push_back("aaaa");
    plstStr.push_back("aaaa");
}

mp_int32 ExecScriptTest(const mp_string& strScriptFileName, const mp_string& strParam,
    std::vector<mp_string> pvecResult[], mp_bool bNeedCheckSign, pFunc cb)
{
    return MP_FAILED;
}

mp_int32 ExecTest(mp_void* pThis, mp_int32 iCommandID, mp_string strParam, std::vector<mp_string> pvecResult[],
    mp_int32 (*cb)(void*, const mp_string&), void* pTaskStep)
    {
        return MP_SUCCESS;
    }
}

mp_int32 WriteInputTest(const mp_string& strUniqueID, const mp_string& strInput)
{
    return MP_FAILED;
}

TEST_F(TaskStepScriptTest, InitTest)
{
    mp_string id;
    const mp_string taskId;
    mp_string name;
    mp_int32 ratio;
    mp_int32 order;
    TaskStepScript script(id, taskId, name, ratio, order);
    Json::Value param;
    script.Init(param);
}

TEST_F(TaskStepScriptTest, SetRunModeTest)
{
    mp_string id;
    const mp_string taskId;
    mp_string name;
    mp_int32 ratio;
    mp_int32 order;
    TaskStepScript script(id, taskId, name, ratio, order);
    ScriptRunMode param;
    script.SetRunMode(param);
}

TEST_F(TaskStepScriptTest, InitBuiltInWithRdAdminExecTest)
{
    mp_string id;
    const mp_string taskId;
    mp_string name;
    mp_int32 ratio;
    mp_int32 order;
    TaskStepScript script(id, taskId, name, ratio, order);
    mp_string startScript;
    mp_string startScriptParam;
    mp_string stopScript;
    mp_string stopScriptParam;
    script.InitBuiltInWithRdAdminExec(startScript, startScriptParam, stopScript, stopScriptParam);
}

TEST_F(TaskStepScriptTest, InitBuiltInWithRootExecTest)
{
    mp_string id;
    const mp_string taskId;
    mp_string name;
    mp_int32 ratio;
    mp_int32 order;
    TaskStepScript script(id, taskId, name, ratio, order);
    mp_int32 startScript;
    mp_string startScriptParam;
    mp_int32 stopScript;
    mp_string stopScriptParam;
    script.InitBuiltInWithRootExec(startScript, startScriptParam, stopScript, stopScriptParam);
}

TEST_F(TaskStepScriptTest, Init3rdScriptTest)
{
    mp_string id;
    const mp_string taskId;
    mp_string name;
    mp_int32 ratio;
    mp_int32 order;
    TaskStepScript script(id, taskId, name, ratio, order);
    mp_string startScript;
    mp_string startScriptParam;
    mp_string stopScript;
    mp_string stopScriptParam;
    script.Init3rdScript(startScript, startScriptParam, stopScript, stopScriptParam);
}

TEST_F(TaskStepScriptTest, RunTest)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    stub.set(ADDR(CRootCaller, Exec), ExecTest);
    mp_string id;
    const mp_string taskId;
    mp_string name;
    mp_int32 ratio;
    mp_int32 order;
    TaskStepScript script(id, taskId, name, ratio, order);

    EXPECT_EQ(MP_SUCCESS, script.Run());
    mp_string startScript = "1111";
    mp_string startScriptParam;
    mp_int32 stopScript;
    mp_string stopScriptParam;
    mp_int32 startScriptId;
    script.InitBuiltInWithRootExec(startScriptId, startScriptParam, stopScript, stopScriptParam);
    script.Run();
    mp_string stopScriptSt;
    script.Init3rdScript(startScript, startScriptParam, stopScriptSt, stopScriptParam, false);
    script.Run();
    script.is3rdScript = false;
    script.InitBuiltInWithRootExec(startScriptId, startScriptParam, stopScript, stopScriptParam);
    script.Run();
    mp_string stopScriptaa;
    script.InitBuiltInWithRdAdminExec(startScript, startScriptParam, stopScriptaa, stopScriptParam);
    script.Run();
}

TEST_F(TaskStepScriptTest, CancelTest)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    mp_string id;
    const mp_string taskId;
    mp_string name;
    mp_int32 ratio;
    mp_int32 order;
    TaskStepScript script(id, taskId, name, ratio, order);
    EXPECT_EQ(MP_FAILED, script.Cancel());
    mp_string startScript = "1111";
    mp_string startScriptParam;
    mp_int32 stopScript;
    mp_string stopScriptParam;
    mp_int32 startScriptId;
    script.m_stopScript = "aaa";
    script.InitBuiltInWithRootExec(startScriptId, startScriptParam, stopScript, stopScriptParam);
    script.Cancel();
    mp_string stopScriptSt;
    script.Init3rdScript(startScript, startScriptParam, stopScriptSt, stopScriptParam, false);
    script.Cancel();
    script.is3rdScript = false;
    script.InitBuiltInWithRootExec(startScriptId, startScriptParam, stopScript, stopScriptParam);
    script.Cancel();
    script.InitBuiltInWithRdAdminExec(startScript, startScriptParam, stopScriptSt, stopScriptParam);
    script.Cancel();
}

TEST_F(TaskStepScriptTest, CancelJsonTest)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    stub.set(ADDR(CRootCaller, Exec), ExecTest);
    mp_string id;
    const mp_string taskId;
    mp_string name;
    mp_int32 ratio;
    mp_int32 order;
    TaskStepScript script(id, taskId, name, ratio, order);
    Json::Value respParam;
    script.Cancel(respParam);
    mp_string startScript = "1111";
    mp_string startScriptParam;
    mp_int32 stopScript;
    mp_string stopScriptParam;
    mp_int32 startScriptId;
    script.m_stopScript = "aaa";
    script.InitBuiltInWithRootExec(startScriptId, startScriptParam, stopScript, stopScriptParam);
    script.Cancel(respParam);
    mp_string stopScriptSt;
    script.Init3rdScript(startScript, startScriptParam, stopScriptSt, stopScriptParam, false);
    script.Cancel(respParam);
    script.is3rdScript = false;
    script.InitBuiltInWithRootExec(startScriptId, startScriptParam, stopScript, stopScriptParam);
    script.Cancel(respParam);
    script.InitBuiltInWithRdAdminExec(startScript, startScriptParam, stopScriptSt, stopScriptParam);
    script.Cancel(respParam);
}

TEST_F(TaskStepScriptTest, StopTest)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    mp_string id;
    const mp_string taskId;
    mp_string name;
    mp_int32 ratio;
    mp_int32 order;
    TaskStepScript script(id, taskId, name, ratio, order);
    Json::Value param;
    EXPECT_EQ(MP_FAILED, script.Stop(param));
}

TEST_F(TaskStepScriptTest, RedoTest)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    mp_string id;
    const mp_string taskId;
    mp_string name;
    mp_int32 ratio;
    mp_int32 order;
    TaskStepScript script(id, taskId, name, ratio, order);
    mp_string innerPID;
    EXPECT_EQ(MP_SUCCESS, script.Redo(innerPID));
}

TEST_F(TaskStepScriptTest, Update23Test)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    mp_string id;
    const mp_string taskId;
    mp_string name;
    mp_int32 ratio;
    mp_int32 order;
    TaskStepScript script(id, taskId, name, ratio, order);
    Json::Value param;

    EXPECT_EQ(MP_SUCCESS, script.Update(param));
}

TEST_F(TaskStepScriptTest, FinishzzTest)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    mp_string id;
    const mp_string taskId;
    mp_string name;
    mp_int32 ratio;
    mp_int32 order;
    TaskStepScript script(id, taskId, name, ratio, order);
    Json::Value param;
    EXPECT_EQ(MP_SUCCESS, script.Finish(param));
}

TEST_F(TaskStepScriptTest, RefreshStepInfoTest)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    mp_string id;
    const mp_string taskId;
    mp_string name;
    mp_int32 ratio;
    mp_int32 order;
    TaskStepScript script(id, taskId, name, ratio, order);
    Json::Value param;
    EXPECT_EQ(MP_FAILED, script.RefreshStepInfo());
    stub.set(ADDR(CMpFile, ReadFile), ReadFileTest);
    stub.set(ADDR(CMpString, StrToken), StrTokenTest);
    EXPECT_EQ(MP_SUCCESS, script.RefreshStepInfo());
}

TEST_F(TaskStepScriptTest, UpdateTaskStatusFileTest)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    mp_string id;
    const mp_string taskId;
    mp_string name;
    mp_int32 ratio;
    mp_int32 order;
    TaskStepScript script(id, taskId, name, ratio, order);
    stub.set(ADDR(CIPCFile, WriteInput), WriteInputTest);
    EXPECT_EQ(MP_FAILED, script.UpdateTaskStatusFile());
}

TEST_F(TaskStepScriptTest, RunScriptByRdAdminTest)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    mp_string id;
    const mp_string taskId;
    mp_string name;
    mp_int32 ratio;
    mp_int32 order;
    TaskStepScript script(id, taskId, name, ratio, order);
    mp_string scriptName;
    mp_string param;
    EXPECT_EQ(MP_FAILED, script.RunScriptByRdAdmin(scriptName, param));
    scriptName = "aaaa";
    EXPECT_EQ(MP_SUCCESS, script.RunScriptByRdAdmin(scriptName, param));
    script.RunScriptByRdAdmin(scriptName, param);
    stub.set(SecureCom::SysExecScript, ExecScriptTest);
    script.RunScriptByRdAdmin(scriptName, param);
}

TEST_F(TaskStepScriptTest, RunScriptByRootTest)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    mp_string id;
    const mp_string taskId;
    mp_string name;
    mp_int32 ratio;
    mp_int32 order;
    TaskStepScript script(id, taskId, name, ratio, order);
    mp_int32 scriptName = 0;
    mp_string param;
    EXPECT_EQ(MP_FAILED, script.RunScriptByRoot(scriptName, param));
    scriptName = 1;
    EXPECT_EQ(MP_FAILED, script.RunScriptByRoot(scriptName, param));
    stub.set(ADDR(CRootCaller, Exec), ExecTest);
    EXPECT_EQ(MP_SUCCESS, script.RunScriptByRoot(scriptName, param));
}