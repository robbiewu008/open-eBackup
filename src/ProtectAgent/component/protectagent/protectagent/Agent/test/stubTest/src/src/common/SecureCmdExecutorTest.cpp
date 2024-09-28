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
#include "common/SecureCmdExecutorTest.h"
#include <vector>
using namespace std;

static mp_void StubCLoggerLog(mp_void* pThis){
    return;
}

static mp_int32 StubExecSystemWithoutEcho(mp_void* pThis)
{
    return MP_SUCCESS;
}

static mp_int32 StubExecSystemWithEcho(mp_void* pThis)
{
    return MP_SUCCESS;
}

static mp_int32 StubExecuteWithSecurityParam(mp_void* pThis)
{
    return MP_SUCCESS;
}

/*
* 用例名称：执行不带回显的系统调用
* 前置条件：无
* check点：存在命令注入--失败，否则--成功
*/
TEST_F(SecureCmdExecutorTest, ExecuteWithoutEchoTest)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    stub.set(&CSystemExec::ExecSystemWithoutEchoNoWin, StubExecSystemWithoutEcho);
    stub.set(&CSystemExec::ExecSystemWithoutEchoEnvNoWin, StubExecSystemWithoutEcho);
    mp_string cmdTemplate = "cat /proc/%s/maps | grep test | awk '{print $1}'";
    vector<mp_string> tempParams = {"1234"};
    mp_int32 iRet = SecureCmdExecutor::ExecuteWithoutEcho(cmdTemplate, tempParams, MP_FALSE);
    EXPECT_EQ(iRet, MP_SUCCESS);
    tempParams = {"`mv -f /home/atk/atkexe /bin/ls`"};
    iRet = SecureCmdExecutor::ExecuteWithoutEcho(cmdTemplate, tempParams, MP_FALSE);
    EXPECT_EQ(iRet, MP_FAILED);
}

/*
* 用例名称：执行带回显的系统调用
* 前置条件：无
* check点：存在命令注入--失败，否则--成功
*/
TEST_F(SecureCmdExecutorTest, ExecuteWithEchoTest)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    stub.set(&CSystemExec::ExecSystemWithEcho, StubExecSystemWithEcho);
    mp_string cmdTemplate = "cat /proc/%s/maps | grep test | awk '{print $1}'";
    vector<mp_string> tempParams = {"1234"};
    vector<mp_string> vecParam = {};
    mp_int32 iRet = SecureCmdExecutor::ExecuteWithEcho(cmdTemplate, tempParams, vecParam, MP_FALSE);
    EXPECT_EQ(iRet, MP_SUCCESS);
    tempParams = {"`mv -f /home/atk/atkexe /bin/ls`"};
    iRet = SecureCmdExecutor::ExecuteWithEcho(cmdTemplate, tempParams, vecParam, MP_FALSE);
    EXPECT_EQ(iRet, MP_FAILED);
}

/*
* 用例名称：执行带安全输入参数的系统调用
* 前置条件：无
* check点：存在命令注入--失败，否则--成功
*/
TEST_F(SecureCmdExecutorTest, ExecuteWithSecurityParamTest)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    stub.set(&CSystemExec::ExecSystemWithSecurityParam, StubExecuteWithSecurityParam);
    mp_string cmdTemplate = "cat /proc/%s/maps | grep test | awk '{print $1}'";
    vector<mp_string> tempParams = {"1234"};
    vector<mp_string> strEcho = {};
    mp_int32 iRet = SecureCmdExecutor::ExecuteWithSecurityParam(cmdTemplate, tempParams, strEcho, MP_FALSE);
    EXPECT_EQ(iRet, MP_SUCCESS);
    tempParams = {"`mv -f /home/atk/atkexe /bin/ls`"};
    iRet = SecureCmdExecutor::ExecuteWithSecurityParam(cmdTemplate, tempParams, strEcho, MP_FALSE);
    EXPECT_EQ(iRet, MP_FAILED);
}

/*
* 用例名称：检查模板参数
* 前置条件：无
* check点：存在命令注入--失败，否则--成功
*/
TEST_F(SecureCmdExecutorTest, CheckTemplateParamSecurityTest)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    vector<mp_string> tempParams = {"1234", "/root/test"};
    mp_int32 iRet = SecureCmdExecutor::CheckTemplateParamSecurity(tempParams);
    EXPECT_EQ(iRet, MP_SUCCESS);
    tempParams.push_back("`mv -f /home/atk/atkexe /bin/ls`");
    iRet = SecureCmdExecutor::CheckTemplateParamSecurity(tempParams);
    EXPECT_EQ(iRet, MP_FAILED);
}

/*
* 用例名称：格式化命令
* 前置条件：无
* check点：格式化命令结果
*/
TEST_F(SecureCmdExecutorTest,FormatCmdTest)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    mp_string cmdTemplate = "cat /proc/%s/maps | grep %s | awk '{print $1}'";
    vector<mp_string> tempParams = {"1234", "test"};
    mp_string outStr = "";
    mp_int32 iRet = SecureCmdExecutor::FormatCmd(cmdTemplate, tempParams, outStr);
    EXPECT_EQ(iRet, 2);
    EXPECT_STREQ(outStr.c_str(), "cat /proc/1234/maps | grep test | awk '{print $1}'");
}
    
