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
#include "rootexec/SystemCallTest.h"
#include <sys/ioctl.h>
#include "array/array.h"
#include "array/disk.h"
#include "securecom/RootCaller.h"
#include "securecom/SecureUtils.h"
#include "common/Utils.h"

#include <map>
using namespace std;

namespace {

static mp_void StubCLoggerLog(mp_void){
    return;
}

mp_int32 StubFailedExecSystemWithEcho(const mp_string& strCommand, vector<mp_string>& strEcho)
{
    return MP_FAILED;
}

mp_int32 StubSuccessExecSystemWithEcho(const mp_string& strCommand, vector<mp_string>& strEcho)
{
    strEcho.push_back("cgroup on /sys/fs/cgroup/cpu,cpuacct type cgroup (rw,nosuid,nodev,noexec,relatime,cpu,cpuacct)");
    return MP_SUCCESS;
}

mp_int32 StubSuccess(mp_void* pthis)
{
    return MP_SUCCESS;
}

mp_int32 StubGetThirdPartyScriptSuccess(
    const mp_string& strUniqueID, mp_string& strScriptName, mp_string& isUserDefined, mp_string& paramValues)
{
    paramValues = "test";
    return MP_SUCCESS;
}

mp_void StubVoid(void *pobj)
{
    return;
}

mp_void StubGetOSTypeRedhat(mp_int32 &iOSType)
{
    iOSType = HOST_OS_REDHAT;
}

mp_void StubGetOSTypeSuse(mp_int32 &iOSType)
{
    iOSType = HOST_OS_SUSE;
}

mp_void StubGetOSTypeOther(mp_int32 &iOSType)
{
    iOSType = HOST_OS_OTHER_LINUX;
}

mp_int32 StubGetOSVersionFive(mp_int32 iOSType, mp_string &strOSVersion)
{
    strOSVersion = "5";
    return MP_SUCCESS;
}

mp_int32 StubGetOSVersionTen(mp_int32 iOSType, mp_string &strOSVersion)
{
    strOSVersion = "10";
    return MP_SUCCESS;
}

FILE* StubPopenNull(mp_void* pthis)
{
    FILE* pStream = nullptr;
    return pStream;
}

FILE* StubPopen(mp_void* pthis)
{
    FILE* pStream = new FILE;
    return pStream;
}

char *Stubfgets(char *s, int size, FILE *stream)
{
    s = "test";
    return s;
}

vector<mp_string> StubAwk(const vector<mp_string>& vecInput, int nPrint, char cSep)
{
    vector<mp_string> outVec;
    outVec.push_back("t");
    return outVec;
}

mp_bool StubFormattingPathTrue(mp_string& strPath)
{
    strPath = "/opt/DataBackup/ProtectClient/ProtectClient-E/stmp";
    return MP_TRUE;
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

mp_bool StubCgroupDirExistTrue(const mp_char* pszDirPath){
    return MP_TRUE;
}

mp_bool StubCgroupDirExistFalse(const mp_char* pszDirPath){
    return MP_FALSE;
}

mp_int32 StubCgroupDirCreateSuccess(const mp_char* pszDirPath){
    return MP_SUCCESS;
}

mp_int32 StubCgroupDirCreateFailed(const mp_char* pszDirPath){
    return MP_FAILED;
}

mp_int32 StubGetValueString(mp_void* pthis, const mp_string& strSection, const mp_string& strKey, mp_string& strValue)
{
    if (strSection == CFG_SYSTEM_SECTION) {
        if (strKey == CFG_SIGN_IGNORE) {
            strValue = CFG_SIGN_IGNORE;
        }
    }
    return MP_SUCCESS;
}

mp_int32 StubReadInput(mp_void* pthis, const mp_string& strUniqueID, mp_string& strInput)
{
    strInput = "test.sh:1:param";
    return MP_SUCCESS;
}

mp_int32 StubReadInputErr(mp_void* pthis, const mp_string& strUniqueID, mp_string& strInput)
{
    strInput = "test.sh:param";
    return MP_SUCCESS;
}

mp_int32 StubGetDiskArrayInfo(mp_void* pthis, const mp_string& strDevice, mp_string& strVendor, mp_string& strProduct)
{
    strVendor = "strVendor";
    strProduct = "strProduct";
    return MP_SUCCESS;
}

mp_int32 GetParamFromTmpFileFailed(mp_void* pThis, const mp_string& strUniqueID, mp_string& strParam)
{
    return MP_FAILED;
}

mp_int32 GetParamFromTmpFileSucc(mp_void* pThis, const mp_string& strUniqueID, mp_string& strParam)
{
    return MP_SUCCESS;
}

mp_int32 GetParamFromTmpFileForCPULimitSucc(mp_void* pThis, const mp_string& strUniqueID, mp_string& strParam)
{
    strParam = "HadoopPlugin;998;2000";
    return MP_SUCCESS;
}

mp_int32 GetHostLunIDListFailed(const mp_string& strDevice, vector<mp_int32>& vecHostLunID)
{
    return MP_FAILED;
}

mp_int32 GetHostLunIDListSucc(const mp_string& strDevice, vector<mp_int32>& vecHostLunID)
{
    vecHostLunID.push_back(0);
    return MP_SUCCESS;
}

mp_int32 WriteResultFailed(const mp_string& strUniqueID, vector<mp_string>& vecRlt)
{
    return MP_FAILED;
}

mp_int32 WriteResultSucc(const mp_string& strUniqueID, vector<mp_string>& vecRlt)
{
    return MP_SUCCESS;
}

mp_int32 GetAPPScriptsFailed
    (mp_int32 iCommandID, const mp_string& appScriptFolder, std::vector<mp_string>& vecScripts)
{
    return MP_FAILED;
}

mp_int32 GetAPPScriptsSuccSize0
    (mp_int32 iCommandID, const mp_string& appScriptFolder, std::vector<mp_string>& vecScripts)
{
    vecScripts.clear();
    return MP_SUCCESS;
}

mp_int32 GetAPPScriptsSuccSizeNot0
    (mp_int32 iCommandID, const mp_string& appScriptFolder, std::vector<mp_string>& vecScripts)
{
    vecScripts.push_back("123456");
    return MP_SUCCESS;
}

mp_int32 ExecEbkUserScriptFailed(mp_string& strScriptFileName, const mp_string& strUniqueID)
{
    return MP_FAILED;
}

mp_int32 ExecEbkUserScriptSucc(mp_string& strScriptFileName, const mp_string& strUniqueID)
{
    return MP_SUCCESS;
}

mp_int32 GetFolderFileFailed(mp_string& strFolder, vector<mp_string>& vecFileList)
{
    return MP_FAILED;
}

mp_int32 GetFolderFileSucc(mp_string& strFolder, vector<mp_string>& vecFileList)
{
    vecFileList.push_back("_freeze.sh");
    vecFileList.push_back("_unfreeze.sh");
    return MP_SUCCESS;
}

mp_int32 StubGetDisk00Page(mp_void* pthis, const mp_string& strDevice, vector<mp_string>& vecResult)
{
    vecResult.push_back("c8");
    return MP_SUCCESS;
}

mp_int32 GetDiskC8PageFailed(const mp_string& strDevice, mp_string& strLunID)
{
    return MP_FAILED;
}

mp_int32 GetDiskC8PageSucc(const mp_string& strDevice, mp_string& strLunID)
{
    return MP_SUCCESS;
}

mp_int32 GetDiskC8PageSuccess(const mp_string& strDevice, mp_string& strLunID)
{
    strLunID = "test";
    return MP_SUCCESS;
}

char* StubRealpathNull(mp_void *pobj)
{
    char* pStrParam = nullptr;
    return pStrParam;
}

char* StubRealpath(mp_void *pobj)
{
    char* pStrParam = "/test";
    return pStrParam;
}

mp_bool StubFormattingPathSucc(mp_string& strPath)
{
    if (mp_string::npos != strPath.find("stop.sh")) {
        strPath = "/opt/DataBackup/ProtectClient/Plugins/GeneralDBPlugin/stop.sh";
    } else if(strPath == "/opt/DataBackup/ProtectClient/ProtectClient-E/../../..") {
        strPath = "/opt";
    } else if (strPath == "/opt/DataBackup/ProtectClient/ProtectClient-E/..") {
        strPath = "/opt/DataBackup/ProtectClient/Plugins";
    }

    return MP_TRUE;
}

mp_bool StubFormattingPathFail(mp_string& strPath)
{
    strPath = "touch /tmp/test";
    return MP_TRUE;
}

mp_int32 StubExecSystemWithoutEchoInvoid()
{
    return ERROR_COMMON_INVALID_PARAM;
}

mp_int32 StubExecSystemWithoutEchoFailed()
{
    return ERROR_COMMON_SYSTEM_CALL_FAILED;
}

mp_int32 g_StubFalseOnZero_flag = 0;
mp_bool StubFalseOnZero(mp_void* pthis)
{
    if (g_StubFalseOnZero_flag-- > 0) {
        return MP_TRUE;
    }  else {
        return MP_FALSE;
    }
}

mp_int32 GetFolderDirSuc(mp_void* pthis, const mp_string& strFolder, std::vector<mp_string>& vecNameList)
{
    vecNameList.push_back("HadoopPlugin");
    return MP_SUCCESS;
}

mp_bool StubGetParentPid_fail(mp_string& ppid)
{
    return MP_FALSE;
}

mp_bool StubGetParentPid_succ(mp_string& ppid)
{
    return MP_TRUE;
}

mp_bool StubGetProcessInfo_fail(const mp_string& pid, mp_string& result)
{
    return MP_FALSE;
}

mp_bool StubGetProcessInfo_succ(mp_void* pthis, const mp_string& pid, mp_string& result)
{
    result = "/opt/DataBackup/ProtectClient/ProtectClient-E//bin/rdagent";
    return MP_TRUE;
}

}

TEST_F(CSystemCallTest, GetHostLunIDTest)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    mp_string strUniqueID = "0x123234";
    std::vector<mp_string> vecParam = { "device" };
    CSystemCall caller;

