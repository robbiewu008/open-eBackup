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
#include "rootexec/SystemCall.h"
#include <algorithm>
#include <fstream>
#include <array>
#include "array/array.h"
#include "array/disk.h"
#include "common/ConfigXmlParse.h"
#include "common/Defines.h"
#include "common/ErrorCode.h"
#include "common/Log.h"
#include "common/Path.h"
#include "common/File.h"
#include "common/Pipe.h"
#include "common/CMpThread.h"
#include "securecom/UniqueId.h"
#include "common/Utils.h"
#include "securecom/RootCaller.h"
#include "common/CSystemExec.h"
#include "securecom/SecureUtils.h"
#include "securec.h"

#ifdef LINUX
#include <linux/raw.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <sstream>

#endif

#ifdef LIN_FRE_SUPP
#include <linux/fs.h>

#endif
using namespace std;

namespace {
    constexpr mp_int32 THIRDSCRIPT_NAME_INDEX = 0;
    constexpr mp_int32 THIRDSCRIPT_TYPE_INDEX = 1;
    constexpr mp_int32 THIRDSCRIPT_PARAM_INDEX = 2;
    constexpr mp_int32 IMPORT_CERT_PARAM_NUMBER = 2;
    constexpr mp_int32 WRITE_SCAN_RESULT_PARAM_NUMBER = 3;
    const mp_int32 MAX_READ_NUM = 1600;
    const mp_uint32 NUM_0 = 0;
    const mp_uint32 MB_TO_BYTE = 1024 * 1024;
    const mp_string IPV4_STRING = "127.0.0.1";
    const mp_string IPV6_STRING = "::1";
    const mp_string ETC_HOSTS_PATH_LINUX = "/etc/hosts";
}

/* ------------------------------------------------------
Function Name: GetHostLunID
Description     : 获取 通道的设备列表
Input             : strDevice -- 设备名称
Return           : MP_SUCCESS 成功
                   非MP_SUCCESS失败
Create By      : wangxingping wwx549190
-------------------------------------------------------- */
mp_int32 CSystemCall::GetHostLunID(const mp_string& strUniqueID, const std::vector<mp_string>& vecParam)
{
    LOGGUARD("");
    mp_string strDevice = vecParam.empty() ? "" : vecParam.front();
    vector<mp_int32> vecHostLunID;
    mp_int32 iRet = Array::GetHostLunIDList(strDevice, vecHostLunID);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "GetHostLunID::GetHostLunIDList, ret %d.", iRet);
        return iRet;
    }
    vector<mp_string> vecResult;
    mp_string str;
    stringstream ss_stream;
    for (vector<mp_int32>::iterator iter = vecHostLunID.begin(); iter != vecHostLunID.end(); ++iter) {
        ss_stream.clear();
        ss_stream << *iter;
        ss_stream >> str;
        vecResult.push_back(str.c_str());
    }
    iRet = CIPCFile::WriteResult(strUniqueID, vecResult);

    return iRet;
}

/* ------------------------------------------------------------
Function Name: ExecSysCmd
Description  : ִ执行系统命令，从加密的输入临时文件中读出并进行解密，执行，将结果写入结果临时文件
Others       :-------------------------------------------------------- */
mp_int32 CSystemCall::ExecSysCmd(const mp_string& strUniqueID, mp_int32 iCommandID,
    const std::vector<mp_string>& vecParam)
{
    LOGGUARD("");
    mp_string strParam = CMpString::StrJoin(vecParam, " ");
    CCommandMap commandMap;
    if (commandMap.NoParam(iCommandID) && !strParam.empty()) {
        COMMLOG(OS_LOG_ERROR, "Command[%d] should no param, ", iCommandID);
        return MP_FAILED;
    }

    mp_int32 iRet = CheckCmdDelimiter(strParam);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "The parameter contains prohibited characters.");
        return iRet;
    }

    if (!CSystemCall::WhiteListVerify(iCommandID, strParam)) {
        ERRLOG("Failed to verify the trustlist.");
        return MP_FAILED;
    }

    // 组装命令
    mp_string strExecCmd = commandMap.GetCommandString(iCommandID) + " " + strParam;
    COMMLOG(OS_LOG_DEBUG, "Command to be executed is \"%s\".", strExecCmd.c_str());
    mp_bool bNeedEcho = commandMap.NeedEcho(iCommandID);
    vector<mp_string> vecResult;
    if (bNeedEcho) {
        // 如果有其他不需要重定向命令，在这里添加
        iRet = CSystemExec::ExecSystemWithEcho(strExecCmd, vecResult);
    } else {
        iRet = CSystemExec::ExecSystemWithoutEcho(strExecCmd);
    }

    if (iRet != MP_SUCCESS) {
        // 记录日志
        COMMLOG(OS_LOG_ERROR, "Call ExecSystemWithoutEcho failed, ret %d.", iRet);
        return iRet;
    }

    // 将执行结果写入结果文件
    if (bNeedEcho) {
        return CIPCFile::WriteResult(strUniqueID, vecResult);
    } else {
        return MP_SUCCESS;
    }
}

/* -----------------------------------------------------------
Function Name: ExecScript
Description  : ִ执行脚本命令，不处理参数，由脚本处理,将结果写入临时结果文件中
Others       :-------------------------------------------------------- */
mp_int32 CSystemCall::ExecScript(const mp_string& strUniqueID, mp_int32 iCommandID,
    const std::vector<mp_string>& vecParam)
{
    LOGGUARD("");
    // 根据iCommandID获取具体脚本名称
    CCommandMap commandMap;
    if (!commandMap.WhiteListVerify(iCommandID, vecParam)) {
        ERRLOG("Failed to verify the process trustlist.");
        return ERROR_SCRIPT_COMMON_PARAM_WRONG;
    }

    mp_string strScriptName = commandMap.GetCommandString(iCommandID);
    COMMLOG(OS_LOG_INFO, "Script name is \"%s\".", strScriptName.c_str());
    mp_string strScriptPath = CPath::GetInstance().GetSBinFilePath(strScriptName);
    if (!CMpFile::FileExist(strScriptPath)) {
        ERRLOG("Script %s is not exist.",  strScriptName.c_str());
        return INTER_ERROR_SRCIPT_FILE_NOT_EXIST;
    }
    mp_string strAgentPath = CMpString::BlankComma(CPath::GetInstance().GetRootPath());
    // 组装命令
    mp_char acCmd[MAX_MAIN_CMD_LENGTH] = {0};
    CHECK_FAIL(snprintf_s(acCmd, sizeof(acCmd), sizeof(acCmd) - 1, "%s %s %s",
        CMpString::BlankComma(strScriptPath).c_str(), strAgentPath.c_str(), strUniqueID.c_str()));
    mp_string strCmd = acCmd;

    COMMLOG(OS_LOG_DEBUG, "Command \"%s\" will be executed.", strCmd.c_str());
    CHECK_FAIL_EX(CheckCmdDelimiter(strCmd));
    // 执行脚本均不获取回显
    mp_int32 iRet = CSystemExec::ExecSystemWithSecurityParam(strCmd, vecParam);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Script \"%s\" excuted failed, iRet = %d.", strCmd.c_str(), iRet);
    }

    // 设置结果文件权限
    mp_int32 iChowRet = CIPCFile::ChownResult(strUniqueID);
    if (iChowRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Chown result file failed, UniqueID %s.", strUniqueID.c_str());
    }
    return iRet;
}

/* --------------------------------------------------------
Function Name: ExecEbkUserScript
Description  : ִ执行APP脚本命令
-------------------------------------------------------- */
mp_int32 CSystemCall::ExecEbkUserScript(mp_string& strScriptFileName, const mp_string& strUniqueID)
{
    LOGGUARD("");
    mp_string strAgentPath = CPath::GetInstance().GetRootPath();

    // 校验文件权限
    mp_int32 iRet = CMpFile::CheckFileAccess(strScriptFileName);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_INFO, "Script \"%s\" has wrong access, iRet=%d.", strScriptFileName.c_str(), iRet);
        return ERROR_SCRIPT_APP_FAILED;
    }

    // 组装命令
    strScriptFileName = CMpString::BlankComma(strScriptFileName);
    strAgentPath = CMpString::BlankComma(strAgentPath);
    mp_char acCmd[MAX_MAIN_CMD_LENGTH] = {0};
    CHECK_FAIL(snprintf_s(acCmd,
        sizeof(acCmd),
        sizeof(acCmd) - 1,
        "%s %s %s",
        strScriptFileName.c_str(),
        strAgentPath.c_str(),
        strUniqueID.c_str()));
    mp_string strLogCmd = strScriptFileName + " " + strAgentPath + " " + strUniqueID;

    COMMLOG(OS_LOG_INFO, "Script \"%s\" will be excuted.", strLogCmd.c_str());

    mp_string strExecCmd = acCmd;
    COMMLOG(OS_LOG_DEBUG, "Command \"%s\" will be executed.", strLogCmd.c_str());
    CHECK_FAIL_EX(CheckCmdDelimiter(strExecCmd));
    // 执行脚本均不获取回显
    iRet = CSystemExec::ExecSystemWithoutEcho(strExecCmd);
    if (iRet != MP_SUCCESS) {
        if (iRet != (mp_int32)ERROR_COMMON_INVALID_PARAM && iRet != (mp_int32)ERROR_COMMON_SYSTEM_CALL_FAILED) {
            COMMLOG(OS_LOG_ERROR, "Script \"%s\" excuted failed, iRet = %d.", strLogCmd.c_str(), iRet);
            return ERROR_SCRIPT_COMMON_EXEC_FAILED;
        }
        COMMLOG(OS_LOG_ERROR, "Excute script \"%s\" failed, iRet = %d.", strLogCmd.c_str(), iRet);
        return ERROR_SCRIPT_APP_FAILED;
    }

    // 设置结果文件权限
    iRet = CIPCFile::ChownResult(strUniqueID);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Chown result file failed, UniqueID %s.", strUniqueID.c_str());
        return ERROR_SCRIPT_APP_FAILED;
    }

    COMMLOG(OS_LOG_INFO, "Execute script %s success.", strScriptFileName.c_str());
    return MP_SUCCESS;
}

/* --------------------------------------------------------
Function Name: ExecAppScript
Description  : ִ执行APP脚本命令
-------------------------------------------------------- */
mp_int32 CSystemCall::ExecAppScript(const mp_string& strUniqueID, mp_int32 iCommandID)
{
    LOGGUARD("");
    mp_int32 iRet;
    vector<mp_string> vecRlt;
    vector<mp_string>::iterator iter;
    // 查找ebk_user下的脚本
    mp_string strScriptPath = CPath::GetInstance().GetEbkUserPath();
    iRet = GetAPPScripts(iCommandID, strScriptPath, vecRlt);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get app script failed. iRet %d.", iRet);
        return iRet;
    }

    if (vecRlt.size() == 0) {
        COMMLOG(OS_LOG_WARN, "There have no app script file.");
        return INTER_ERROR_SRCIPT_FILE_NOT_EXIST;
    }

    for (iter = vecRlt.begin(); iter != vecRlt.end(); ++iter) {
        mp_string strScriptFileName = strScriptPath + PATH_SEPARATOR + (*iter);
        iRet = ExecEbkUserScript(strScriptFileName, strUniqueID);
        if (iRet != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "ExecEbkUserScript failed, iRet %d.", iRet);
            return iRet;
        }
    }

    COMMLOG(OS_LOG_INFO, "Execute command %d success.", iCommandID);
    return MP_SUCCESS;
}

