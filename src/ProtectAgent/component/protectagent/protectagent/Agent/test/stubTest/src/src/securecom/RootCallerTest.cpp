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
#include "securecom/RootCallerTest.h"

namespace {
static mp_void StubCLoggerLog(mp_void){
    return;
}

mp_int32 StubSuccess(mp_void* pthis)
{
    return MP_SUCCESS;
}

mp_int32 StubFailed(mp_void* pthis)
{
    return MP_FAILED;
}

mp_bool StubTrue(mp_void* pthis)
{
    return MP_TRUE;
}

mp_bool StubFalse(mp_void* pthis)
{
    return MP_FALSE;
}
FILE* StubPopen(mp_void* pthis)
{
    return nullptr;
}

mp_int32 StubFailedTwo(mp_void* pthis){
    static mp_int32 flag = 0;
    if (flag ++ < 1) {
        return MP_SUCCESS;
    }
    return MP_FAILED;
}

mp_int32 StubFailedThree(mp_void* pthis){
    static mp_int32 flag = 0;
    if (flag ++ < 2) {
        return MP_SUCCESS;
    }
    return MP_FAILED;
}

mp_int32 StubFailedFour(mp_void* pthis){
    static mp_int32 flag = 0;
    if (flag ++ < 3) {
        return MP_SUCCESS;
    }
    return MP_FAILED;
}
mp_int32 UpdateInnerPIDCallBack(void* pPointer, const mp_string& innerPID)
{
    return MP_FAILED;
}
}
static mp_int32 StubCConfigXmlParserGetValueInt32ReturnSuccess(mp_void* pthis, mp_string strSection, mp_string strKey, mp_int32& iValue)
{
    return MP_SUCCESS;
}
#define StubClogToVoidLogNullPointReference() do { \
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_int32&))ADDR(CConfigXmlParser,GetValueInt32), StubCConfigXmlParserGetValueInt32ReturnSuccess); \
} while (0)

mp_int32 StubReadResultFile(mp_int32 iCommandID, mp_string& strUniqueID, vector<mp_string> pvecResult[])
{
    return MP_SUCCESS;
}

mp_int32 StubWriteInputSuccess(mp_string& strUniqueID, mp_string& strInput)
{
    return MP_SUCCESS;
}

mp_int32 StubDirExistSuccess(const mp_char* pszDirPath)
{
    return MP_TRUE;
}

mp_int32 StubDirExistFail(const mp_char* pszDirPath)
{
    return MP_FALSE;
}

mp_int32 StubExecSuccess(mp_int32 iCommandID, mp_string strParam, vector<mp_string> pvecResult[],
    mp_int32 (*cb)(void*, const mp_string&), void* pTaskStep)
{
    return MP_SUCCESS;
}

mp_int32 StubExecFail(mp_int32 iCommandID, mp_string strParam, vector<mp_string> pvecResult[],
    mp_int32 (*cb)(void*, const mp_string&), void* pTaskStep)
{
    return MP_FAILED;
}

mp_int32 StubWaitForEnd(thread_id_t* id, mp_void** retValue)
{
    return MP_SUCCESS;
}

/*
* 用例名称：获得命令字符串
* 前置条件：无
* check点：无
*/
TEST_F(CRootCallerTest, GetSudoCmdStr) {
    stub.set(&CLogger::Log, StubCLoggerLog);
    CRootCaller rootCaller;
    mp_int32 iCommandID;
    mp_string scriptCmd;
    {
        stub.set(&CMpFile::FileExist, StubFalse);
        rootCaller.ExecUserDefineScript(iCommandID, scriptCmd);
    }
}

/*
* 用例名称：root权限执行用户指定的脚本
* 前置条件：无
* check点：检查返回值
*/
TEST_F(CRootCallerTest, ExecUserDefineScript) {
    stub.set(&CLogger::Log, StubCLoggerLog);
    CRootCaller rootCaller;
    mp_int32 iRet;
    mp_int32 iCommandID;
    mp_string scriptCmd;

    stub.set(CheckCmdDelimiter, StubFailed);
    iRet = rootCaller.ExecUserDefineScript(iCommandID, scriptCmd);
    EXPECT_EQ(iRet, MP_FAILED);

    stub.set(CheckCmdDelimiter, StubSuccess);
    stub.set(&CSystemExec::ExecSystemWithoutEchoNoWin, StubSuccess);
    iRet = rootCaller.ExecUserDefineScript(iCommandID, scriptCmd);
    EXPECT_EQ(iRet, MP_SUCCESS);

    stub.set(&CSystemExec::ExecSystemWithoutEchoNoWin, StubFailed);
    iRet = rootCaller.ExecUserDefineScript(iCommandID, scriptCmd);
    EXPECT_EQ(iRet, MP_FAILED);
}

/*
* 用例名称：读取结果文件
* 前置条件：无
* check点：检查返回值
*/
TEST_F(CRootCallerTest, ReadResultFile) {
    stub.set(&CLogger::Log, StubCLoggerLog);
    CRootCaller rootCaller;
    mp_int32 iRet;
    mp_int32 iCommandID;
    mp_string strUniqueID;
    vector<mp_string> pvecResult;

    iCommandID = ROOT_COMMAND_SCRIPT_ORACLENATIVE_BACKUPDATA;
    stub.set(&ErrorCode::GetSpecCommonID, StubSuccess);
    stub.set(&CMpFile::FileExist, StubFalse);
    iRet = rootCaller.ReadResultFile(iCommandID, strUniqueID, &pvecResult);
    EXPECT_EQ(ERROR_FILESYSTEM_NO_SPACE, iRet);

    stub.set(&ErrorCode::GetSpecCommonID, StubFailed);
    stub.set(&CIPCFile::ReadResult, StubFailed);
    stub.set(&CRootCaller::RemoveFile, StubSuccess);
    iRet = rootCaller.ReadResultFile(iCommandID, strUniqueID, &pvecResult);
    EXPECT_EQ(MP_FAILED, iRet);

    stub.set(&ErrorCode::GetSpecCommonID, StubFailed);
    stub.set(&CIPCFile::ReadResult, StubSuccess);
    iRet = rootCaller.ReadResultFile(iCommandID, strUniqueID, &pvecResult);
    EXPECT_EQ(MP_SUCCESS, iRet);
}

/*
* 用例名称：删除文件
* 前置条件：无
* check点：1.文件是否存在 2.命令是否执行成功
*/
TEST_F(CRootCallerTest, RemoveFile) {
    stub.set(&CLogger::Log, StubCLoggerLog);
    CRootCaller rootCaller;
    mp_int32 iRet;
    mp_string fileName;

    stub.set(&CMpFile::FileExist, StubTrue);
    stub.set(&CRootCaller::Exec, StubFailed);
    iRet = rootCaller.RemoveFile(fileName);
    EXPECT_EQ(MP_FAILED, iRet);
}