    stub.set(ADDR(Array, GetHostLunIDList), StubFailed);
    EXPECT_EQ(MP_FAILED, caller.GetHostLunID(strUniqueID, vecParam));

    stub.set(ADDR(Array, GetHostLunIDList), StubSuccess);
    stub.set(ADDR(CIPCFile, WriteResult), StubSuccess);
    EXPECT_EQ(MP_SUCCESS, caller.GetHostLunID(strUniqueID, vecParam));
}

TEST_F(CCommandMapTest, xFunc)
{
    CCommandMap commonMap;

    commonMap.InitDB2ScriptMap();
    commonMap.InitOracleScriptMap();
    commonMap.InitSysCmdMap1();
    commonMap.InitSysCmdMap2();
    commonMap.InitSysCmdMap4();
    commonMap.InitSysCmdMap5();
    commonMap.InitNeedEchoCmdMap1();
    commonMap.InitNeedEchoCmdMap2();
}


TEST_F(CCommandMapTest, GetCommandString)
{
    mp_int32 iCommandID = 0;
    CCommandMap commonMap;
    
    commonMap.GetCommandString(iCommandID);
    
    commonMap.m_mapCommand.insert(map<mp_int32, mp_string>::value_type(0, "hagrp"));
    commonMap.GetCommandString(iCommandID);
}


TEST_F(CCommandMapTest, NeedEcho)
{
    mp_int32 iCommandID = 0;
    CCommandMap commonMap;
    
    commonMap.NeedEcho(iCommandID);
    
    commonMap.m_mapNeedEcho.insert(map<mp_int32, mp_bool>::value_type(0, MP_TRUE));
    commonMap.NeedEcho(iCommandID);
}


/*
 * 用例名称：ִ执行系统命令，从加密的输入临时文件中读出并进行解密，执行，将结果写入结果临时文件
 * 前置条件：无
 * check点：检查返回值
 */
TEST_F(CSystemCallTest, ExecSysCmd)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    mp_string strUniqueID;
    mp_int32 iCommandID;
    std::vector<mp_string> vecParam = { "a=1" };
    CSystemCall caller;

    iCommandID = ROOT_COMMAND_IOSCANFNC;
    EXPECT_EQ(MP_FAILED, caller.ExecSysCmd(strUniqueID, iCommandID, vecParam));

    iCommandID = ROOT_COMMAND_LS;
    stub.set(CheckCmdDelimiter, StubFailed);
    EXPECT_EQ(MP_FAILED, caller.ExecSysCmd(strUniqueID, iCommandID, vecParam));

    stub.set(CheckCmdDelimiter, StubSuccess);
    stub.set(&CSystemCall::WhiteListVerify, StubFalse);
    EXPECT_EQ(MP_FAILED, caller.ExecSysCmd(strUniqueID, iCommandID, vecParam));

    stub.set(&CSystemCall::WhiteListVerify, StubTrue);
    stub.set(&CCommandMap::NeedEcho, StubTrue);
    stub.set(&CSystemExec::ExecSystemWithEcho, StubFailed);
    EXPECT_EQ(MP_FAILED, caller.ExecSysCmd(strUniqueID, iCommandID, vecParam));
    stub.set(&CSystemExec::ExecSystemWithEcho, StubSuccess);
    stub.set(&CIPCFile::WriteResult, StubSuccess);
    EXPECT_EQ(MP_SUCCESS, caller.ExecSysCmd(strUniqueID, iCommandID, vecParam));

    stub.set(&CCommandMap::NeedEcho, StubFailed);
    stub.set(ADDR(CSystemExec, ExecSystemWithoutEchoNoWin), StubSuccess);
    EXPECT_EQ(MP_SUCCESS, caller.ExecSysCmd(strUniqueID, iCommandID, vecParam));
}