/* -------------------------------------------------------
Function Name: GetAPPScripts
Description  : 获取APP脚本文件
-------------------------------------------------------- */
mp_int32 CSystemCall::GetAPPScripts(
    mp_int32 iCommandID, const mp_string& appScriptFolder, std::vector<mp_string>& vecScripts)
{
    std::vector<mp_string> vecFileList;
    mp_string scriptCMD = ((iCommandID == ROOT_COMMAND_SCRIPT_FREEZEAPP) ?
        "freeze" :
        ((iCommandID == ROOT_COMMAND_SCRIPT_THAWAPP) ? "unfreeze" : "unknow"));
    COMMLOG(OS_LOG_INFO, "Get app %s script file.", scriptCMD.c_str());

    vecScripts.clear();
    mp_string fileFolder = appScriptFolder;
    mp_int32 iRet = CMpFile::GetFolderFile(fileFolder, vecFileList);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get app script failed, app script folder:%s.", appScriptFolder.c_str());
        return ERROR_SCRIPT_APP_FAILED;
    }
    for (std::vector<mp_string>::iterator it = vecFileList.begin(); it != vecFileList.end(); ++it) {
        mp_string fileKey = "";
        if (iCommandID == ROOT_COMMAND_SCRIPT_FREEZEAPP) {
            fileKey = "_freeze.sh";
        } else if (iCommandID == ROOT_COMMAND_SCRIPT_THAWAPP) {
            fileKey = "_unfreeze.sh";
        }

        if (fileKey == "") {
            continue;
        }
        mp_string& fileName = *it;
        std::string::size_type pos = fileName.find(fileKey);
        // 只取文件名以 filekey结尾的文件
        if ((std::string::npos != pos) && (fileName.length() == (pos + fileKey.length()))) {
            vecScripts.push_back(fileName);
            COMMLOG(OS_LOG_DEBUG, "The %s script file is %s.", scriptCMD.c_str(), fileName.c_str());
        }
    }

    return MP_SUCCESS;
}

/* --------------------------------------------------------
Function Name: ExecAddFirewall
Description  : 添加防火墙
Return       : 0: 成功, 其他: 失败
-------------------------------------------------------- */
mp_int32 CSystemCall::ExecAddFirewall()
{
    COMMLOG(OS_LOG_INFO, "Begin add firewall.");

#ifndef WIN32
    mp_string strScriptPath = CPath::GetInstance().GetSBinFilePath("firewall_tools.sh");
    strScriptPath = CMpString::BlankComma(strScriptPath);
    mp_string strCmd = strScriptPath + " add";
    CHECK_FAIL_EX(CheckCmdDelimiter(strCmd));
    COMMLOG(OS_LOG_DEBUG, "Command \"%s\" will be executed.", strCmd.c_str());
    // 执行脚本均不获取回显
    mp_int32 iRet = CSystemExec::ExecSystemWithoutEcho(strCmd);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Excute script \"%s\" failed, iRet = %d.", strCmd.c_str(), iRet);
        return ERROR_SCRIPT_COMMON_EXEC_FAILED;
    }

    COMMLOG(OS_LOG_INFO, "Add firewall success.");
    return MP_SUCCESS;
#else
    COMMLOG(OS_LOG_INFO, "No need add firewall on windows.");
    return MP_SUCCESS;
#endif
}

/* ------------------------------------------------------------
Function Name: GetDisk80Page
Description  : 获取磁盘80页信息，从临时文件中读取输入参数
               执行成功后，将结果写入结果文件中
Others       :-------------------------------------------------------- */
mp_int32 CSystemCall::GetDisk80Page(const mp_string& strUniqueID, const std::vector<mp_string>& vecParam)
{
    LOGGUARD("");
    mp_string strDevice = vecParam.empty() ? "" : vecParam.front();

    mp_string strSN;
    mp_int32 iRet = Array::GetDisk80Page(strDevice, strSN);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "CDiskPage::GetDisk80Page, ret %d.", iRet);
        return iRet;
    }

    vector<mp_string> vecResult;
    vecResult.push_back(strSN);
    return CIPCFile::WriteResult(strUniqueID, vecResult);
}

/* ------------------------------------------------------------
Function Name: GetDisk83Page
Description  : 查询disk 83页
Others       :-------------------------------------------------------- */
mp_int32 CSystemCall::GetDisk83Page(const mp_string& strUniqueID, const std::vector<mp_string>& vecParam)
{
    LOGGUARD("");
    mp_string strDevice = vecParam.empty() ? "" : vecParam.front();

    mp_string strLunWWN;
    mp_string strLunID;
    mp_int32 iRet = Array::GetDisk83Page(strDevice, strLunWWN, strLunID);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "CDiskPage::GetDisk80Page, ret %d.", iRet);
        return iRet;
    }

    vector<mp_string> vecResult;
    vecResult.push_back(strLunWWN);
    vecResult.push_back(strLunID);
    return CIPCFile::WriteResult(strUniqueID, vecResult);
}

/* ------------------------------------------------------------
Function Name: GetDisk00Page
Description  : 查询disk 00页
Return       : Support VPD pages
Called by    : Array::GetDisk00Page
create by    : longjiang wx290533
Others       :-------------------------------------------------------- */
mp_int32 CSystemCall::GetDisk00Page(const mp_string& strUniqueID, const std::vector<mp_string>& vecParam)
{
    LOGGUARD("");
    mp_string strDevice = vecParam.empty() ? "" : vecParam.front();

    vector<mp_string> vecResult;
    mp_int32 iRet = Array::GetDisk00Page(strDevice, vecResult);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "CDiskPage::GetDisk80Page, ret %d.", iRet);
        return iRet;
    }

    return CIPCFile::WriteResult(strUniqueID, vecResult);
}

/* ------------------------------------------------------------
Function Name: GetDiskC8Page
Description  : 查询disk 83页
Return       : lunid
Called by    : Array::GetDiskC8Page
Modification : longjiang wx290533
Others       :-------------------------------------------------------- */
mp_int32 CSystemCall::GetDiskC8Page(const mp_string& strUniqueID, const std::vector<mp_string>& vecParam)
{
    LOGGUARD("");
    mp_string strDevice = vecParam.empty() ? "" : vecParam.front();

    mp_string strLunID;
    mp_int32 iRet = Array::GetDiskC8Page(strDevice, strLunID);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "CDiskPage::GetDiskC8Page, ret %d.", iRet);
        return iRet;
    }

    vector<mp_string> vecResult;
    if (!strLunID.empty()) {
        vecResult.push_back(strLunID);
    }
    return CIPCFile::WriteResult(strUniqueID, vecResult);
}

/* ------------------------------------------------------------
Function Name: GetDiskCapacity
Description  : 查询磁盘容量
Others       :-------------------------------------------------------- */
mp_int32 CSystemCall::GetDiskCapacity(const mp_string& strUniqueID, const std::vector<mp_string>& vecParam)
{
    LOGGUARD("");
    mp_string strDevice = vecParam.empty() ? "" : vecParam.front();

    mp_string strBuf;
    mp_int32 iRet = Disk::GetDiskCapacity(strDevice, strBuf);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "CDiskPage::GetDisk80Page, ret %d.", iRet);
        return iRet;
    }

    vector<mp_string> vecResult;
    vecResult.push_back(strBuf);
    return CIPCFile::WriteResult(strUniqueID, vecResult);
}

/* ------------------------------------------------------------
Function Name: GetDisk83Page
Description  : 获取磁盘83页信息，从临时文件中读取输入参数
               执行成功后，将结果写入结果文件中
Others       :-------------------------------------------------------- */
mp_int32 CSystemCall::GetVendorAndProduct(const mp_string& strUniqueID, const std::vector<mp_string>& vecParam)
{
    COMMLOG(OS_LOG_INFO, "Begin GetVendorAndProduct.");
    mp_string strDevice = vecParam.empty() ? "" : vecParam.front();

    mp_string strVendor;
    mp_string strProduct;
    mp_int32 iRet = Array::GetDiskArrayInfo(strDevice, strVendor, strProduct);
    if (iRet != MP_SUCCESS) {
        // ��¼��־
        COMMLOG(OS_LOG_ERROR, "Array::GetVendor, ret %d.", iRet);
        return iRet;
    }
    COMMLOG(OS_LOG_INFO, "End GetDiskArrayInfo.");
    // 将执行结果写入结果文件
    vector<mp_string> vecResult;
    vecResult.push_back(strVendor);
    vecResult.push_back(strProduct);
    return CIPCFile::WriteResult(strUniqueID, vecResult);
}

/* ------------------------------------------------------------
Function Name: GetRawMajorMinor
Description  : 获取raw设备的主分区和此分区信息
               执行成功后，将结果写入结果文件中
-------------------------------------------------------- */
mp_int32 CSystemCall::GetRawMajorMinor(const mp_string& strUniqueID)
{
    mp_int32 iRet;
#ifdef LINUX
    // 构造临时输入文件路径
    mp_string strMinor = "";
    iRet = GetParamFromTmpFile(strUniqueID, strMinor);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Call GetParamFormTmpFile failed, ret %d.", iRet);
        return iRet;
    }

    mp_int32 fd = open(RAW_CTRL_FILE.c_str(), O_RDWR, 0);
    if (fd < 0) {
        fd = open(RAW_CTRL_FILE_NEW.c_str(), O_RDWR, 0);
        if (fd < 0) {
            COMMLOG(OS_LOG_ERROR,
                "Open raw control device \"%s\" and \"%s\" failed.",
                RAW_CTRL_FILE.c_str(),
                RAW_CTRL_FILE_NEW.c_str());
            return MP_FAILED;
        }
    }

    COMMLOG(OS_LOG_DEBUG,
        "Open raw control device \"%s\" or \"%s\" succ.",
        RAW_CTRL_FILE.c_str(),
        RAW_CTRL_FILE_NEW.c_str());

    struct raw_config_request rq;
    rq.raw_minor = atoi(strMinor.c_str());
    iRet = ioctl(fd, RAW_GETBIND, &rq);
    if (iRet < 0) {
        close(fd);
        COMMLOG(OS_LOG_DEBUG, "Get raw device bounded info failed, errno %d.", errno);
        return MP_FAILED;
    }

    COMMLOG(OS_LOG_DEBUG, "major = %d, minor = %d", rq.block_major, rq.block_minor);
    vector<mp_string> vecResult;
    ostringstream oss;
    ostringstream oss1;
    oss << (mp_int32)rq.block_major;
    oss1 << (mp_int32)rq.block_minor;
    vecResult.push_back(oss.str());
    vecResult.push_back(oss1.str());
    close(fd);
    iRet = CIPCFile::WriteResult(strUniqueID, vecResult);
#else
    iRet = MP_FAILED;
#endif
    return iRet;
}

