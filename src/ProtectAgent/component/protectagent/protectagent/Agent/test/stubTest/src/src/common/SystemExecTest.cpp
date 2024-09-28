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
#include "common/SystemExecTest.h"
#include <vector>

using namespace std;
static mp_int32 StubExecSystemWithoutEcho(mp_string strLogCmd, mp_string& strCommand, mp_bool bNeedRedirect){
    return 0;
}

static mp_void StubCLoggerLog(mp_void){
    return;
}

TEST_F(CSystemExecTest,ExecSystemWithoutEchoTest){
    stub.set(&CLogger::Log, StubCLoggerLog);
    mp_string strCommand = "test";
    mp_bool bNeedRedirect = MP_TRUE;
    
    stub.set(CheckCmdDelimiter, CSystemExecTestStubCheckCmdDelimiter);
    CSystemExec::ExecSystemWithoutEcho(strCommand,bNeedRedirect);
    CSystemExec::ExecSystemWithoutEcho(strCommand, "", bNeedRedirect);
}

TEST_F(CSystemExecTest,ExecSystemWithEcho){
    stub.set(&CLogger::Log, StubCLoggerLog);
    mp_string strCommand = "test";
    vector<mp_string> strEcho;
    mp_bool bNeedRedirect = MP_TRUE;
    
    CSystemExec::ExecSystemWithEcho(strCommand,strEcho,bNeedRedirect);
    
    stub.set(CheckCmdDelimiter, CSystemExecTestStubCheckCmdDelimiter);
    CSystemExec::ExecSystemWithEcho(strCommand,strEcho,bNeedRedirect);
}

TEST_F(CSystemExecTest,ExecSystemWithoutEchoEnvNoWinTest){
    stub.set(&CLogger::Log, StubCLoggerLog);
    mp_string strLogCmd;
    mp_string strCommand;
    mp_string strEnv;
    mp_bool bNeedRedirect;
    CSystemExec::ExecSystemWithoutEchoEnvNoWin(strLogCmd, strCommand, strEnv, bNeedRedirect);

}