TEST_F(CSystemCallTest, ExecScript)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    mp_string strUniqueID;
    mp_int32 iCommandID;
    std::vector<mp_string> vecParam = { "a=1" };
    CSystemCall caller;
    CPath::GetInstance().SetRootPath("/opt/DataBackup/ProtectClient/ProtectClient-E");

    stub.set(&CMpFile::FileExist, StubFalse);
    EXPECT_EQ(INTER_ERROR_SRCIPT_FILE_NOT_EXIST, caller.ExecScript(strUniqueID, iCommandID, vecParam));

    stub.set(&CMpFile::FileExist, StubTrue);
    stub.set(&CSystemExec::ExecSystemWithSecurityParam, StubFailed);
    EXPECT_EQ(MP_FAILED, caller.ExecScript(strUniqueID, iCommandID, vecParam));

    stub.set(&CSystemExec::ExecSystemWithSecurityParam, StubSuccess);
    stub.set(ADDR(CIPCFile, ChownResult), StubSuccess);
    EXPECT_EQ(MP_SUCCESS, caller.ExecScript(strUniqueID, iCommandID, vecParam));

    stub.set(ADDR(CIPCFile, ChownResult), StubFailed);
    EXPECT_EQ(MP_SUCCESS, caller.ExecScript(strUniqueID, iCommandID, vecParam));
}

TEST_F(CSystemCallTest, ExecThirdPartyScript)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    CSystemCall caller;
    mp_string strUniqueID;
    CPath::GetInstance().SetRootPath("/opt/DataBackup/ProtectClient/ProtectClient-E");

    EXPECT_EQ(MP_FAILED, caller.ExecThirdPartyScript(strUniqueID, std::vector<mp_string>()));

    std::vector<mp_string> vecParam = { "../../test.sh", "1" };
    stub.set(&CMpFile::FileExist, StubFalse);
    EXPECT_EQ(INTER_ERROR_SRCIPT_FILE_NOT_EXIST, caller.ExecThirdPartyScript(strUniqueID, vecParam));
//    stub.set(&CMpFile::FileExist, StubTrue);
//    EXPECT_EQ(MP_FAILED, caller.ExecThirdPartyScript(strUniqueID, vecParam));

    std::vector<mp_string> vecParam2 = { "test.sh", "1" };
    stub.set(&CMpFile::FileExist, StubTrue);
    stub.set(&CSystemExec::ExecSystemWithSecurityParam, StubFailed);
    EXPECT_EQ(MP_FAILED, caller.ExecThirdPartyScript(strUniqueID, vecParam2));
    stub.set(&CSystemExec::ExecSystemWithSecurityParam, StubSuccess);
    EXPECT_EQ(MP_SUCCESS, caller.ExecThirdPartyScript(strUniqueID, vecParam2));
}

TEST_F(CSystemCallTest, ExecScriptByScriptUser)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    CSystemCall caller;
    mp_string strUniqueID;
    CPath::GetInstance().SetRootPath("/opt/DataBackup/ProtectClient/ProtectClient-E");
    EXPECT_EQ(MP_FAILED, caller.ExecScriptByScriptUser(strUniqueID, std::vector<mp_string>()));
    std::vector<mp_string> vecParam = { "../../test.sh", "1" };
    stub.set(&CMpFile::FileExist, StubFalse);
    EXPECT_EQ(INTER_ERROR_SRCIPT_FILE_NOT_EXIST, caller.ExecScriptByScriptUser(strUniqueID, vecParam));
    stub.set(&CMpFile::FileExist, StubTrue);

    int sStatus = system("touch ./testUserScript.sh");
    if (sStatus != 0) {
        printf("touch ./testUserScript.sh fail.");
    }
    sStatus = system("ls -l ./testUserScript.sh");
    if (sStatus != 0) {
        printf("ls -l fail.");
    }

    std::vector<mp_string> vecParam2 = { "./testUserScript.sh", "1" };
    stub.set(&CSystemExec::ExecSystemWithSecurityParam, StubFailed);
    stub.set(GetFileOwnerName, StubFailed);
    EXPECT_EQ(MP_FAILED, caller.ExecScriptByScriptUser(strUniqueID, vecParam2));

    stub.reset(GetFileOwnerName);
    EXPECT_EQ(MP_FAILED, caller.ExecScriptByScriptUser(strUniqueID, vecParam2));

    stub.set(&CSystemExec::ExecSystemWithSecurityParam, StubSuccess);
    EXPECT_EQ(MP_SUCCESS, caller.ExecScriptByScriptUser(strUniqueID, vecParam2));

    sStatus = system("rm ./testUserScript.sh");
    if (sStatus != 0) {
        printf("rm fail.");
    }
}

TEST_F(CSystemCallTest, ExecUserDefineScript)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    CSystemCall caller;
    mp_string strUniqueID;

    stub.set(&CSystemExec::ExecSystemWithoutEchoNoWin, StubFailed);
    stub.set(&CSystemCall::CheckUserDefineScriptLegality, StubSuccess);
    EXPECT_EQ(MP_FAILED, caller.ExecUserDefineScript(strUniqueID));
}

TEST_F(CSystemCallTest, GetDisk80Page)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    mp_string strUniqueID = "0x123234";
    std::vector<mp_string> vecParam = { "device" };
    CSystemCall caller;

    stub.set(ADDR(Array, GetDisk80Page), StubFailed);
    EXPECT_EQ(MP_FAILED, caller.GetDisk80Page(strUniqueID, vecParam));

    stub.set(ADDR(Array, GetDisk80Page), StubSuccess);
    stub.set(ADDR(CIPCFile, WriteResult), StubSuccess);
    EXPECT_EQ(MP_SUCCESS, caller.GetDisk80Page(strUniqueID, vecParam));
}