/* ------------------------------------------------------------
Function Name: ExecThirdPartyScript
Description  : ִ执行第三方脚本，传入参数为脚本名称和脚本真实参数，中间用冒号分隔
Others       :-------------------------------------------------------- */
mp_int32 CSystemCall::ExecThirdPartyScript(const mp_string& strUniqueID, const std::vector<mp_string>& vecParam)
{
    LOGGUARD("Start exec thirdParty script.");
    if (vecParam.size() < THIRDSCRIPT_PARAM_INDEX) {
        ERRLOG("ThirdPartyScript vecParam is invalid, %d.", vecParam.size());
        return MP_FAILED;
    }
    mp_string strScriptName = vecParam[THIRDSCRIPT_NAME_INDEX];
    mp_string isUserDefined = vecParam[THIRDSCRIPT_TYPE_INDEX];
    std::vector<mp_string> vecValue;
    vecValue.assign(vecParam.begin() + THIRDSCRIPT_PARAM_INDEX, vecParam.end());

    mp_string strScriptPath = CMpString::BlankComma(
        CPath::GetInstance().GetThirdPartyFilePath(strScriptName, isUserDefined));
    if (!CMpFile::FileExist(strScriptPath)) {
        ERRLOG("Script %s is not exist.", strScriptName.c_str());
        return INTER_ERROR_SRCIPT_FILE_NOT_EXIST;
    }
    CMpString::FormattingPath(strScriptPath);
    CHECK_FAIL_EX(CheckPathString(strScriptPath, CPath::GetInstance().GetThirdPartyPath()));

    // 为保证和R3C10兼容，脚本参数顺序调整为UID,PATH
    mp_string strAgentPath =  CMpString::BlankComma(CPath::GetInstance().GetRootPath());
    mp_string strCmd = strScriptPath + " " + strUniqueID + " " + strAgentPath;
    CHECK_FAIL_EX(CheckCmdDelimiter(strCmd));
    mp_int32 iRet = CSystemExec::ExecSystemWithSecurityParam(strCmd, vecValue);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "ExecSystemWithoutEcho failed, iRet is %d.", iRet);
    }
    return iRet;
}

/* ------------------------------------------------------------
Function Name: ExecScriptByScriptUser
Description  : 以脚本用户执行脚本，传入参数为脚本名称和脚本真实参数，中间用冒号分隔
Others       :-------------------------------------------------------- */
mp_int32 CSystemCall::ExecScriptByScriptUser(const mp_string& strUniqueID,
    const std::vector<mp_string>& vecParam)
{
    LOGGUARD("Start exec script by script user.");
    if (vecParam.size() < THIRDSCRIPT_PARAM_INDEX) {
        ERRLOG("User Script vecParam is invalid, %d.", vecParam.size());
        return MP_FAILED;
    }
    mp_string strScriptName = vecParam[THIRDSCRIPT_NAME_INDEX];
    mp_string isUserDefined = vecParam[THIRDSCRIPT_TYPE_INDEX];
    std::vector<mp_string> vecValue;
    vecValue.assign(vecParam.begin() + THIRDSCRIPT_PARAM_INDEX, vecParam.end());

    // exist confirm
    mp_string strScriptPath = CMpString::BlankComma(strScriptName);
    if (!CMpFile::FileExist(strScriptPath)) {
        ERRLOG("Script %s is not exist.", strScriptName.c_str());
        return INTER_ERROR_SRCIPT_FILE_NOT_EXIST;
    }
    if (!CMpString::FormattingPath(strScriptPath)) {
        ERRLOG("Script path is invalid, %s.", strScriptPath.c_str());
        return MP_FAILED;
    }

    // get script user
    mp_string scriptSuer;
    mp_int32 statRet = GetFileOwnerName(strScriptPath, scriptSuer);
    if (statRet != MP_SUCCESS || scriptSuer.empty()) {
        ERRLOG("Get script user failed, script=%s.", strScriptPath.c_str());
        return MP_FAILED;
    }
    mp_string swithUserPrefix = "su - " + scriptSuer + " ";

    // 为保证和R3C10兼容，脚本参数顺序调整为UID,PATH
    mp_string strAgentPath =  CMpString::BlankComma(CPath::GetInstance().GetRootPath());
    mp_string strCmd = strScriptPath + " " + strUniqueID + " " + strAgentPath;
    CHECK_FAIL_EX(CheckCmdDelimiter(strCmd));

    // use script's user
    strCmd = swithUserPrefix + strCmd;

    mp_string strFileName = RESULT_TMP_FILE + strUniqueID;
    mp_string strFilePathFile = CPath::GetInstance().GetStmpFilePath(strFileName);
    mp_int32 iRet = CSystemExec::ExecSystemWithSecurityParam(strCmd, vecValue, MP_TRUE, strFilePathFile);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Exec SystemWithSecurityParam cmd failed, iRet is %d.", iRet);
    }
    // 设置结果文件权限
    mp_int32 iChowRet = CIPCFile::ChownResult(strUniqueID);
    if (iChowRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Chown result file failed, UniqueID %s.", strUniqueID.c_str());
    }
    return iRet;
}

/* ------------------------------------------------------------
Function Name: ExecUserDefineScript
Description  : ִ执行由用户指定的脚本
Input        : userDefineCmd:要执行的脚本的全路径加名称以及参数。如"/xx/yyy.sh 1"
Others       :-------------------------------------------------------- */
mp_int32 CSystemCall::ExecUserDefineScript(const mp_string &userDefineCmd)
{
    CHECK_FAIL_EX(CheckCmdDelimiter(userDefineCmd));
    if (CheckUserDefineScriptLegality(userDefineCmd) != MP_SUCCESS) {
        ERRLOG("User define script command is invalid, cmd %s", userDefineCmd.c_str());
        return MP_FAILED;
    }
    mp_uint32 iRet = CSystemExec::ExecSystemWithoutEcho(userDefineCmd);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "ExecSystemWithoutEcho failed, iRet is %d.", iRet);
    }
    return iRet;
}

/* ------------------------------------------------------------
Function Name: BatchGetLUNInfo
Description  : 批量获取LUN信息
               传入/dev/sdb;/dev/sdc
               获取devicename;vendor;product;arraysn;lunid;wwn
                         /dev/sdb;HUAWEI;S5500T;210235G6GR10D7000004;218;6200bc71001f37540769e56b000000da
                         /dev/sdc;HUAWEI;S5500T;210235G6GR10D7000004;218;6200bc71001f37540769e56b000000da
-------------------------------------------------------- */
mp_int32 CSystemCall::BatchGetLUNInfo(const mp_string& strUniqueID, const std::vector<mp_string>& vecParam)
{
    LOGGUARD("");
    mp_string strParam = vecParam.empty() ? "" : vecParam.front();

    COMMLOG(OS_LOG_DEBUG, "Begin to batch get lun info %s", strParam.c_str());
    vector<mp_string> vecDevs;
    CMpString::StrSplitEx(vecDevs, strParam, STR_SEMICOLON);
    if (vecDevs.empty()) {
        COMMLOG(OS_LOG_ERROR, "BatchGetLUNInfo failed, device list is empty.");
        return MP_FAILED;
    }

    mp_string strDevice;
    mp_string strLUNInfo;
    // 分析设备信息，获取LUN的信息
    vector<mp_string> vecResult;
    for (vector<mp_string>::iterator iter = vecDevs.begin(); iter != vecDevs.end(); ++iter) {
        strDevice = *iter;
        if (strDevice.empty()) {
            continue;
        }

        mp_int32 iRet = GetLUNInfo(strDevice, strLUNInfo);
        if (iRet != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "Batch get LUN info failed.");
            return iRet;
        }
        vecResult.push_back(strLUNInfo);
        strLUNInfo.clear();
    }

    COMMLOG(OS_LOG_DEBUG, "Batch get lun info success.");
    // 写结果文件
    return CIPCFile::WriteResult(strUniqueID, vecResult);
}

/* ------------------------------------------------------------
Function Name: GetLUNInfo
Others        :-------------------------------------------------------- */
mp_int32 CSystemCall::GetLUNInfo(mp_string& strDevice, mp_string& strLUNInfo)
{
    COMMLOG(OS_LOG_DEBUG, "Begin to batch get this lun( %s ) info.", strDevice.c_str());
    // 厂商和型号
    mp_string strVendor;
    mp_string strProduct;
    mp_int32 iRet = Array::GetDiskArrayInfo(strDevice, strVendor, strProduct);
    if (iRet != MP_SUCCESS) {
        // 记录日志
        COMMLOG(OS_LOG_ERROR, "Array::GetVendor, ret %d.", iRet);
        return iRet;
    }

    // 去掉空格
    strVendor = CMpString::Trim(strVendor);
    if (strVendor.empty()) {
        COMMLOG(OS_LOG_ERROR, "Get vendor info of Device name(%s) failed.", strDevice.c_str());
        strLUNInfo = strDevice + ";;;;;";
        return MP_SUCCESS;
    }

    // product内容查询出来后并没有使用，如果为空只做提示，不做退出
    strProduct = CMpString::Trim(strProduct);
    if (!strProduct.empty()) {
        COMMLOG(OS_LOG_WARN, "Get product info of Device name(%s) failed.", strDevice.c_str());
    }

    // not huawei lun, do not get lun info continue
    if (Array::CheckHuaweiLUN(strVendor) == MP_FALSE) {
        COMMLOG(OS_LOG_WARN, "Device name(%s) is not in huawei/huasai array.", strDevice.c_str());
        strLUNInfo = strDevice + ";" + strVendor + ";" + strProduct + ";;;";
        return MP_SUCCESS;
    }

    // 阵列SN
    mp_string strSN;
    iRet = Array::GetDisk80Page(strDevice, strSN);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get arraysn info of Device name(%s) failed.", strDevice.c_str());
        strLUNInfo = strDevice + ";" + strVendor + ";" + strProduct + ";;;";
        return MP_SUCCESS;
    }

    // LUN WWN和LUN ID
    mp_string strLUNID;
    mp_string strLUNWWN;
    iRet = Array::GetDisk83Page(strDevice, strLUNWWN, strLUNID);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get 83 page of device failed.");
        return iRet;
    }

    iRet = GetLUNIdWhenC8(strDevice, strLUNID);
    if (iRet != MP_SUCCESS) {
        return iRet;
    }

    // 组装结果文件
    // devicename;vendor;product;arraysn;lunid;wwn
    strLUNInfo = strDevice + ";" + strVendor + ";" + strProduct + ";" + strSN + ";" + strLUNID + ";" + strLUNWWN;
    COMMLOG(OS_LOG_DEBUG, "Batch get this lun( %s ) info success.", strDevice.c_str());
    return MP_SUCCESS;
}

mp_int32 CSystemCall::GetLUNIdWhenC8(mp_string& strDevice, mp_string& strLUNID)
{
    vector<mp_string> vecPages;
    // if support C8,get lun id from C8
    mp_int32 iRet = Array::GetDisk00Page(strDevice, vecPages);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get 00 page of device failed.");
        return iRet;
    }

    // get lun id from
    vector<mp_string>::iterator itPg = find(vecPages.begin(), vecPages.end(), "c8");
    if (itPg != vecPages.end()) {
        iRet = Array::GetDiskC8Page(strDevice, strLUNID);
        if (iRet != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "Get C8 page of device failed.");
            return iRet;
        }
    }

    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Function Name: SyncDataFile
