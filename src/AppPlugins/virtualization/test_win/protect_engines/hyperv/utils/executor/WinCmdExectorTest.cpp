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
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "stub.h"
#include "addr_pri.h"
#include <tchar.h>
#include <iostream>
#include <map>
#include <cerrno>
#include <memory>
#include <fstream>
#include "protect_engines\hyperv\resource_discovery\HyperVResourceAccess.h"
#include "protect_engines\hyperv\utils\executor\WinCmdExector.h"
#include "log/Log.h"
#include "common/uuid/Uuid.h"
#include "securec.h"
#include "common/Types.h"
#include "common/File.h"
#include "common/Constants.h"
#include <vector>
#include <winsock2.h>
#include <windows.h>
#include <WinBase.h>
#include "common/JsonHelper.h"
using namespace VirtPlugin;
using namespace HyperVPlugin;
using namespace HyperVPlugin::Utils;

namespace {
const std::string RESOURCE_ACCESS_SCRIPT_DIR = "C:/DataBackup/ProtectClient/Plugins/VirtualizationPlugin/bin";
const std::string RESOURCE_ACCESS_EXECUTOR = "Executor";
const DWORD TIMEOUT_DEFAULT = 300;
const std::string RESOURCE_TYPE = "HyperV";
const std::string CMD_TYPE_LIST_DISK = "ListDisk";
}  // namespace
class WinCmdExectorTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
};

void WinCmdExectorTest::SetUp()
{}

void WinCmdExectorTest::TearDown()
{}

void WinCmdExectorTest::SetUpTestCase()
{}

void WinCmdExectorTest::TearDownTestCase()
{}

ACCESS_PRIVATE_FUN(WinCmdExector, int(HANDLE &, HANDLE &), CreateProcPipe);
ACCESS_PRIVATE_FUN(WinCmdExector, int(const Param &, const std::string &, Json::Value &), GetScriptExecResult);
class WinCmdExectorStub {
public:
    int StubCreateProcPipeSuccess(HANDLE &hStdInputRd, HANDLE &hStdInputWr)
    {
        return MP_SUCCESS;
    }
    int StubCreateProcPipeFailed(HANDLE &hStdInputRd, HANDLE &hStdInputWr)
    {
        return MP_FAILED;
    }
    BOOL StubCreateProcessWFailed(LPCWSTR lpApplicationName, LPWSTR lpCommandLine,
        LPSECURITY_ATTRIBUTES lpProcessAttributes, LPSECURITY_ATTRIBUTES lpThreadAttributes, BOOL bInheritHandles,
        DWORD dwCreationFlags, LPVOID lpEnvironment, LPCWSTR lpCurrentDirectory, LPSTARTUPINFOW lpStartupInfo,
        LPPROCESS_INFORMATION lpProcessInformation)
    {
        return false;
    }
    BOOL StubCreateProcessWSuccess(LPCWSTR lpApplicationName, LPWSTR lpCommandLine,
        LPSECURITY_ATTRIBUTES lpProcessAttributes, LPSECURITY_ATTRIBUTES lpThreadAttributes, BOOL bInheritHandles,
        DWORD dwCreationFlags, LPVOID lpEnvironment, LPCWSTR lpCurrentDirectory, LPSTARTUPINFOW lpStartupInfo,
        LPPROCESS_INFORMATION lpProcessInformation)
    {
        return true;
    }
    BOOL StubWriteFileFailed(HANDLE hFile, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite,
        LPDWORD lpNumberOfBytesWritten, LPOVERLAPPED lpOverlapped)
    {
        return false;
    }
    BOOL StubWriteFileSuccess(HANDLE hFile, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite,
        LPDWORD lpNumberOfBytesWritten, LPOVERLAPPED lpOverlapped)
    {
        return true;
    }
    DWORD StubWaitForSingleObject(HANDLE hHandle, DWORD dwMilliseconds)
    {
        return WAIT_OBJECT_0;
    }
    BOOL StubGetExitCodeProcessSuccess(HANDLE hProcess, LPDWORD &lpExitCode)
    {
        return true;
    }
    int StubWriteFile2Success(std::string &strFilePath, std::vector<std::string> &vecInput)
    {
        return HyperVPlugin::SUCCESS;
    }
    bool StubFileExistSuccess(const std::string &pszFilePath)
    {
        return true;
    }
    int StubGetScriptExecResultSuccess(const Param &cmdParam, const std::string &uniqueId, Json::Value &result)
    {

        return HyperVPlugin::SUCCESS;
    }
};
/**
 * 测试用例：测试window命令执行
 * 前置条件：创建管道失败
 * Check点：测试window命令执行失败
 */
TEST_F(WinCmdExectorTest, ExecuteTestCreateProcPipeFailed)
{
    struct HyperVPlugin::Utils::Param cmdParam;
    cmdParam.timeout = TIMEOUT_DEFAULT;
    cmdParam.scriptDir = RESOURCE_ACCESS_SCRIPT_DIR;
    cmdParam.scriptName = RESOURCE_ACCESS_EXECUTOR;
    WinCmdExector::GetInstance().SetTargetHost("SCVMM");
    WinCmdExector::GetInstance().SetTargetType(CMD_RUNNING_TARGET::TYPE_SCVMM);
    Json::Value result;
    Stub stub;
    stub.set(get_private_fun::WinCmdExectorCreateProcPipe(), ADDR(WinCmdExectorStub, StubCreateProcPipeFailed));
    stub.set(ADDR(Module::CIPCFile, WriteFile), ADDR(WinCmdExectorStub, StubWriteFile2Success));
    stub.set(ADDR(Module::CFile, FileExist), ADDR(WinCmdExectorStub, StubFileExistSuccess));
    stub.set(get_private_fun::WinCmdExectorGetScriptExecResult(), ADDR(WinCmdExectorStub, StubGetScriptExecResultSuccess));
    int retRes = WinCmdExector::GetInstance().Execute(cmdParam, result);
    EXPECT_EQ(HyperVPlugin::FAILED, retRes);
}
/**
 * 测试用例：测试window命令执行
 * 前置条件：创建管道失败
 * Check点：测试window命令执行失败
 */