TEST_F(CSystemCallTest, GetDisk83Page)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    mp_string strUniqueID = "0x123234";
    std::vector<mp_string> vecParam = { "device" };
    CSystemCall caller;

    stub.set(ADDR(Array, GetDisk83Page), StubFailed);
    EXPECT_EQ(MP_FAILED, caller.GetDisk83Page(strUniqueID, vecParam));

    stub.set(ADDR(Array, GetDisk83Page), StubSuccess);
    stub.set(ADDR(CIPCFile, WriteResult), StubSuccess);
    EXPECT_EQ(MP_SUCCESS, caller.GetDisk83Page(strUniqueID, vecParam));
}

TEST_F(CSystemCallTest, GetDiskCapacity)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    mp_string strUniqueID = "0x123234";
    std::vector<mp_string> vecParam = { "device" };
    CSystemCall caller;

    stub.set(ADDR(Disk, GetDiskCapacity), StubFailed);
    EXPECT_EQ(MP_FAILED, caller.GetDiskCapacity(strUniqueID, vecParam));

    stub.set(ADDR(Disk, GetDiskCapacity), StubSuccess);
    stub.set(ADDR(CIPCFile, WriteResult), StubSuccess);
    EXPECT_EQ(MP_SUCCESS, caller.GetDiskCapacity(strUniqueID, vecParam));
}

TEST_F(CSystemCallTest, GetVendorAndProduct)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    mp_string strUniqueID = "0x123234";
    std::vector<mp_string> vecParam = { "device" };
    CSystemCall caller;

    stub.set(ADDR(Array, GetDiskArrayInfo), StubFailed);
    EXPECT_EQ(MP_FAILED, caller.GetVendorAndProduct(strUniqueID, vecParam));

    stub.set(ADDR(Array, GetDiskArrayInfo), StubSuccess);
    stub.set(ADDR(CIPCFile, WriteResult), StubSuccess);
    EXPECT_EQ(MP_SUCCESS, caller.GetVendorAndProduct(strUniqueID, vecParam));
}

static mp_void StubStrSplit(vector<mp_string>& vecTokens, const mp_string& strText, mp_char cSep)
{
    static int i = 0;
    if (i++ >= 1)
    {
         vecTokens.push_back("6688");
    }
}

static mp_void StubStrSplitTwo(vector<mp_string>& vecTokens, const mp_string& strText, mp_char cSep)
{
    vecTokens.push_back("");
    vecTokens.push_back("6688");
}

TEST_F(CCommandMapTest, BatchGetLUNInfo)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    mp_string strUniqueID = "0x123234";
    CSystemCall caller;

    EXPECT_EQ(MP_FAILED, caller.BatchGetLUNInfo(strUniqueID, std::vector<mp_string>()));

    std::vector<mp_string> vecParam = { ";device" };
    stub.set(&CSystemCall::GetLUNInfo, StubFailed);
    EXPECT_EQ(MP_FAILED, caller.BatchGetLUNInfo(strUniqueID, vecParam));

    stub.set(&CSystemCall::GetLUNInfo, StubSuccess);
    stub.set(ADDR(CIPCFile, WriteResult), StubSuccess);
    EXPECT_EQ(MP_SUCCESS, caller.BatchGetLUNInfo(strUniqueID, vecParam));
}

TEST_F(CSystemCallTest, GetLUNInfo)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    CSystemCall caller;
    mp_string strDevice;
    mp_string strLUNInfo;

    stub.set(ADDR(Array, GetDiskArrayInfo), StubFailed);
    EXPECT_EQ(MP_FAILED, caller.GetLUNInfo(strDevice, strLUNInfo));

    stub.set(ADDR(Array, GetDiskArrayInfo), StubSuccess);
    EXPECT_EQ(MP_SUCCESS, caller.GetLUNInfo(strDevice, strLUNInfo));

    stub.set(ADDR(Array, GetDiskArrayInfo), StubGetDiskArrayInfo);
    stub.set(ADDR(Array, CheckHuaweiLUN), StubFalse);
    EXPECT_EQ(MP_SUCCESS, caller.GetLUNInfo(strDevice, strLUNInfo));

    stub.set(ADDR(Array, GetDiskArrayInfo), StubGetDiskArrayInfo);
    stub.set(ADDR(Array, CheckHuaweiLUN), StubTrue);
    stub.set(ADDR(Array, GetDisk80Page), StubFailed);
    EXPECT_EQ(MP_SUCCESS, caller.GetLUNInfo(strDevice, strLUNInfo));

    stub.set(ADDR(Array, GetDiskArrayInfo), StubGetDiskArrayInfo);
    stub.set(ADDR(Array, CheckHuaweiLUN), StubTrue);
    stub.set(ADDR(Array, GetDisk80Page), StubSuccess);
    stub.set(ADDR(Array, GetDisk83Page), StubFailed);
    EXPECT_EQ(MP_FAILED, caller.GetLUNInfo(strDevice, strLUNInfo));

    stub.set(ADDR(Array, GetDiskArrayInfo), StubGetDiskArrayInfo);
    stub.set(ADDR(Array, CheckHuaweiLUN), StubTrue);
    stub.set(ADDR(Array, GetDisk80Page), StubSuccess);
    stub.set(ADDR(Array, GetDisk83Page), StubSuccess);
    stub.set(ADDR(CSystemCall, GetLUNIdWhenC8), StubFailed);
    EXPECT_EQ(MP_FAILED, caller.GetLUNInfo(strDevice, strLUNInfo));

    stub.set(ADDR(Array, GetDiskArrayInfo), StubGetDiskArrayInfo);
    stub.set(ADDR(Array, CheckHuaweiLUN), StubTrue);
    stub.set(ADDR(Array, GetDisk80Page), StubSuccess);
    stub.set(ADDR(Array, GetDisk83Page), StubSuccess);
    stub.set(ADDR(CSystemCall, GetLUNIdWhenC8), StubSuccess);
    EXPECT_EQ(MP_SUCCESS, caller.GetLUNInfo(strDevice, strLUNInfo));
}

TEST_F(CSystemCallTest, GetLUNIdWhenC8)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    CSystemCall caller;
    mp_string strDevice;
    mp_string strLUNInfo;

    stub.set(ADDR(Array, GetDisk00Page), StubFailed);
    EXPECT_EQ(MP_FAILED, caller.GetLUNIdWhenC8(strDevice, strLUNInfo));

    stub.set(ADDR(Array, GetDisk00Page), StubGetDisk00Page);
    stub.set(ADDR(Array, GetDiskC8Page), StubFailed);
    EXPECT_EQ(MP_FAILED, caller.GetLUNIdWhenC8(strDevice, strLUNInfo));

    stub.set(ADDR(Array, GetDisk00Page), StubGetDisk00Page);
    stub.set(ADDR(Array, GetDiskC8Page), StubSuccess);
    EXPECT_EQ(MP_SUCCESS, caller.GetLUNIdWhenC8(strDevice, strLUNInfo));
}