Description  : 同步数据文件缓存
Return       : MP_SUCCESS 成功
               非MP_SUCCESS失败
Others        :-------------------------------------------------------- */
mp_int32 CSystemCall::SyncDataFile(const mp_string& strUniqueID)
{
#ifdef LINUX
    mp_string strParam = "";

    // 构造临时输入文件路径
    mp_int32 iRet = GetParamFromTmpFile(strUniqueID, strParam);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Call GetParamFormTmpFile failed, ret %d.", iRet);
        return iRet;
    }
    mp_char pFilePath[PATH_MAX + 1] = {0};
    if (strParam.length() > PATH_MAX || NULL == realpath(strParam.c_str(), pFilePath)) {
        COMMLOG(OS_LOG_ERROR, "Check real path failed,errno[%d]:%s.", errno, strerror(errno));
        return MP_FAILED;
    }

    COMMLOG(OS_LOG_DEBUG, "Begin to sync data file.");

    mp_int32 fd = open(pFilePath, O_RDONLY | O_NONBLOCK);
    if (fd < MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Open data file failed, errno[%d]:%s.", errno, strerror(errno));
        return MP_FAILED;
    }

    // 下刷缓存至硬盘
    iRet = fsync(fd);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Sync data file failed, errno[%d]:%s.", errno, strerror(errno));
        close(fd);
        return MP_FAILED;
    }
    close(fd);

    COMMLOG(OS_LOG_DEBUG, "Sync data file successful.");
    return MP_SUCCESS;
#else
    COMMLOG(OS_LOG_INFO, "Sync data file not implemente.");
    return MP_FAILED;
#endif
}

/* ------------------------------------------------------
Function Name: ReloadUDEVRules
Description     : 重新加载udev规则
Input             : NA
Return           : MP_SUCCESS 成功
                   非MP_SUCCESS失败
-------------------------------------------------------- */
mp_int32 CSystemCall::ReloadUDEVRules(const mp_string& strUniqueID)
{
#ifdef LINUX
    mp_int32 iRet;
    mp_int32 iOSType = HOST_OS_UNKNOWN;
    mp_string strOSVersion;
    mp_string strExecCmd = "udevadm control --reload-rules";

    COMMLOG(OS_LOG_INFO, "Begin reload udev rules...");

    // 获取OS版本
    SecureCom::GetOSType(iOSType);
    iRet = SecureCom::GetOSVersion(iOSType, strOSVersion);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get OS version failed.");
        return iRet;
    }

    if ((iOSType == HOST_OS_REDHAT && strOSVersion == "5") || (iOSType == HOST_OS_SUSE && strOSVersion == "10")) {
        strExecCmd = "udevcontrol reload_rules";
    } else if (HOST_OS_OTHER_LINUX == iOSType) {
        // 当前其他的OS，如Rocky4\Rocky6\iSoft使用老版本命令
        strExecCmd = "udevcontrol reload_rules";
    }
    COMMLOG(OS_LOG_DEBUG, "cmd '%s' will be excute.", strExecCmd.c_str());
    iRet = CSystemExec::ExecSystemWithoutEcho(strExecCmd);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Excute reload udev rules failed, iRet %d.", iRet);
        return iRet;
    }

    COMMLOG(OS_LOG_INFO, "Reload udev rules succ.");
#endif
    return MP_SUCCESS;
}

mp_int32 CSystemCall::CheckDirExist(mp_string& strUniqueID)
{
#ifdef LINUX
    mp_int32 iRet;
    mp_string strParam = "";

    COMMLOG(OS_LOG_INFO, "Begin check dir exist...");

    // 构造临时输入文件路径
    iRet = GetParamFromTmpFile(strUniqueID, strParam);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Call GetParamFormTmpFile failed, ret %d.", iRet);
        return iRet;
    }

    if (!CMpFile::DirExist(strParam.c_str())) {
        COMMLOG(OS_LOG_INFO, "Mount point dose not exist, mount point %s.", strParam.c_str());
        return ERROR_DEVICE_FILESYS_MOUNT_POINT_NOT_EXIST;
    }

    COMMLOG(OS_LOG_INFO, "Check dir exist succ.");
#endif
    return MP_SUCCESS;
}

mp_int32 CSystemCall::SignScripts()
{
    mp_int32 iRet;

    COMMLOG(OS_LOG_INFO, "Begin sign scripts with root permission.");

    iRet = SecureCom::GenSignFile();
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Sign scripts with root permission failed.");
        return iRet;
    }

    COMMLOG(OS_LOG_INFO, "Sign scripts with root permission succ.");
    return MP_SUCCESS;
}

#ifdef LIN_FRE_SUPP

/* ------------------------------------------------------
Function Name: FreezeFileSys
Description     : 冻结文件系统的操作
Input             : strDriveLetter冻结，解冻所需的挂载点
Return           : MP_SUCCESS 成功
                    非MP_SUCCESS失败
Create By      : tanyuanjun 00285255
-------------------------------------------------------- */
mp_int32 CSystemCall::FreezeFileSys(const mp_string& strUniqueID)
{
    mp_int32 fd = MP_INVALID_HANDLE;
    mp_int32 iRet = MP_SUCCESS;
    mp_int32 iErr = 0;
    mp_char szErr[systemCallNum256] = {0};
    mp_long lTimeOut = OPER_TIME_OUT;
    mp_string strLogFalg;
    mp_string strDriveLetter;

    COMMLOG(OS_LOG_INFO, "Begian freeze file system %s...", strDriveLetter.c_str());
    // CodeDex误报，FORTIFY.Path_Manipulation
    // 构造临时输入文件路径
    mp_string strFilePath = CPath::GetInstance().GetTmpFilePath(mp_string(INPUT_TMP_FILE) + strUniqueID);
    if (CMpFile::FileExist(strFilePath)) {
        iRet = CIPCFile::ReadInput(strUniqueID, strDriveLetter);
        if (iRet != MP_SUCCESS) {
            // 记录日志
            COMMLOG(OS_LOG_ERROR, "ReadInput failed, ret %d.", iRet);
            return iRet;
        }
    } else {
        COMMLOG(OS_LOG_ERROR, "tmp file \"%s\" is not exist.", strFilePath.c_str());
        return ERROR_COMMON_OPER_FAILED;
    }

    fd = open(strDriveLetter.c_str(), O_RDONLY);
    if (fd < MP_SUCCESS) {
        iErr = GetOSError();
        COMMLOG(OS_LOG_ERROR,
            "Open the mounted point %s failed, errno[%d]: %s.",
            strDriveLetter.c_str(),
            iErr,
            GetOSStrErr(iErr, szErr, sizeof(szErr)));

        return ERROR_COMMON_OPER_FAILED;
    }

    iRet = ioctl(fd, FIFREEZE, &lTimeOut);
    close(fd);

    if (iRet < MP_SUCCESS) {
        iErr = GetOSError();
        COMMLOG(OS_LOG_ERROR,
            "file system %s freeze failed, errno[%d]: %s.",
            strDriveLetter.c_str(),
            iErr,
            GetOSStrErr(iErr, szErr, sizeof(szErr)));

        return ERROR_COMMON_APP_FREEZE_FAILED;
    }

    COMMLOG(OS_LOG_INFO, "File system %s freeze succ.", strDriveLetter.c_str());

    return MP_SUCCESS;
}

/* ------------------------------------------------------
Function Name: FreezeFileSys
Description     : 解冻文件系统的操作
Input             : strDriveLetter冻结，解冻所需的挂载点
Return           : MP_SUCCESS 成功
                    非MP_SUCCESS失败
Create By      : tanyuanjun 00285255
-------------------------------------------------------- */
mp_int32 CSystemCall::ThawFileSys(const mp_string& strUniqueID)
{
    mp_int32 iRet;
    mp_long lTimeOut = OPER_TIME_OUT;
    mp_char szErr[systemCallNum256] = {0};
    mp_string strDriveLetter;

    COMMLOG(OS_LOG_INFO, "Begin thaw file system %s...", strDriveLetter.c_str());
    // CodeDex误报，FORTIFY.Path_Manipulation
    // 构造临时输入文件路径
    mp_string strFile = CPath::GetInstance().GetTmpFilePath(mp_string(INPUT_TMP_FILE) + strUniqueID);
    if (CMpFile::FileExist(strFile)) {
        iRet = CIPCFile::ReadInput(strUniqueID, strDriveLetter);
        if (iRet != MP_SUCCESS) {
            // 记录日志
            COMMLOG(OS_LOG_ERROR, "ThawFileSys: ReadInput failed, ret %d.", iRet);
            return iRet;
        }
    } else {
        COMMLOG(OS_LOG_ERROR, "ThawFileSys: tmp file \"%s\" is not exist.", strFile.c_str());
        return ERROR_COMMON_OPER_FAILED;
    }

    mp_int32 fd = open(strDriveLetter.c_str(), O_RDONLY);
    if (fd < MP_SUCCESS) {
        mp_int32 iErr = GetOSError();
        COMMLOG(OS_LOG_ERROR,
            "Open the mounted point %s failed, errno[%d]: %s.",
            strDriveLetter.c_str(),
            iErr,
            GetOSStrErr(iErr, szErr, sizeof(szErr)));
        return ERROR_COMMON_OPER_FAILED;
    }

    iRet = ioctl(fd, FITHAW, &lTimeOut);
    close(fd);

    if (iRet < MP_SUCCESS) {
        // Rocky二次解冻返回成功、SuSE11\RedHat6二次解冻返回错误码22(Invalid argument)
        // 此种情况统一返回MP_SUCCESS
        // 对文件系统保护出现异常时，多次解冻不报错误
        mp_int32 iErr = GetOSError();
        if (iErr != THAW_ERR) {
            COMMLOG(OS_LOG_ERROR,
                "file system %s thaw failed, errno[%d]: %s.",
                strDriveLetter.c_str(),
                iErr,
                GetOSStrErr(iErr, szErr, sizeof(szErr)));

            return ERROR_COMMON_APP_THAW_FAILED;
        }

        COMMLOG(OS_LOG_INFO, "File system %s is thaw, so return 0.", strDriveLetter.c_str());
    }

    COMMLOG(OS_LOG_INFO, "File system %s thaw succ.", strDriveLetter.c_str());

    return MP_SUCCESS;
}

#endif  // LINUX2.6.29 up

/* ------------------------------------------------------------
Function Name: CCommandMap
Description  : CCommandMap构造函数
               生成命令字和脚本名称的对应关系
               生成命令字和系统命令的对应关系
Others       :-------------------------------------------------------- */
CCommandMap::CCommandMap()
{
    InitDB2ScriptMap();
    InitOracleScriptMap();
    InitOracleBkScriptMap();
    InitHostScriptMap();
    InitSysCmdMap1();
    InitSysCmdMap2();
    InitSysCmdMap4();
    InitSysCmdMap5();
    InitNeedEchoCmdMap1();
    InitNeedEchoCmdMap2();
    InitUpgradeCmdMap();
    InitModifyCmdMap();
    InitNoParamMap();
    InitWhiteFunMap();
    InitSanclientMap();
}