TEST_F(WinCmdExectorTest, ExecuteTestCreateProcessWFailed)
{
    struct HyperVPlugin::Utils::Param cmdParam;
    cmdParam.timeout = TIMEOUT_DEFAULT;
    cmdParam.scriptDir = RESOURCE_ACCESS_SCRIPT_DIR;
    cmdParam.scriptName = RESOURCE_ACCESS_EXECUTOR;
    WinCmdExector::GetInstance().SetTargetHost("SCVMM");
    WinCmdExector::GetInstance().SetTargetType(CMD_RUNNING_TARGET::TYPE_SCVMM);
    Json::Value result;
    Stub stub;
    stub.set(get_private_fun::WinCmdExectorCreateProcPipe(), ADDR(WinCmdExectorStub, StubCreateProcPipeSuccess));
    stub.set(ADDR(Module::CIPCFile, WriteFile), ADDR(WinCmdExectorStub, StubWriteFile2Success));
    stub.set(ADDR(Module::CFile, FileExist), ADDR(WinCmdExectorStub, StubFileExistSuccess));
    stub.set(CreateProcessW, ADDR(WinCmdExectorStub, StubCreateProcessWFailed));
    stub.set(get_private_fun::WinCmdExectorGetScriptExecResult(), ADDR(WinCmdExectorStub, StubGetScriptExecResultSuccess));
    int retRes = WinCmdExector::GetInstance().Execute(cmdParam, result);
    EXPECT_EQ(HyperVPlugin::FAILED, retRes);
}
/**
 * 测试用例：测试window命令执行
 * 前置条件：写文件失败
 * Check点：测试window命令执行失败
 */
TEST_F(WinCmdExectorTest, ExecuteTestWriteFileFailed)
{
    struct HyperVPlugin::Utils::Param cmdParam;
    cmdParam.timeout = TIMEOUT_DEFAULT;
    cmdParam.scriptDir = RESOURCE_ACCESS_SCRIPT_DIR;
    cmdParam.scriptName = RESOURCE_ACCESS_EXECUTOR;
    WinCmdExector::GetInstance().SetTargetHost("SCVMM");
    WinCmdExector::GetInstance().SetTargetType(CMD_RUNNING_TARGET::TYPE_SCVMM);
    Json::Value result;
    Stub stub;
    stub.set(get_private_fun::WinCmdExectorCreateProcPipe(), ADDR(WinCmdExectorStub, StubCreateProcPipeSuccess));
    stub.set(ADDR(Module::CIPCFile, WriteFile), ADDR(WinCmdExectorStub, StubWriteFile2Success));
    stub.set(ADDR(Module::CFile, FileExist), ADDR(WinCmdExectorStub, StubFileExistSuccess));
    stub.set(CreateProcessW, ADDR(WinCmdExectorStub, StubCreateProcessWSuccess));
    stub.set(WriteFile, ADDR(WinCmdExectorStub, StubWriteFileFailed));
    stub.set(get_private_fun::WinCmdExectorGetScriptExecResult(), ADDR(WinCmdExectorStub, StubGetScriptExecResultSuccess));
    int retRes = WinCmdExector::GetInstance().Execute(cmdParam, result);
    EXPECT_EQ(HyperVPlugin::FAILED, retRes);
}
/**
 * 测试用例：测试window命令执行
 * 前置条件：执行命令成功
 * Check点：测试window命令执行成功
 */
TEST_F(WinCmdExectorTest, ExecuteTestWaitForSingleObjectSuccess)
{
    struct HyperVPlugin::Utils::Param cmdParam;
    cmdParam.timeout = TIMEOUT_DEFAULT;
    cmdParam.scriptDir = RESOURCE_ACCESS_SCRIPT_DIR;
    cmdParam.scriptName = RESOURCE_ACCESS_EXECUTOR;
    WinCmdExector::GetInstance().SetTargetHost("SCVMM");
    WinCmdExector::GetInstance().SetTargetType(CMD_RUNNING_TARGET::TYPE_SCVMM);
    Json::Value result;
    Stub stub;
    stub.set(get_private_fun::WinCmdExectorCreateProcPipe(), ADDR(WinCmdExectorStub, StubCreateProcPipeSuccess));
    stub.set(ADDR(Module::CIPCFile, WriteFile), ADDR(WinCmdExectorStub, StubWriteFile2Success));
    stub.set(ADDR(Module::CFile, FileExist), ADDR(WinCmdExectorStub, StubFileExistSuccess));
    stub.set(CreateProcessW, ADDR(WinCmdExectorStub, StubCreateProcessWSuccess));
    stub.set(WriteFile, ADDR(WinCmdExectorStub, StubWriteFileSuccess));
    stub.set(WaitForSingleObject, ADDR(WinCmdExectorStub, StubWaitForSingleObject));
    stub.set(GetExitCodeProcess, ADDR(WinCmdExectorStub, StubGetExitCodeProcessSuccess));
    stub.set(get_private_fun::WinCmdExectorGetScriptExecResult(), ADDR(WinCmdExectorStub, StubGetScriptExecResultSuccess));
    int retRes = WinCmdExector::GetInstance().Execute(cmdParam, result);
    EXPECT_EQ(HyperVPlugin::SUCCESS, retRes);
}