TEST_F(CSystemCallTest, CheckParentPathTest)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    mp_string path1 = "/opt/DataBackup/ProtectClient/";
    CSystemCall caller;
    EXPECT_EQ(MP_FAILED, caller.CheckParentPath(path1));
}


/*
 * 用例名称：执行APP脚本命令
 * 前置条件：无
 * check点：检查返回值
 */
TEST_F(CCommandMapTest, ExecEbkUserScript)
{
    mp_string strUniqueID = "0x123234";
    mp_string fileName;
    mp_int32 iRet;
    CSystemCall caller;

    stub.set(&CMpFile::CheckFileAccess, stub_return_success);
    iRet = caller.ExecEbkUserScript(fileName, strUniqueID);
    EXPECT_EQ(iRet, 5);

    stub.set(&CMpFile::CheckFileAccess, StubFailed);
    iRet = caller.ExecEbkUserScript(fileName, strUniqueID);
    EXPECT_EQ(iRet, ERROR_SCRIPT_APP_FAILED);

    stub.set(&CMpFile::CheckFileAccess, StubSuccess);
    stub.set(CheckCmdDelimiter, StubSuccess);
    stub.set(ADDR(CSystemExec, ExecSystemWithoutEchoNoWin), StubExecSystemWithoutEchoInvoid);
    iRet = caller.ExecEbkUserScript(fileName, strUniqueID);
    EXPECT_EQ(iRet, ERROR_SCRIPT_APP_FAILED);

    stub.set(ADDR(CSystemExec, ExecSystemWithoutEchoNoWin), StubExecSystemWithoutEchoFailed);
    iRet = caller.ExecEbkUserScript(fileName, strUniqueID);
    EXPECT_EQ(iRet, ERROR_SCRIPT_APP_FAILED);

    stub.set(ADDR(CSystemExec, ExecSystemWithoutEchoNoWin), StubSuccess);
    stub.set(ADDR(CIPCFile, ChownResult), StubSuccess);
    iRet = caller.ExecEbkUserScript(fileName, strUniqueID);
    EXPECT_EQ(iRet, MP_SUCCESS);

    stub.set(ADDR(CIPCFile, ChownResult), StubFailed);
    iRet = caller.ExecEbkUserScript(fileName, strUniqueID);
    EXPECT_EQ(iRet, ERROR_SCRIPT_APP_FAILED);
}

TEST_F(CCommandMapTest, ExecAppScriptTest)
{
    mp_string strUniqueID;
    mp_int32 iCommandID;
    CSystemCall caller;
    mp_int32 iRet;

    stub.set(ADDR(CSystemCall, GetAPPScripts), GetAPPScriptsFailed);
    iRet = caller.ExecAppScript(strUniqueID, iCommandID);
    EXPECT_EQ(iRet, MP_FAILED);

    stub.set(ADDR(CSystemCall, GetAPPScripts), GetAPPScriptsSuccSize0);
    iRet = caller.ExecAppScript(strUniqueID, iCommandID);
    EXPECT_EQ(iRet, INTER_ERROR_SRCIPT_FILE_NOT_EXIST);

    stub.set(ADDR(CSystemCall, GetAPPScripts), GetAPPScriptsSuccSizeNot0);
    stub.set(ADDR(CSystemCall, ExecEbkUserScript), ExecEbkUserScriptFailed);
    iRet = caller.ExecAppScript(strUniqueID, iCommandID);
    EXPECT_EQ(iRet, MP_FAILED);

    stub.set(ADDR(CSystemCall, ExecEbkUserScript), ExecEbkUserScriptSucc);
    iRet = caller.ExecAppScript(strUniqueID, iCommandID);
    EXPECT_EQ(iRet, MP_SUCCESS);
}

TEST_F(CCommandMapTest, ExecAddFirewall)
{
    mp_int32 iRet;
    CSystemCall caller;

    iRet = caller.ExecAddFirewall();
    EXPECT_EQ(iRet, ERROR_SCRIPT_COMMON_EXEC_FAILED);

    stub.set(ADDR(CSystemExec, ExecSystemWithoutEchoNoWin), StubSuccess);
    iRet = caller.ExecAddFirewall();
    EXPECT_EQ(iRet, MP_SUCCESS);
}

TEST_F(CCommandMapTest, GetLUNIdWhenC8)
{
    mp_int32 iRet;
    mp_string dev;
    mp_string lunId;
    CSystemCall caller;

    iRet = caller.GetLUNIdWhenC8(dev, lunId);
    EXPECT_EQ(iRet, MP_FAILED);
}

TEST_F(CCommandMapTest, GetDisk00PageTest)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    mp_string strUniqueID = "0x123234";
    std::vector<mp_string> vecParam = { "device" };
    CSystemCall caller;

    stub.set(ADDR(Array, GetDisk00Page), StubFailed);
    EXPECT_EQ(MP_FAILED, caller.GetDisk00Page(strUniqueID, vecParam));

    stub.set(ADDR(Array, GetDisk00Page), StubSuccess);
    stub.set(ADDR(CIPCFile, WriteResult), StubFailed);
    EXPECT_EQ(MP_FAILED, caller.GetDisk00Page(strUniqueID, vecParam));

    stub.set(ADDR(Array, GetDisk00Page), StubSuccess);
    stub.set(ADDR(CIPCFile, WriteResult), StubSuccess);
    EXPECT_EQ(MP_SUCCESS, caller.GetDisk00Page(strUniqueID, vecParam));
}

/*
* Description  : 查询disk 83页
* 前置条件：存在disk
* check点：1、检查查询disk 83页是否成功
*/
TEST_F(CCommandMapTest, GetDiskC8PageTest)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    mp_string strUniqueID = "0x123234";
    std::vector<mp_string> vecParam = { "device" };
    CSystemCall caller;

    stub.set(ADDR(Array, GetDiskC8Page), StubFailed);
    EXPECT_EQ(MP_FAILED, caller.GetDiskC8Page(strUniqueID, vecParam));

    stub.set(ADDR(Array, GetDiskC8Page), StubSuccess);
    stub.set(ADDR(CIPCFile, WriteResult), StubFailed);
    EXPECT_EQ(MP_FAILED, caller.GetDiskC8Page(strUniqueID, vecParam));

    stub.set(ADDR(Array, GetDiskC8Page), StubSuccess);
    stub.set(ADDR(CIPCFile, WriteResult), StubSuccess);
    EXPECT_EQ(MP_SUCCESS, caller.GetDiskC8Page(strUniqueID, vecParam));
}