/* ------------------------------------------------------------
Function Name: InitDB2ScriptMap
Description  : 初始化脚本命令map
Others       :-------------------------------------------------------- */
mp_void CCommandMap::InitDB2ScriptMap()
{
    // 初始化脚本名字,需加脚本后缀
    (mp_void) m_mapCommand.emplace(map<mp_int32, mp_string>::value_type(ROOT_COMMAND_SCRIPT_INIT, "initiator.sh"));
    (mp_void) m_mapCommand.emplace(map<mp_int32, mp_string>::value_type(ROOT_COMMAND_SCRIPT_PACKAGELOG, "packlog.sh"));
}

/* ------------------------------------------------------------
Function Name: InitOracleScriptMap
Description  : 初始化脚本命令map
Others       :-------------------------------------------------------- */
mp_void CCommandMap::InitOracleScriptMap()
{
    (mp_void) m_mapCommand.emplace (
        map<mp_int32, mp_string>::value_type(ROOT_COMMAND_SCRIPT_MOUNT_NAS_FILESYS, "mountnasfilesystem.sh"));
    (mp_void) m_mapCommand.emplace (
        map<mp_int32, mp_string>::value_type(ROOT_COMMAND_SCRIPT_MOUNT_SANCLIENT_FILESYS, "mountfileiosystem.sh"));
    (mp_void) m_mapCommand.emplace (
        map<mp_int32, mp_string>::value_type(ROOT_COMMAND_SCRIPT_UMOUNT_NAS_FILESYS, "umountnasfilesystem.sh"));
    (mp_void) m_mapCommand.emplace (
        map<mp_int32, mp_string>::value_type(ROOT_COMMAND_SCRIPT_CLEAR_MOUNT_POINT, "clearmountpoint.sh"));
    (mp_void) m_mapCommand.emplace (
        map<mp_int32, mp_string>::value_type(ROOT_COMMAND_SCRIPT_SET_CGROUP, "setcgroup.sh"));
    (mp_void) m_mapCommand.emplace (
        map<mp_int32, mp_string>::value_type(ROOT_COMMAND_SCRIPT_VMFS_CHECK_TOOL, "vmfs_check_tool.sh"));
    (mp_void) m_mapCommand.emplace (
        map<mp_int32, mp_string>::value_type(ROOT_COMMAND_SCRIPT_VMFS_MOUNT, "vmfs_mount.sh"));
    (mp_void) m_mapCommand.emplace (
        map<mp_int32, mp_string>::value_type(ROOT_COMMAND_SCRIPT_VMFS_UMOUNT, "vmfs_umount.sh"));
}

mp_void CCommandMap::InitOracleBkScriptMap()
{
    (mp_void) m_mapCommand.emplace (
        map<mp_int32, mp_string>::value_type(ROOT_COMMAND_SCRIPT_PREPARE_NASMEDIA, "preparenasmedia.sh"));
    (mp_void) m_mapCommand.emplace (
        map<mp_int32, mp_string>::value_type(ROOT_COMMAND_SCRIPT_PREPARE_DATATURBOMEDIA, "preparedataturbomedia.sh"));
    (mp_void) m_mapCommand.emplace (
        map<mp_int32, mp_string>::value_type(ROOT_COMMAND_SCRIPT_UMOUNT_NASMEDIA, "umountnasmedia.sh"));
    (mp_void) m_mapCommand.emplace (
        map<mp_int32, mp_string>::value_type(ROOT_COMMAND_SCRIPT_MOUNT_DATATURBO_FILESYS,
        "mountdataturbofilesystem.sh"));
    (mp_void) m_mapCommand.emplace (
        map<mp_int32, mp_string>::value_type(ROOT_COMMAND_CHECK_AND_CREATE_DATATURBO_LINK, "CreateDataturbolink"));
}

/* ------------------------------------------------------------
Function Name: InitHostScriptMap
Description  : 初始化脚本命令map
Others       :-------------------------------------------------------- */
mp_void CCommandMap::InitHostScriptMap()
{
    // 初始化脚本名字,需加脚本后缀
    (mp_void) m_mapCommand.emplace (map<mp_int32, mp_string>::value_type(ROOT_COMMAND_SCRIPT_SCANDISK, "scandisk.sh"));
    // add by zhuyuanjie, shutdown should not use popen way to execute, use script (2019/11/29)
    (mp_void) m_mapCommand.emplace (
        map<mp_int32, mp_string>::value_type(ROOT_COMMAND_SCRIPT_LINK_ISCISITARGET, "linkiscsitarget.sh"));
    (mp_void) m_mapCommand.emplace (
        map<mp_int32, mp_string>::value_type(ROOT_COMMAND_SCRIPT_GETHOSTOS, "gethostos.sh"));
}

/* ------------------------------------------------------------
Function Name: InitSysCmdMap1
Description  : 初始化系统命令map，因扇出太高，拆成多个函数，每个函数只注册10个系统命令
Others       :-------------------------------------------------------- */
mp_void CCommandMap::InitSysCmdMap1()
{
    (mp_void) m_mapCommand.insert(map<mp_int32, mp_string>::value_type(ROOT_COMMAND_FSCK, "fsck"));
    (mp_void) m_mapCommand.insert(map<mp_int32, mp_string>::value_type(ROOT_COMMAND_FSTYP, "fstyp"));
    (mp_void) m_mapCommand.insert(map<mp_int32, mp_string>::value_type(ROOT_COMMAND_CAT, "cat"));
    (mp_void) m_mapCommand.insert(map<mp_int32, mp_string>::value_type(ROOT_COMMAND_LS, "ls -l"));
    (mp_void) m_mapCommand.insert(map<mp_int32, mp_string>::value_type(ROOT_COMMAND_CHMOD_OTHER_READ, "chmod o+rx "));
}

/* ------------------------------------------------------------
Function Name: InitSysCmdMap2
Description  : 初始化系统命令map，因扇出太高，拆成多个函数，每个函数只注册10个系统命令
Others       :-------------------------------------------------------- */
mp_void CCommandMap::InitSysCmdMap2()
{
    (mp_void) m_mapCommand.insert(map<mp_int32, mp_string>::value_type(ROOT_COMMAND_MOUNT, "mount"));
    (mp_void) m_mapCommand.insert(map<mp_int32, mp_string>::value_type(ROOT_COMMAND_GETCONF, "getconf"));
    // add by wangguitao 2017-11-29, if mount is echo, exec failed, do not return failed.
    // add new command ROOT_COMMAND_MOUNT_NOECHO, and is not echo, exec failed, and return failed.
    (mp_void) m_mapCommand.emplace (map<mp_int32, mp_string>::value_type(ROOT_COMMAND_MOUNT_NOECHO, "mount"));
    (mp_void) m_mapCommand.emplace (map<mp_int32, mp_string>::value_type(ROOT_COMMAND_KILL, "kill -9"));
    (mp_void) m_mapCommand.emplace (map<mp_int32, mp_string>::value_type(ROOT_COMMAND_DU, "du -sk"));
}

/* ------------------------------------------------------------
Function Name: InitSysCmdMap4
Description  : 初始化系统命令map，因扇出太高，拆成多个函数，每个函数只注册20个系统命令
Others       :-------------------------------------------------------- */
mp_void CCommandMap::InitSysCmdMap4()
{
    (mp_void) m_mapCommand.emplace (map<mp_int32, mp_string>::value_type(ROOT_COMMAND_IOSCANFNC, "ioscan -fnC disk"));
}

/* ------------------------------------------------------------
Function Name: InitSysCmdMap5
Description  : 初始化系统命令map，因扇出太高，拆成多个函数，每个函数只注册20个系统命令
Others       :-------------------------------------------------------- */
mp_void CCommandMap::InitSysCmdMap5()
{
    (mp_void) m_mapCommand.emplace (map<mp_int32, mp_string>::value_type(ROOT_COMMAND_CFGMGR, "cfgmgr -v"));
    (mp_void) m_mapCommand.emplace (map<mp_int32, mp_string>::value_type(ROOT_COMMAND_CFGADM, "cfgadm -al"));
    (mp_void) m_mapCommand.emplace (map<mp_int32, mp_string>::value_type(ROOT_COMMAND_DEVFSADM, "devfsadm"));
    (mp_void) m_mapCommand.emplace (map<mp_int32, mp_string>::value_type(ROOT_COMMAND_RM, "rm"));
}

/* ------------------------------------------------------------
Function Name: InitNeedEchoCmdMap1
Description  : 初始化需要回显命令map
Others       :-------------------------------------------------------- */
mp_void CCommandMap::InitNeedEchoCmdMap1()
{
    // 初始化命令字对应的是否需要回显值
    (mp_void) m_mapNeedEcho.emplace (map<mp_int32, mp_bool>::value_type(ROOT_COMMAND_CAT, MP_TRUE));
    (mp_void) m_mapNeedEcho.emplace (map<mp_int32, mp_bool>::value_type(ROOT_COMMAND_LS, MP_TRUE));
}

/* ------------------------------------------------------------
Function Name: InitNeedEchoCmdMap2
Description  : 初始化需要回显命令map
Others       :-------------------------------------------------------- */
mp_void CCommandMap::InitNeedEchoCmdMap2()
{
    (mp_void) m_mapNeedEcho.insert(map<mp_int32, mp_bool>::value_type(ROOT_COMMAND_FSTYP, MP_TRUE));
    (mp_void) m_mapNeedEcho.insert(map<mp_int32, mp_bool>::value_type(ROOT_COMMAND_MOUNT, MP_TRUE));
    (mp_void) m_mapNeedEcho.insert(map<mp_int32, mp_bool>::value_type(ROOT_COMMAND_GETCONF, MP_TRUE));
    (mp_void) m_mapNeedEcho.insert(map<mp_int32, mp_bool>::value_type(ROOT_COMMAND_DU, MP_TRUE));
}

/* ------------------------------------------------------------
Function Name: InitNoParamMap
Description  : 初始化无参数命令
Others       :-------------------------------------------------------- */
mp_void CCommandMap::InitNoParamMap()
{
    (mp_void) m_mapNoParam.emplace(map<mp_int32, mp_bool>::value_type(ROOT_COMMAND_IOSCANFNC, MP_TRUE));
    (mp_void) m_mapNoParam.emplace(map<mp_int32, mp_bool>::value_type(ROOT_COMMAND_CFGMGR, MP_TRUE));
    (mp_void) m_mapNoParam.emplace(map<mp_int32, mp_bool>::value_type(ROOT_COMMAND_CFGADM, MP_TRUE));
    (mp_void) m_mapNoParam.emplace(map<mp_int32, mp_bool>::value_type(ROOT_COMMAND_DEVFSADM, MP_TRUE));
}