/*
* Description  : 获取raw设备的主分区和此分区信息
* 前置条件：存在raw设备
* check点：1、获取raw设备的主分区和此分区信息是否成功，2、将结果写入结果文件是否成功
*/
TEST_F(CSystemCallTest, GetRawMajorMinorTest)
{
    mp_string strUniqueID = "0x123234";
    CSystemCall caller;
    mp_int32 iRet;

    stub.set(ADDR(CSystemCall, GetParamFromTmpFile), GetParamFromTmpFileFailed);
    iRet = caller.GetRawMajorMinor(strUniqueID);
    EXPECT_EQ(iRet, MP_FAILED);

    stub.set(ADDR(CSystemCall, GetParamFromTmpFile), GetParamFromTmpFileSucc);
    iRet = caller.GetRawMajorMinor(strUniqueID);
    EXPECT_EQ(iRet, MP_FAILED);

    stub.set(open, StubSuccess);
    iRet = caller.GetRawMajorMinor(strUniqueID);
    EXPECT_EQ(iRet, MP_FAILED);

    stub.set(ioctl, StubSuccess);
    stub.set(&CIPCFile::WriteResult, StubSuccess);
    iRet = caller.GetRawMajorMinor(strUniqueID);
    EXPECT_EQ(iRet, MP_SUCCESS);
}


/*
 * 用例名称：ִ同步数据文件缓存
 * 前置条件：无
 * check点：检查返回值
 */
TEST_F(CCommandMapTest, SyncDataFile)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    CSystemCall caller;
    mp_string strUniqueID = "0x123234";

    // 构造临时输入文件路径失败
    EXPECT_EQ(MP_FAILED, caller.SyncDataFile(strUniqueID));

    // 构造临时输入文件路径失败
    stub.set(&CSystemCall::GetParamFromTmpFile, GetParamFromTmpFileSucc);
    stub.set(realpath, StubRealpathNull);
    EXPECT_EQ(MP_FAILED, caller.SyncDataFile(strUniqueID));

    //打开文件失败
    stub.set(realpath, StubRealpath);
    stub.set(open, StubFailed);
    EXPECT_EQ(MP_FAILED, caller.SyncDataFile(strUniqueID));

    // 下刷缓存至硬盘失败
    stub.set(open, StubSuccess);
    stub.set(fsync, StubFailed);
    EXPECT_EQ(MP_FAILED, caller.SyncDataFile(strUniqueID));

    // 同步数据文件缓存成功
    stub.set(open, StubSuccess);
    stub.set(fsync, StubSuccess);
    EXPECT_EQ(MP_SUCCESS, caller.SyncDataFile(strUniqueID));
}

/*
 * 用例名称：ִ重新加载udev规则
 * 前置条件：无
 * check点：检查返回值
 */
TEST_F(CCommandMapTest, ReloadUDEVRules)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    CSystemCall caller;
    mp_string strUniqueID = "0x123234";

    stub.set(&CSystemExec::ExecSystemWithoutEchoNoWin, StubFailed);
    EXPECT_EQ(MP_FAILED, caller.ReloadUDEVRules(strUniqueID));

    stub.set(&CSystemExec::ExecSystemWithoutEchoNoWin, StubSuccess);
    EXPECT_EQ(MP_SUCCESS, caller.ReloadUDEVRules(strUniqueID));

    stub.set(&SecureCom::GetOSType, StubGetOSTypeRedhat);
    stub.set(&SecureCom::GetOSVersion, StubGetOSVersionFive);
    EXPECT_EQ(MP_SUCCESS, caller.ReloadUDEVRules(strUniqueID));

    stub.set(&SecureCom::GetOSType, StubGetOSTypeSuse);
    stub.set(&SecureCom::GetOSVersion, StubGetOSVersionTen);
    EXPECT_EQ(MP_SUCCESS, caller.ReloadUDEVRules(strUniqueID));

    stub.set(&SecureCom::GetOSType, StubGetOSTypeOther);
    stub.set(&CSystemExec::ExecSystemWithoutEchoNoWin, StubFailed);
    EXPECT_EQ(MP_FAILED, caller.ReloadUDEVRules(strUniqueID));
    stub.reset(&SecureCom::GetOSType);
    stub.reset(&SecureCom::GetOSVersion);
    stub.reset(&CSystemExec::ExecSystemWithoutEchoNoWin);
}

TEST_F(CCommandMapTest, CheckDirExist)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    CSystemCall caller;
    mp_string strUniqueID = "0x123234";

    stub.set(ADDR(CSystemCall, GetParamFromTmpFile), StubFailed);
    EXPECT_EQ(MP_FAILED, caller.CheckDirExist(strUniqueID));

    stub.set(ADDR(CSystemCall, GetParamFromTmpFile), StubSuccess);
    stub.set(&CMpFile::DirExist, StubFalse);
    EXPECT_EQ(ERROR_DEVICE_FILESYS_MOUNT_POINT_NOT_EXIST, caller.CheckDirExist(strUniqueID));

    stub.set(ADDR(CSystemCall, GetParamFromTmpFile), StubSuccess);
    stub.set(&CMpFile::DirExist, StubTrue);
    EXPECT_EQ(MP_SUCCESS, caller.CheckDirExist(strUniqueID));
}

TEST_F(CCommandMapTest, SignScripts)
{
    mp_int32 iRet;
    mp_string strUniqueID = "0x123234";
    CSystemCall caller;

    iRet = caller.SignScripts();
    EXPECT_EQ(iRet, MP_FAILED);

    stub.set(&SecureCom::GenSignFile, StubSuccess);
    iRet = caller.SignScripts();
    EXPECT_EQ(iRet, MP_SUCCESS);
}

/*
* 用例名称 : 启动获取数据进程
* 前置条件：无
* check点：1、读取临时文件失败，启动进程失败
*          2、读取临时文件成功，启动进程成功
*/
TEST_F(CCommandMapTest, StartDataProcess)
{
    mp_string strUniqueID = "0x123234";
    CSystemCall caller;
    stub.set(&CLogger::Log, StubCLoggerLog);

    EXPECT_EQ(MP_FAILED, caller.StartDataProcess(strUniqueID, std::vector<mp_string>()));

    std::vector<mp_string> vecParam = { ";6.7" };
    stub.set(CheckCmdDelimiter, StubSuccess);
    stub.set(popen, StubPopenNull);
    EXPECT_EQ(ERROR_COMMON_SYSTEM_CALL_FAILED, caller.StartDataProcess(strUniqueID, vecParam));

    stub.set(popen, StubPopen);
    EXPECT_EQ(MP_SUCCESS, caller.StartDataProcess(strUniqueID, vecParam));

    stub.reset(CheckCmdDelimiter);
    stub.reset(popen);
}