/* ------------------------------------------------------------
Function Name: InitUpgradeCmdMap
Description  : 初始化推送升级需要的脚本
Others       :-------------------------------------------------------- */
mp_void CCommandMap::InitUpgradeCmdMap()
{
    (mp_void) m_mapCommand.emplace (
        map<mp_int32, mp_string>::value_type(ROOT_COMMAND_SCRIPT_UPGRADECALLER, "upgrade_caller.sh"));
    (mp_void) m_mapCommand.emplace (
        map<mp_int32, mp_string>::value_type(ROOT_COMMAND_SCRIPT_PUSHUPDATECERT, "push_update_cert.sh"));
    (mp_void) m_mapCommand.emplace (
        map<mp_int32, mp_string>::value_type(ROOT_COMMAND_SCRIPT_CHECKBEFOREUPGRADE, "upgrade_check.sh"));
    (mp_void) m_mapCommand.emplace (
        map<mp_int32, mp_string>::value_type(ROOT_COMMAND_SCRIPT_PREPAREFORUPGRADE, "upgrade_prepare.sh"));
}

/* ------------------------------------------------------------
Function Name: InitUpgradeCmdMap
Description  : 初始化推送修改需要的脚本
Others       :-------------------------------------------------------- */
mp_void CCommandMap::InitModifyCmdMap()
{
    (mp_void) m_mapCommand.emplace (
        map<mp_int32, mp_string>::value_type(ROOT_COMMAND_SCRIPT_MODIFYCALLER, "modify_caller.sh"));
    (mp_void) m_mapCommand.emplace (
        map<mp_int32, mp_string>::value_type(ROOT_COMMAND_SCRIPT_CHECKBEFOREMODIFY, "modify_check.sh"));
    (mp_void) m_mapCommand.emplace (
        map<mp_int32, mp_string>::value_type(ROOT_COMMAND_SCRIPT_PREPAREFORMODIFY, "modify_prepare.sh"));
}

/* ------------------------------------------------------------
Function Name: GetCommandString
Description  : 根据命令字获取对应的脚本名称或系统命令字符串
Others       :-------------------------------------------------------- */
mp_string CCommandMap::GetCommandString(mp_int32 iCommandID)
{
    map<mp_int32, mp_string>::const_iterator it = m_mapCommand.find(iCommandID);
    if (it != m_mapCommand.end()) {
        return it->second;
    } else {
        return "unknown";
    }
}

/* ------------------------------------------------------------
Function Name: NeedEcho
Description  : 根据命令字获取该命令是否需要获取回显信息
                MP_TRUE:需要回显
                MP_FALSE:不需要回显
Others       :-------------------------------------------------------- */
mp_bool CCommandMap::NeedEcho(mp_int32 iCommandID)
{
    map<mp_int32, mp_bool>::const_iterator it = m_mapNeedEcho.find(iCommandID);
    if (it != m_mapNeedEcho.end()) {
        return MP_TRUE;
    } else {
        return MP_FALSE;
    }
}

/* ------------------------------------------------------------
Function Name: NoParam
Description  : 根据命令字获取该命令是否为无参数命令
                MP_TRUE:无参数
                MP_FALSE:有参数
Others       :-------------------------------------------------------- */
mp_bool CCommandMap::NoParam(mp_int32 iCommandID)
{
    map<mp_int32, mp_bool>::const_iterator it = m_mapNoParam.find(iCommandID);
    if (it != m_mapNoParam.end()) {
        return MP_TRUE;
    } else {
        return MP_FALSE;
    }
}

mp_bool CCommandMap::WhiteListVerify(int iCommandID, const std::vector<mp_string>& vecParam)
{
    INFOLOG("Start verify cmd: %d", iCommandID);
    std::map<int, WhiteVerifyFun>::iterator iter = m_mapWhiteFun.find(iCommandID);
    mp_bool bRet = MP_TRUE;
    if (iter != m_mapWhiteFun.end()) {
        bRet = iter->second(vecParam);
    } else {
        INFOLOG("Can't find verify fun");
    }
    return bRet;
}

mp_void CCommandMap::InitWhiteFunMap()
{
    m_mapWhiteFun.insert(std::make_pair(ROOT_COMMAND_SCRIPT_UPGRADECALLER,
        std::bind(&CCommandMap::UpgradeCallVerify, this, std::placeholders::_1)));
    m_mapWhiteFun.insert(std::make_pair(ROOT_COMMAND_SCRIPT_PUSHUPDATECERT,
        std::bind(&CCommandMap::UpgradeCallVerify, this, std::placeholders::_1)));
}

mp_bool CCommandMap::UpgradeCallVerify(const std::vector<mp_string>& vecParam)
{
    mp_string ppid;
    mp_bool bRet = CSystemCall::GetParentPid(ppid);
    if (bRet != MP_TRUE) {
        ERRLOG("Failed to obtain the parent process ID.");
        return MP_FALSE;
    }
    INFOLOG("The process ppid: %s", ppid.c_str());

    mp_string processInfo;
    bRet = CSystemCall::GetProcessInfo(ppid, processInfo);
    if (bRet != MP_TRUE) {
        ERRLOG("Failed to obtain process information.");
        return MP_FALSE;
    }

    INFOLOG("The process path[%s].", processInfo.c_str());
    mp_string rootPath = CPath::GetInstance().GetRootPath();
    mp_string rdagentPath = rootPath + DOUBLE_SLASH + AGENT_BIN_DIR + PATH_SEPARATOR + "rdagent";
    if (processInfo == rdagentPath) {
        INFOLOG("Verifying the dataprocess process succeeded.");
    } else {
        ERRLOG("Failed to verify the C++ process.");
        return MP_FALSE;
    }
    return MP_TRUE;
}

mp_bool CheckIsRdagent(const mp_string& pidStr, mp_string& ppid, mp_string& processInfo)
{
    INFOLOG("Begin to check rdagent process");
    mp_string rootPath = CPath::GetInstance().GetRootPath();
    mp_string rdagentPath = rootPath + DOUBLE_SLASH + AGENT_BIN_DIR + PATH_SEPARATOR + "rdagent";
    mp_bool bRet =  CSystemCall::GetProcessInfo(pidStr, processInfo);
    if (processInfo == rdagentPath) {
        ppid = pidStr;
        return MP_TRUE;
    }
    return  CSystemCall::GetParentPidImpl(pidStr, ppid);
}

mp_bool CSystemCall::GetParentPid(mp_string& ppid)
{
    mp_uint32 pid = (mp_uint32)getppid();
    mp_string pidStr = CMpString::to_string(pid);

    mp_string processInfo;
    return CheckIsRdagent(pidStr, ppid, processInfo);
}

mp_bool CSystemCall::GetParentPidImpl(const mp_string& pid, mp_string& ppid)
{
    ppid = "";
    std::ostringstream oss;
    oss << "ps -aef |grep " << pid;
    mp_string strCmd = oss.str();
    FILE* pStream = popen(strCmd.c_str(), "r");
    if (pStream == nullptr) {
        ERRLOG("Exec popen failed.");
        return MP_FALSE;
    }

    std::vector<mp_string> vecResult;
    while (!feof(pStream)) {
        char tmpBuf[1600] = {0};
        fgets(tmpBuf, sizeof(tmpBuf), pStream);
        if (strlen(tmpBuf) > 0) {
            tmpBuf[strlen(tmpBuf) - 1] = 0;  // 去掉获取出来的字符串末尾的'\n'
        }
        vecResult.push_back(tmpBuf);
    }
    pclose(pStream);

    if (vecResult.empty()) {
        ERRLOG("Failed to find the process.");
        return MP_FALSE;
    }

    vecResult = GrepV(vecResult, "grep");
    std::vector<mp_string> vecPid = Awk(vecResult, AWK_COL_FIRST_2);
    std::vector<mp_string> vecPpid = Awk(vecResult, AWK_COL_FIRST_3);
    std::vector<mp_string> vecInfo = Awk(vecResult, AWK_COL_LAST_1);
    size_t process_size = vecPid.size();
    for (size_t index = 0; index < process_size; ++index) {
        if (vecPid[index] == pid) {
            ppid = vecPpid[index];
            break;
        }
    }

    if (ppid.empty()) {
        return MP_FALSE;
    }
    return MP_TRUE;
}

mp_bool CSystemCall::GetProcessInfo(const mp_string& pid, mp_string& result)
{
    INFOLOG("The process pid: %s", pid.c_str());
    std::ostringstream oss;
    oss << "ps -p " << pid << " -o pid,args | grep " << pid;
    mp_string strCmd = oss.str();
    FILE* pStream = popen(strCmd.c_str(), "r");
    if (pStream == nullptr) {
        ERRLOG("Exec popen failed.");
        return MP_FALSE;
    }

    std::vector<mp_string> vecResult;
    while (!feof(pStream)) {
        char tmpBuf[MAX_READ_NUM] = {0};
        fgets(tmpBuf, sizeof(tmpBuf), pStream);
        if (strlen(tmpBuf) > 0) {
            tmpBuf[strlen(tmpBuf) - 1] = 0;  // 去掉获取出来的字符串末尾的'\n'
        }
        vecResult.push_back(tmpBuf);
    }
    pclose(pStream);
    if (vecResult.empty()) {
        ERRLOG("Failed to find the process.");
        return MP_FALSE;
    }
    INFOLOG("The process: %s information is %s", pid.c_str(), vecResult[0].c_str());
    vecResult = GrepV(vecResult, "grep");
    std::vector<mp_string> vecPid = Awk(vecResult, AWK_COL_FIRST_1);
    std::vector<mp_string> vecProcess = Awk(vecResult, AWK_COL_FIRST_2);

    size_t process_size = vecPid.size();
    for (size_t index = 0; index < process_size; ++index) {
        if (vecPid[index] == pid) {
            result = vecProcess[index];
            break;
        }
    }

    return result.empty() ? MP_FALSE : MP_TRUE;
}

/* ------------------------------------------------------------
Function Name: GetParamFormTmpFile
Description  : 根据id获取临时文件内容
Others       :-------------------------------------------------------- */
mp_int32 CSystemCall::GetParamFromTmpFile(const mp_string& strUniqueID, mp_string& strParam)
{
    // 构造临时输入文件路径
#ifdef WIN32
    mp_string strFilePath = CPath::GetInstance().GetTmpFilePath(mp_string(INPUT_TMP_FILE) + strUniqueID);
    if (CMpFile::FileExist(strFilePath)) {
        mp_int32 iRet = CIPCFile::ReadInput(strUniqueID, strParam);
        if (iRet != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "ReadInput failed, ret %d.", iRet);
            return iRet;
        }
    }
#else
    CMpPipe pipe;
    mp_int32 iRet = pipe.ReadInput(strUniqueID, strParam);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "ReadInput failed, ret %d.", iRet);
        return iRet;
    }
#endif // WIN32
    // 临时文件不存在按参数为空处理
    return MP_SUCCESS;
}

mp_int32 CSystemCall::StartDataProcess(const mp_string& strUniqueID, const std::vector<mp_string>& vecParam)
{
    // vmware data process used
    static const mp_int32 VMWARE_DATAPROCESS_PARAM_NUM = 2;
    COMMLOG(OS_LOG_DEBUG, "Begin to start path process.");

    mp_string strParam = vecParam.empty() ? "" : vecParam.front();
    COMMLOG(OS_LOG_INFO, "The parameter passed is: '%s'.", strParam.c_str());

    vector<mp_string> vecCmds;
    CMpString::StrSplit(vecCmds, strParam, ';');
    if (VMWARE_DATAPROCESS_PARAM_NUM != vecCmds.size()) {
        COMMLOG(
            OS_LOG_ERROR, "Both service type and vddk version must be provided, please check the parameter passed!");
        return MP_FAILED;
    }
    for (vector<mp_string>::iterator iter = vecCmds.begin(); iter != vecCmds.end(); ++iter) {
        CHECK_FAIL_EX(CheckCmdDelimiter(*iter));
    }
    CHECK_FAIL_EX(ChecVddkVersion(vecCmds[1]));
    mp_string strDpName = CPath::GetInstance().GetSBinPath() + PATH_SEPARATOR + OM_DPP_EXEC_NAME + " " + vecCmds[0] +
                          " " + vecCmds[1];
    mp_string strVddkPath = "";
    strVddkPath = "LD_LIBRARY_PATH=" + CPath::GetInstance().GetLibPath() + PATH_SEPARATOR + "vddk" + PATH_SEPARATOR +
                  vecCmds[1] + PATH_SEPARATOR + "vmware-vix-disklib-distrib/lib64/" + ":" +
                  CPath::GetInstance().GetBinPath();

    strDpName = strVddkPath + " nohup " + strDpName + " 1>>" + CPath::GetInstance().GetSlogPath() + PATH_SEPARATOR +
                ROOT_EXEC_LOG_NAME + " 2>&1 &";
    FILE* pStream = popen(strDpName.c_str(), "r");
    if (NULL == pStream) {
        mp_int32 iErr = GetOSError();
        COMMLOG(OS_LOG_ERROR, "Run service '%s' failed, ret: '%d'.", OM_DPP_EXEC_NAME.c_str(), iErr);
        return ERROR_COMMON_SYSTEM_CALL_FAILED;
    }
    pclose(pStream);
    return MP_SUCCESS;
}

mp_bool CSystemCall::WhiteListVerify(mp_int32 iCommandID, const mp_string& strParam)
{
    mp_bool bRet = MP_TRUE;
    switch (ROOT_COMMAND(iCommandID)) {
        case ROOT_COMMAND_KILL: {
            bRet = KillWhiteListVerify(strParam);
            break;
        }
        case ROOT_COMMAND_CAT: {
            bRet = CatWhiteListVerify(strParam);
            break;
        }
        case ROOT_COMMAND_MOUNT: {
            bRet = MountWhiteListVerify(strParam);
            break;
        }
        case ROOT_COMMAND_RM: {
            bRet = RmWhiteListVerify(strParam);
            break;
        }
        case ROOT_COMMAND_CHMOD_OTHER_READ: {
            bRet = ChmodOtherReadWhiteListVerify(strParam);
            break;
        }
    }
    return bRet;
}

mp_bool CSystemCall::KillWhiteListVerify(const mp_string& strParam)
{
    mp_string processInfo;
    if (GetProcessInfo(strParam, processInfo)) {
        if (!VerifyProcess(processInfo)) {
            ERRLOG("Failed to verify the process information.");
            return MP_FALSE;
        }
    }
    INFOLOG("The kill command parameters are successfully verified.");
    return MP_TRUE;
}

mp_bool CSystemCall::CatWhiteListVerify(const mp_string& strParam)
{
    mp_string filePath = strParam;
    if (CMpString::FormattingPath(filePath) != MP_TRUE) {
        ERRLOG("Format path (%s) error.", strParam.c_str());
        return MP_FALSE;
    }
    if (filePath.compare("/etc/hosts") == 0) {
        INFOLOG("Verifying the filepath succeed");
    } else if (filePath.find("/mnt/databackup/") == 0) {
        INFOLOG("Verifying the filepath succeed");
    } else {
        ERRLOG("Failed to verify the path");
        return MP_FALSE;
    }
    return MP_TRUE;
}

mp_bool CSystemCall::MountWhiteListVerify(const mp_string& strParam)
{
    if (strParam.empty()) {
        INFOLOG("Verifying the param succeed");
        return MP_TRUE;
    }
    ERRLOG("Verifying the param failed");
    return MP_FALSE;
}

mp_bool CSystemCall::VerifyProcess(const mp_string& processInfo)
{
    mp_string pluginPath = CPath::GetInstance().GetRootPath() + "/..";
    if (!CMpString::FormattingPath(pluginPath)) {
        ERRLOG("Format path (%s) error.", pluginPath.c_str());
        return MP_FAILED;
    }

    INFOLOG("The process path[%s].", processInfo.c_str());
    mp_string dataprocesspath = CPath::GetInstance().GetSBinPath() + PATH_SEPARATOR + "dataprocess";
    mp_string naspluginpath = pluginPath + "/NasPlugin/bin/AppPlugin_NAS";
    mp_string jarStr = pluginPath + "/HadoopPlugin/lib/databackup.agent.jar";
    if (processInfo == dataprocesspath) {
        INFOLOG("Verifying the dataprocess process succeeded.");
    } else if (processInfo == naspluginpath) {
        INFOLOG("Verifying the NAS process succeeded.");
    } else if (processInfo == naspluginpath) {
        INFOLOG("Verifying the java process succeeded.");
    } else {
        ERRLOG("Failed to verify the C++ process.");
        return MP_FALSE;
    }
    return MP_TRUE;
}

mp_bool CSystemCall::RmWhiteListVerify(const mp_string &filePath)
{
    mp_string tmpFilePath = filePath;
    if (CMpString::FormattingPath(tmpFilePath) != MP_TRUE) {
        ERRLOG("Format path (%s) error.", filePath.c_str());
        return MP_FALSE;
    }
    std::vector<mp_string> rmWhitePath;
    rmWhitePath.emplace_back(CPath::GetInstance().GetRootPath() + PATH_SEPARATOR + AGENT_STMP_DIR);
    rmWhitePath.emplace_back(CPath::GetInstance().GetRootPath() + DOUBLE_SLASH + AGENT_STMP_DIR);

    for (auto const &it : rmWhitePath) {
        if (CheckPathString(tmpFilePath, it) == MP_SUCCESS) {
            return MP_TRUE;
        }
    }
    ERRLOG("File path (%s) is not in remove white list.", filePath.c_str());
    return MP_FALSE;
}

mp_bool CSystemCall::ChmodOtherReadWhiteListVerify(const mp_string &filePath)
{
    return MP_FALSE;
}

mp_int32 CSystemCall::CheckUserDefineScriptLegality(const mp_string& scriptCmd)
{
    mp_string tmpFilePath = scriptCmd.substr(scriptCmd.find(PATH_SEPARATOR), scriptCmd.find(" "));
    if (!CMpString::FormattingPath(tmpFilePath)) {
        ERRLOG("Format path (%s) error.", scriptCmd.c_str());
        return MP_FAILED;
    }

    mp_string customPath = CPath::GetInstance().GetRootPath() + "/../../..";
    if (!CMpString::FormattingPath(customPath)) {
        ERRLOG("Format path (%s) error.", customPath.c_str());
        return MP_FAILED;
    }

    mp_string pluginPath = customPath + "/DataBackup/ProtectClient/Plugins";
    std::array<mp_string, 1> UserDefineScriptBlackList = {
        "^" + pluginPath + "/HadoopPlugin/"
    };
    for (auto const &it : UserDefineScriptBlackList) {
        std::regex reg(it);
        std::smatch match;
        std::regex_search(tmpFilePath, match, reg);
        if (!match.empty()) {
            ERRLOG("User define script is in black list, scriptCmd %s.", scriptCmd.c_str());
            return MP_FAILED;
        }
    }
    const mp_int16 strCount = 2;
    std::array<mp_string, strCount> UserDefineScriptWhiteList = {
        mp_string("^") + pluginPath + "/\\w+/\\w+\\.sh",
        "^/opt/oceanstor/dataturbo/script/dataturbo_rescan$"
    };
    for (auto const &it : UserDefineScriptWhiteList) {
        std::regex reg(it);
        std::smatch match;
        std::regex_search(tmpFilePath, match, reg);
        if (!match.empty()) {
            return MP_SUCCESS;
        }
    }
    ERRLOG("User define script is not in white list, scriptCmd %s.", scriptCmd.c_str());
    return MP_FAILED;
}