/*
* 用例名称 : 校验用户自定义脚本
* 前置条件：执行命令成功
* check点：1、自定义脚本合法，执行成功
*          2、自定义脚本非法，执行失败
*/
TEST_F(CSystemCallTest, CheckUserDefineScriptLegality)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    CSystemCall caller;
    mp_string strUniqueID = "/opt/DataBackup/ProtectClient/ProtectClient-E//../Plugins/Plugin/stop.sh /logpath 59610 59640 127.0.0.1 59578";

    stub.set(&CSystemExec::ExecSystemWithoutEchoNoWin, StubSuccess);
    stub.set(&CMpString::FormattingPath, StubFormattingPathSucc);
    EXPECT_EQ(MP_SUCCESS, caller.ExecUserDefineScript(strUniqueID));

    strUniqueID = "/opt/oceanstor/dataturbo/script/dataturbo_rescan";
    EXPECT_EQ(MP_SUCCESS, caller.ExecUserDefineScript(strUniqueID));
    strUniqueID = "/opt/oceanstor/dataturbo/root_rescan.sh";
    EXPECT_EQ(MP_FAILED, caller.ExecUserDefineScript(strUniqueID));
    strUniqueID = "/opt/oceanstor/dataturbo/script/dataturbo_rescan.sh";
    stub.set(&CMpString::FormattingPath, StubFormattingPathSucc);
    EXPECT_EQ(MP_FAILED, caller.ExecUserDefineScript(strUniqueID));
    stub.reset(&CSystemExec::ExecSystemWithoutEchoNoWin);

    strUniqueID = "touch /tmp/test";
    stub.set(&CMpString::FormattingPath, StubFormattingPathFail);
    EXPECT_EQ(MP_FAILED, caller.ExecUserDefineScript(strUniqueID));

    stub.set(&CMpString::FormattingPath, StubFalse);
    EXPECT_EQ(MP_FAILED, caller.ExecUserDefineScript(strUniqueID));
    stub.reset(&CMpString::FormattingPath);
}
#ifndef LINUX

/*
 * 用例名称：根据命令IDִ对参数进行白名单验证
 * 前置条件：无
 * check点：检查返回值
 */
TEST_F(CCommandMapTest, WhiteListVerifyFailed)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    mp_int32 iCommandID;
    mp_string strParam;
    CSystemCall caller;
    
    EXPECT_EQ(MP_TRUE, caller.WhiteListVerify(iCommandID, strParam));

    // iCommandID == ROOT_COMMAND_KILL
    iCommandID = ROOT_COMMAND_KILL;
    stub.set(&CSystemCall::KillWhiteListVerify, StubFalse);
    EXPECT_EQ(MP_FALSE, caller.WhiteListVerify(iCommandID, strParam));
    stub.set(&CSystemCall::KillWhiteListVerify, StubTrue);
    EXPECT_EQ(MP_TRUE, caller.WhiteListVerify(iCommandID, strParam));
    stub.reset(&CSystemCall::KillWhiteListVerify);

    // iCommandID == ROOT_COMMAND_CAT
    iCommandID = ROOT_COMMAND_CAT;
    stub.set(&CSystemCall::CatWhiteListVerify, StubTrue);
    EXPECT_EQ(MP_TRUE, caller.WhiteListVerify(iCommandID, strParam));
    stub.reset(&CSystemCall::CatWhiteListVerify);

    // iCommandID == ROOT_COMMAND_MOUNT
    iCommandID = ROOT_COMMAND_MOUNT;
    stub.set(&CSystemCall::MountWhiteListVerify, StubTrue);
    EXPECT_EQ(MP_TRUE, caller.WhiteListVerify(iCommandID, strParam));
    stub.reset(&CSystemCall::MountWhiteListVerify);

    // iCommandID == ROOT_COMMAND_RM
    iCommandID = ROOT_COMMAND_RM;
    stub.set(&CSystemCall::RmWhiteListVerify, StubTrue);
    EXPECT_EQ(MP_TRUE, caller.WhiteListVerify(iCommandID, strParam));
    stub.reset(&CSystemCall::RmWhiteListVerify);
}

/*
 * 用例名称：kill进程的白名单校验
 * 前置条件：无
 * check点：检查返回值
 */
TEST_F(CCommandMapTest, KillWhiteListVerify)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    mp_string strParam;
    CSystemCall caller;

    // 未获取到进程信息
    stub.set(&CSystemCall::GetProcessInfo, StubFalse);
    EXPECT_EQ(MP_TRUE, caller.KillWhiteListVerify(strParam));

    // 获取到进程信息，白名单校验未通过
    stub.set(&CSystemCall::GetProcessInfo, StubTrue);
    stub.set(&CSystemCall::VerifyProcess, StubFalse);
    EXPECT_EQ(MP_FALSE, caller.KillWhiteListVerify(strParam));

    // 获取到进程信息，白名单校验通过
    stub.set(&CSystemCall::VerifyProcess, StubTrue);
    EXPECT_EQ(MP_TRUE, caller.KillWhiteListVerify(strParam));
    stub.reset(&CSystemCall::GetProcessInfo);
    stub.reset(&CSystemCall::VerifyProcess);
}

/*
 * 用例名称：允许Cat路径的白名单校验
 * 前置条件：无
 * check点：检查返回值
 */
TEST_F(CCommandMapTest, CatWhiteListVerify)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    stub.set(&CMpString::FormattingPath, StubTrue);
    mp_string strParam;
    CSystemCall caller;

    EXPECT_EQ(MP_FALSE, caller.CatWhiteListVerify(strParam));
    strParam = "/etc/hosts";
    EXPECT_EQ(MP_TRUE, caller.CatWhiteListVerify(strParam));
    strParam = "/mnt/databackup/";
    EXPECT_EQ(MP_TRUE, caller.CatWhiteListVerify(strParam));
}

/*
 * 用例名称：允许挂载路径的白名单校验
 * 前置条件：无
 * check点：检查返回值
 */
TEST_F(CCommandMapTest, MountWhiteListVerify)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    mp_string strParam;
    CSystemCall caller;

    EXPECT_EQ(MP_TRUE, caller.MountWhiteListVerify(strParam));
    strParam = "/etc/hosts";
    EXPECT_EQ(MP_FALSE, caller.MountWhiteListVerify(strParam));
}

/*
 * 用例名称：获取进程信息
 * 前置条件：无
 * check点：检查返回值
 */
TEST_F(CCommandMapTest, GetProcessInfo)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    mp_string strParam;
    mp_string result;
    CSystemCall caller;

    stub.set(popen, StubPopenNull);
    EXPECT_EQ(MP_FALSE, caller.GetProcessInfo(strParam, result));

    stub.set(popen, StubPopen);
    stub.set(feof, StubTrue);
    stub.set(pclose, StubTrue);
    EXPECT_EQ(MP_FALSE, caller.GetProcessInfo(strParam, result));
    
    g_StubFalseOnZero_flag = 1;
    strParam = "test";
    stub.set(feof, StubFalseOnZero);
    stub.set(pclose, StubTrue);
    stub.set(fgets, Stubfgets);
    EXPECT_EQ(MP_FALSE, caller.GetProcessInfo(strParam, result));
    stub.reset(feof);
    stub.reset(pclose);
    stub.reset(fgets);
    stub.reset(popen);
}

/*
 * 用例名称：校验进程信息
 * 前置条件：无
 * check点：检查返回值
 */
TEST_F(CCommandMapTest, VerifyProcess)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    stub.set(&CMpString::FormattingPath, StubFormattingPathSucc);
    mp_string processInfo;
    mp_string result;
    CSystemCall caller;
    mp_string jarStr = "/opt/DataBackup/ProtectClient/Plugins/HadoopPlugin/lib/databackup.agent.jar";
    
    processInfo = "c c";
    EXPECT_EQ(MP_FALSE, caller.VerifyProcess(processInfo));

    // tmpVec[AWK_COL_FIRST_4] == "java"的情况
    processInfo = "c c c c java";
    processInfo += " -jar ";
    processInfo += jarStr;
    EXPECT_EQ(MP_TRUE, caller.VerifyProcess(processInfo));

    processInfo = "c c c c java" + jarStr;
    EXPECT_EQ(MP_FALSE, caller.VerifyProcess(processInfo));
    processInfo = "c c c c java -jar /opt";
    EXPECT_EQ(MP_FALSE, caller.VerifyProcess(processInfo));

    // tmpVec[AWK_COL_FIRST_4] != "java"的情况
    mp_string dataprocesspath = "/opt/DataBackup/ProtectClient/ProtectClient-E/sbin/dataprocess";
    mp_string naspluginpath = "/opt/DataBackup/ProtectClient/Plugins/NasPlugin/bin/AppPlugin_NAS";
    processInfo = "c c c c " + dataprocesspath;
    EXPECT_EQ(MP_TRUE, caller.VerifyProcess(processInfo));

    processInfo = "c c c c " + naspluginpath;
    EXPECT_EQ(MP_TRUE, caller.VerifyProcess(processInfo));

    processInfo = "c c c c /opt";
    EXPECT_EQ(MP_FALSE, caller.VerifyProcess(processInfo));
}

/*
 * 用例名称：对路劲进行白名单校验
 * 前置条件：无
 * check点：检查返回值
 */
TEST_F(CCommandMapTest, RmWhiteListVerify)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    mp_string filePath;
    CSystemCall caller;

    filePath = "../stmp";
    EXPECT_EQ(MP_FALSE, caller.RmWhiteListVerify(filePath));

    filePath = "/opt/DataBackup/ProtectClient/";
    stub.set(&CMpString::FormattingPath, StubFalse);
    EXPECT_EQ(MP_FALSE, caller.RmWhiteListVerify(filePath));

    stub.set(&CMpString::FormattingPath, StubTrue);
    EXPECT_EQ(MP_FALSE, caller.RmWhiteListVerify(filePath));

    stub.set(&CMpString::FormattingPath, StubFormattingPathTrue);
    EXPECT_EQ(MP_TRUE, caller.RmWhiteListVerify(filePath));
    stub.reset(&CMpString::FormattingPath);
}
#endif

#ifdef LIN_FRE_SUPP
/*
 * 用例名称：ִ冻结文件系统的操作
 * 前置条件：无
 * check点：strDriveLetter冻结，解冻所需的挂载点 检查返回值
 */
TEST_F(CCommandMapTest, FreezeFileSys)
{
    mp_int32 iRet;
    mp_string strUniqueID = "0x123234";
    CSystemCall caller;
    stub.set(&CLogger::Log, StubCLoggerLog);

    stub.set(&CMpFile::FileExist, StubFalse);
    iRet = caller.FreezeFileSys(strUniqueID);
    EXPECT_EQ(iRet, ERROR_COMMON_OPER_FAILED);

    // 文件存在，读取失败
    stub.set(&CMpFile::FileExist, StubTrue);
    stub.set(&CIPCFile::ReadInput, StubFailed);
    iRet = caller.FreezeFileSys(strUniqueID);
    EXPECT_EQ(iRet, MP_FAILED);

    // 文件存在，读取成功，打开读出字符串为文件名的文件 失败
    stub.set(&CIPCFile::ReadInput, StubSuccess);
    stub.set(open, StubFailed);
    iRet = caller.FreezeFileSys(strUniqueID);
    EXPECT_EQ(iRet, ERROR_COMMON_OPER_FAILED);
}
#endif

/*
 * 用例名称：ִ命令参数校验操作
 * 前置条件：命令不在校验列表
 * check点：校验列表，校验成功
 */
TEST_F(CCommandMapTest, WhiteListVerifyTest)
{
    CCommandMap commonMap;
    std::vector<mp_string> vecParam;
    mp_int32 iRet = commonMap.WhiteListVerify(1000, vecParam);
    EXPECT_EQ(iRet, MP_TRUE);
}

/*
 * 用例名称：升级命令校验
 * 前置条件：进程在白名单列表
 * check点：GetProcessInfo获取进程信息，校验返回值
 */
TEST_F(CCommandMapTest, UpgradeCallVerifyTest)
{
    CCommandMap commonMap;
    std::vector<mp_string> vecParam;
    stub.set(ADDR(CSystemCall, GetParentPid), StubGetParentPid_fail);
    mp_int32 iRet = commonMap.UpgradeCallVerify(vecParam);
    EXPECT_EQ(iRet, MP_FALSE);

    stub.set(ADDR(CSystemCall, GetParentPid), StubGetParentPid_succ);
    stub.set(ADDR(CSystemCall, GetProcessInfo), StubGetProcessInfo_fail);
    iRet = commonMap.UpgradeCallVerify(vecParam);
    EXPECT_EQ(iRet, MP_FALSE);

    stub.set(ADDR(CSystemCall, GetParentPid), StubGetParentPid_succ);
    stub.set(ADDR(CSystemCall, GetProcessInfo), StubGetProcessInfo_succ);
    iRet = commonMap.UpgradeCallVerify(vecParam);
    EXPECT_EQ(iRet, MP_TRUE);
}