mp_int32 CSystemCall::ChecVddkVersion(const mp_string& version)
{
    mp_string vddkVerReg = "^\\d*\\.?\\d+$";
    std::regex reg(vddkVerReg);
    std::smatch match;
    std::regex_search(version, match, reg);
    if (match.empty()) {
        ERRLOG("The vddk version is invalid., version %s.", version.c_str());
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

bool CSystemCall::CheckIfCNExist(const mp_string &hostName)
{
    std::vector<mp_string> contentVec;
    if (CMpFile::ReadFile(ETC_HOSTS_PATH_LINUX, contentVec) != MP_SUCCESS) {
        return false;
    }
    for (auto contentStr : contentVec) {
        DBGLOG("The string is %s.", contentStr.c_str());
        if (contentStr.find(hostName) != mp_string::npos) {
            return true;
        }
    }
    return false;
}

mp_int32 CSystemCall::AddHostNameToFile(const mp_string& hostName)
{
    mp_uint32 fileSize = 0;
    CMpFile::FileSize(ETC_HOSTS_PATH_LINUX.c_str(), fileSize);
    if (fileSize > MB_TO_BYTE) {
        ERRLOG("The file %s is too big, exit!");
        return MP_FAILED;
    }
    if (CheckIfCNExist(hostName)) {
        INFOLOG("The host name is in file, no need to add, hostname is %s.", hostName.c_str());
        return MP_SUCCESS;
    }

    std::vector<mp_string> content;
    content.push_back(IPV4_STRING + " " + hostName);
    content.push_back(IPV6_STRING + " " + hostName);
    if (CIPCFile::AppendFile(ETC_HOSTS_PATH_LINUX, content) != MP_SUCCESS) {
        ERRLOG("Add failed!");
        return MP_FAILED;
    }
    if (ChmodFile(ETC_HOSTS_PATH_LINUX, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH) != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "chmod file '%s' failed.", ETC_HOSTS_PATH_LINUX.c_str());
        return MP_FAILED;
    }
    INFOLOG("Add successfully!");
    return MP_SUCCESS;
}

mp_int32 CSystemCall::ExecAddHostNameToFile(const mp_string& strUniqueID, const std::vector<mp_string>& vecParam)
{
    if (vecParam.size() != 1) {
        ERRLOG("The number of parameters is error!");
        return MP_FAILED;
    }
    mp_string hostName = vecParam[0];
    return AddHostNameToFile(hostName);
}

mp_int32 CSystemCall::CheckParentPath(const mp_string& path)
{
    mp_string strCommand = "stat -c %u " + path;
    vector<mp_string> vecRlt;
    CHECK_FAIL_EX(CheckCmdDelimiter(path));
    mp_int32 iRet = CSystemExec::ExecSystemWithEcho(strCommand, vecRlt, MP_FALSE);
    if (iRet != MP_SUCCESS || vecRlt.empty()) {
        ERRLOG("Failed to exec stat, result is %d.", iRet);
        return MP_FAILED;
    }

    iRet = atol(vecRlt.front().c_str());
    if (iRet != NUM_0) {
        ERRLOG("Failed to verify the process, the process uid  is %d.", iRet);
        return MP_FAILED;
    }

    return MP_SUCCESS;
}

mp_int32 CSystemCall::WriteCNToHosts(const mp_string& strUniqueID, const std::vector<mp_string>& vecParam)
{
    if (vecParam.size() != IMPORT_CERT_PARAM_NUMBER) {
        return MP_FAILED;
    }
    mp_string content = vecParam[0] + " " + vecParam[1];
    mp_string hostsFile = "/etc/hosts";
    std::vector<mp_string> vecFileContent;
    mp_int32 iRet = CIPCFile::ReadFile(hostsFile, vecFileContent);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Read file %s failed.", hostsFile.c_str());
        return MP_FAILED;
    }
    for (auto &iter : vecFileContent) {
        if (iter == content) {
            INFOLOG("The cert CN has been add to hosts.");
            return MP_SUCCESS;
        }
    }

    vecFileContent.push_back(content);
    CIPCFile::WriteFile(hostsFile, vecFileContent);
    INFOLOG("Write %s to %s succ.", content.c_str(), hostsFile.c_str());
    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Function Name: ScanDirAndFileForInstantlyMount
Description  : 获取文件系统内目录和文件列表，root提权执行，不必修改属组，就可读取目录
               执行成功后，将结果写入结果文件中
Others       :-------------------------------------------------------- */
mp_int32 CSystemCall::ScanDirAndFileForInstantlyMount(const mp_string &strUniqueID,
    const std::vector<mp_string> &vecParam)
{
    LOGGUARD("");
    mp_string path = vecParam.empty() ? "" : vecParam.front();
    std::vector<mp_string> vecFolderPath;
    std::vector<mp_string> vecFilePath;
    mp_int32 iRet = ScanDirAndFile(path, vecFolderPath, vecFilePath);
    if (iRet != MP_SUCCESS) {
        WARNLOG("ScanDirAndFile failed.iRet is:%d.", iRet);
        return MP_FAILED;
    }
    INFOLOG("Scan result: folder size %d, file size %d.", vecFolderPath.size(), vecFilePath.size());
    vector<mp_string> vecResult;
    vecResult.insert(vecResult.end(), vecFolderPath.begin(), vecFolderPath.end());
    // 目录和文件以INSTANLY_MOUNT_SCAN_RESULT_SPLIT_STR分隔
    vecResult.push_back(INSTANLY_MOUNT_SCAN_RESULT_SPLIT_STR);
    vecResult.insert(vecResult.end(), vecFilePath.begin(), vecFilePath.end());
    return CIPCFile::WriteResult(strUniqueID, vecResult);
}

/* ------------------------------------------------------------
Function Name: ScanDirAndFileForInstantlyMount
Description  : 文件系统内目录和文件列表写入NFS文件系统，rdadmin无权限操作，需要root提权执行
               执行成功后，将结果写入结果文件中
Others       :-------------------------------------------------------- */
mp_int32 CSystemCall::WriteScanResultForInstantlyMount(const mp_string &strUniqueID,
    const std::vector<mp_string> &vecParam)
{
    LOGGUARD("");
    if (vecParam.size() < WRITE_SCAN_RESULT_PARAM_NUMBER) {
        ERRLOG("Param is invalid, size:%d.", vecParam.size());
        return MP_FAILED;
    }
    mp_string savePrePath = vecParam.front();
    mp_string savePath = vecParam[1];
    mp_string filePath = vecParam[2];
    DBGLOG("param1: %s, param2: %s param3: %s.", savePrePath.c_str(), savePath.c_str(), filePath.c_str());
    std::vector<mp_string> vecFilePath;
    if (vecParam.size() == WRITE_SCAN_RESULT_PARAM_NUMBER) {
        // 扫描结果为空时，依然生成文件
        WARNLOG("Scan result is empty.");
    } else {
        vecFilePath.assign(vecParam.begin() + WRITE_SCAN_RESULT_PARAM_NUMBER, vecParam.end());
    }
    if (!CMpFile::DirExist(savePrePath.c_str()) && CMpFile::CreateDir(savePrePath.c_str()) != MP_SUCCESS) {
        ERRLOG("Failed to create savePrePath %s.", savePrePath.c_str());
        return MP_FAILED;
    }
    if (!CMpFile::DirExist(savePath.c_str()) && CMpFile::CreateDir(savePath.c_str()) != MP_SUCCESS) {
        ERRLOG("Failed to create savePath %s.", savePath.c_str());
        return MP_FAILED;
    }
    // 检查写入文件路径和目录是否有效
    mp_int32 iRet = CheckPathIsValid(filePath);
    if (iRet != MP_SUCCESS) {
        ERRLOG("filepath %s is invalid.", filePath.c_str());
        return MP_FAILED;
    }
    CIPCFile::WriteFile(filePath, vecFilePath);
    INFOLOG("Write %s succ.", filePath.c_str());
    return MP_SUCCESS;
}

mp_int32 CSystemCall::CheckPathIsValid(const mp_string &filePath)
{
    // E6000设备即时挂载，需要扫描副本目录用于创建文件克隆，写入扫描结果时，需要提权到root执行
    // root执行会校验目录，修改时需要同步修改PluginSubPostJob::ScanAndRecordFile()和PrepareFileSystem定义
    static const mp_string DATA_DIR_NAME = "Data";
    static const mp_string META_DIR_NAME = "Meta";
    static const mp_string RECORD_FILE_NAME = "RecordFile.txt";     // 保存扫描出来的文件
    static const mp_string RECORD_DIR_NAME = "RecordDir.txt";       // 保存扫描出来的目录
    static const mp_string MOUNT_PUBLIC_PATH = "/mnt/databackup/";  // 挂载目录的前置，文件在AIX和soalris上文件系统/mnt开头，文件不会走当前流程

    mp_string realPath = filePath;
    // FormattingPath的目录不存在会返回失败，当前函数只归一化目录，不判断返回值
    (mp_void) CMpString::FormattingPath(realPath);
    DBGLOG("Format path is %s, file ath %s.", realPath.c_str(), filePath.c_str());
    
    if (realPath.find(MOUNT_PUBLIC_PATH) != 0) {
        ERRLOG("realPath %s isn't begin with %s.", realPath.c_str(), MOUNT_PUBLIC_PATH.c_str());
        return MP_FAILED;
    }

    mp_string fileName = BaseFileName(realPath);
    if ((fileName != DATA_DIR_NAME + RECORD_FILE_NAME) && (fileName != DATA_DIR_NAME + RECORD_DIR_NAME) &&
        (fileName != META_DIR_NAME + RECORD_FILE_NAME) && (fileName != META_DIR_NAME + RECORD_DIR_NAME)) {
        ERRLOG("File name %s isn't fix file name.", fileName.c_str());
        return MP_FAILED;
    }

    return MP_SUCCESS;
}

mp_int32 CSystemCall::ScanDirAndFile(
    mp_string &rootPath, std::vector<mp_string> &rootfolderpath, std::vector<mp_string> &rootfilepath)
{
    // 获取当前目录下的所有的文件全路径
    std::vector<mp_string> vecfilePath;
    int iRet = GetFilePath(rootPath, vecfilePath);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Scan file path failed.iRet is:%d.", iRet);
        return iRet;
    }
    rootfilepath.insert(rootfilepath.end(), vecfilePath.begin(), vecfilePath.end());
 
    // 获取当前目录下的所有的子目录全路径
    std::vector<mp_string> vecfolderPath;
    iRet = GetFolderPath(rootPath, vecfolderPath);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Scan Folder path failed.iRet is:%d.", iRet);
        return iRet;
    }
    rootfolderpath.insert(rootfolderpath.end(), vecfolderPath.begin(), vecfolderPath.end());
 
    // 递归扫描子目录下的文件和目录
    for (mp_string folderPath : vecfolderPath) {
        iRet = ScanDirAndFile(folderPath, rootfolderpath, rootfilepath);
        if (iRet != MP_SUCCESS) {
            ERRLOG("ScanDirAndFile failed.iRet is:%d.", iRet);
            return iRet;
        }
    }
    return MP_SUCCESS;
}
 
mp_int32 CSystemCall::GetFolderPath(mp_string &strFolder, std::vector<mp_string> &vecFolderPath)
{
    std::vector<mp_string> folderNameList;
    folderNameList.clear();
    mp_int32 iRet = CMpFile::GetFolderDir(strFolder, folderNameList);
    if (iRet != MP_SUCCESS) {
        return iRet;
    }
    for (mp_string strName : folderNameList) {
        // 快照可见场景下，.snapshot为应用快照目录，非应用目录，过滤掉，不必放到待扫描目录中
        if (strName == ".snapshot") {
            INFOLOG(".snapshot of %s don't need to scan.", strFolder.c_str());
            continue;
        }
        vecFolderPath.push_back(strFolder + PATH_SEPARATOR + strName);
    }
    return MP_SUCCESS;
}
 
mp_int32 CSystemCall::GetFilePath(mp_string &strFolder, std::vector<mp_string> &vecFolderPath)
{
    std::vector<mp_string> folderNameList;
    mp_int32 iRet = CMpFile::GetFolderFile(strFolder, folderNameList);
    if (iRet != MP_SUCCESS) {
        return iRet;
    }
    for (mp_string strName : folderNameList) {
        vecFolderPath.push_back(strFolder + PATH_SEPARATOR + strName);
    }
    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Function Name: InitSanclientMap
Description  : 初始化sanclient脚本命令map
Others       :-------------------------------------------------------- */
mp_void CCommandMap::InitSanclientMap()
{
    (mp_void)m_mapCommand.emplace (
        map<mp_int32, mp_string>::value_type(ROOT_COMMAND_SANCLIENT_ENVCHECK, "sanclientcheck.sh"));
    (mp_void)m_mapCommand.emplace (
        map<mp_int32, mp_string>::value_type(ROOT_COMMAND_SANCLIENT_ACTION, "sanclientaction.sh"));
    (mp_void)m_mapCommand.emplace (
        map<mp_int32, mp_string>::value_type(ROOT_COMMAND_SANCLIENT_CLEAR, "sanclientclear.sh"));
    (mp_void)m_mapCommand.emplace (
        map<mp_int32, mp_string>::value_type(ROOT_COMMAND_SANCLIENT_ACTION_ISCSI, "sanclientactioniscsi.sh"));
    (mp_void)m_mapCommand.emplace (
        map<mp_int32, mp_string>::value_type(ROOT_COMMAND_SANCLIENT_COPY_LOG_META, "sanclientcopylogmeta.sh"));
}


/* ------------------------------------------------------------
Function Name: InitDpcCmdMap
Description  : 初始化Dpc相关命令map
Others       :-------------------------------------------------------- */
mp_void CCommandMap::InitDpcCmdMap()
{
    (mp_void)m_mapCommand.emplace (
        map<mp_int32, mp_string>::value_type(ROOT_COMMAND_DPC_CONFIG_FLOW_CONTROL, "config_dpc_flow_control.sh"));
}