/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file RootCaller.cpp
 * @brief  secure caller funtion
 * @version 1.0.0
 * @date 2020-08-01
 * @author wangguitao 00510599
 */
#include "securecom/RootCaller.h"
#include <sstream>
#include <fstream>
#include <cstdlib>
#include "common/Utils.h"
#include "securecom/UniqueId.h"
#include "common/Log.h"
#include "common/File.h"
#include "common/Pipe.h"
#include "common/Path.h"
#include "common/ErrorCode.h"
#include "common/CSystemExec.h"
#include "securec.h"
using namespace std;

namespace {
    const mp_string INSTALL_TYPE_INTERNAL = "1";
    const mp_string CMD_SUDO = "sudo";
}

namespace {
mp_string GetSudoCmdStr()
{
    mp_string strFilePath = CPath::GetInstance().GetConfFilePath(CFG_RUNNING_PARAM);
    mp_bool iRet = CMpFile::FileExist(strFilePath);
    if (iRet != MP_TRUE) {
        ERRLOG("The testcfg.tmp file does not exist.");
        return "";
    }

    std::ifstream stream;
    stream.open(strFilePath.c_str(), std::ifstream::in);
    mp_string line;
    mp_string strSceneText;
    mp_string strText = "BACKUP_SCENE=";

    if (!stream.is_open()) {
        ERRLOG("The testcfg.tmp file can't open");
        return "";
    }

    while (getline(stream, line)) {
        if (line.find(strText.c_str()) != std::string::npos) {
            strSceneText = line;
            break;
        }
    }
    stream.close();

    std::size_t start = strSceneText.find("=", 0);
    if (start == std::string::npos) {
        ERRLOG("The testcfg.tmp file format error");
        return "";
    }

    mp_string strUserName;
    mp_ulong iErrCode;
    GetCurrentUserName(strUserName, iErrCode);
    mp_string strSceneType = strSceneText.substr(start + 1);
    if (strSceneType != INSTALL_TYPE_INTERNAL || strUserName == "root") {
        return "";
    }
    return CMD_SUDO;
}
}

CRootCaller::CRootCaller()
{
}
CRootCaller::~CRootCaller()
{
}

mp_int32 CRootCaller::Exec(mp_int32 iCommandID, const mp_string& strParam,
    std::vector<mp_string> pvecResult[], mp_int32 (*cb)(void*, const mp_string&), void* pTaskStep)
{
    std::vector<mp_string> vecParam;
    CMpString::StrSplitEx(vecParam, strParam, NODE_COLON);
    return ExecEx(iCommandID, vecParam, pvecResult, cb, pTaskStep);
}

/* ------------------------------------------------------------
Function Name: Exec
Description  : root权限执行函数，供其他模块静态调用
               iCommandID: 命令ID,定义参见ROOT_COMMAND
               strParam: root权限执行参数，非引用，如无参数，直接输入""
               pvecResult: 保存执行结果的vector，如无需结果，直接输入NULL
-------------------------------------------------------- */
mp_int32 CRootCaller::ExecEx(mp_int32 iCommandID, const std::vector<mp_string>& vecParam,
    std::vector<mp_string> pvecResult[], mp_int32 (*cb)(void*, const mp_string&), void* pTaskStep)
{
    mp_int32 iRet = MP_SUCCESS;
    mp_string strUniqueID = CUniqueID::GetInstance().GetString();
    // update innerPID
    if ((cb != NULL) && (pTaskStep != NULL)) {
        iRet = cb(pTaskStep, strUniqueID);
        if (iRet != MP_SUCCESS) {
            ERRLOG("Update inner PID failed, ret %d.", iRet);
            return iRet;
        }
    }

    mp_char acCmd[MAX_MAIN_CMD_LENGTH] = {0};
    mp_string strSBinFilePath = CPath::GetInstance().GetSBinFilePath(ROOT_EXEC_NAME);
    strSBinFilePath = CMpString::BlankComma(strSBinFilePath);
    mp_string m_cmdSudo = GetSudoCmdStr();
    CHECK_FAIL(snprintf_s(acCmd, sizeof(acCmd), sizeof(acCmd) - 1,  "%s %s -c %d -u %s -n",
        m_cmdSudo.c_str(), strSBinFilePath.c_str(), iCommandID, strUniqueID.c_str()));
    mp_string strCmd = acCmd;
    CHECK_FAIL_EX(CheckCmdDelimiter(strCmd));
    iRet = CSystemExec::ExecSystemWithSecurityParam(strCmd, vecParam);
    if (iRet != MP_SUCCESS) {
        // 错误码转换，脚本执行返回转换后的错误码，非脚本执行统一返回-1
        if (iCommandID >= ROOT_COMMAND_SCRIPT_BEGIN && iCommandID <= ROOT_COMMAND_SCRIPT_END) {
            mp_int32 iNewRet = ErrorCode::GetInstance().GetErrorCode(iRet);
            ERRLOG("Exec script failed, command id %d, initial return code is %d, transformed return code is %d",
                iCommandID, iRet, iNewRet);
            if (iCommandID == ROOT_COMMAND_SCRIPT_MOUNT_NAS_FILESYS) {
                ReadResultFile(iCommandID, strUniqueID, pvecResult);
                return iNewRet;
            }
            if (iCommandID != ROOT_COMMAND_THIRDPARTY && iCommandID != ROOT_COMMAND_SCRIPT_USER_DEFINED_USER_DO) {
                (mp_void)RemoveFile(RESULT_TMP_FILE + strUniqueID);
            }
            iRet = iNewRet;
        } else {
            ERRLOG("Exec system failed, commandid %d, initial return code %d", iCommandID, iRet);
            RemoveFile(RESULT_TMP_FILE + strUniqueID);
        }
    }
    mp_int32 res = ReadResultFile(iCommandID, strUniqueID, pvecResult);
    return iRet == MP_SUCCESS ? res : iRet;
}

/* ------------------------------------------------------------
Function Name: Exec
Description  : root权限执行用户指定的脚本，供其他模块静态调用
               cmd: 要执行的命令
               scriptCmd: 要执行的脚本加参数，以空格区分
-------------------------------------------------------- */
mp_int32 CRootCaller::ExecUserDefineScript(mp_int32 iCommandID, const std::string &scriptCmd)
{
    LOGGUARD("");
    mp_char acCmd[MAX_MAIN_CMD_LENGTH] = {0};
    mp_string strSBinFilePath = CPath::GetInstance().GetSBinFilePath(ROOT_EXEC_NAME);
    strSBinFilePath = CMpString::BlankComma(strSBinFilePath);
    std::string m_cmdSudo = GetSudoCmdStr();
    CHECK_FAIL(snprintf_s(acCmd,
        sizeof(acCmd),
        sizeof(acCmd) - 1,
        "%s %s -c %d -u %s",
        m_cmdSudo.c_str(),
        strSBinFilePath.c_str(),
        iCommandID,
        scriptCmd.c_str()));
    // 检查是否包含非法字符
    std::string strCmd(acCmd);
    CHECK_FAIL_EX(CheckCmdDelimiter(strCmd));
    mp_uint32 iRet = CSystemExec::ExecSystemWithoutEcho(strCmd);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Exec system failed, commandid %d, scriptCmd %s", iCommandID, acCmd);
    }

    return iRet;
}
/* ------------------------------------------------------------
Function Name: ReadResultFile
Description  : Exec函数执行完后，如果需要读取结果文件，将调用这个函数
               iCommandID: 命令ID,定义参见ROOT_COMMAND
               strUniqueID: Exec 函数执行的唯一id
               pvecResult: 保存执行结果的vector，如无需结果，直接输入NULL
-------------------------------------------------------- */
mp_int32 CRootCaller::ReadResultFile(mp_int32 iCommandID, const mp_string& strUniqueID, vector<mp_string> pvecResult[])
{
    mp_int32 iRet = MP_SUCCESS;
    mp_string strFileName;

    if (pvecResult != NULL) {
        if (iCommandID == ROOT_COMMAND_SCRIPT_PID) {
            strFileName = "scriptpid_tmp" + strUniqueID;
        } else if (iCommandID == ROOT_COMMAND_THIRDPARTY) {
            strFileName = "RST" + strUniqueID + ".txt";
        } else {
            strFileName = RESULT_TMP_FILE + strUniqueID;
        }

        if (ErrorCode::GetInstance().GetSpecCommonID(iCommandID) != MP_FAILED) {
            mp_string strFilePathFile = CPath::GetInstance().GetStmpFilePath(strFileName);
            // 如果结果文件不存在，有可能是没有满足条件的数据，这种情况返回成功
            if (!CMpFile::FileExist(strFilePathFile)) {
                ERRLOG("ReadResult: Can't find file %s.", strFileName.c_str());
                return ERROR_FILESYSTEM_NO_SPACE;
            }
        }
        iRet = CIPCFile::ReadResult(strFileName, *pvecResult);
        if (iRet != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "Read result file failed.");
        }

        if (iCommandID != ROOT_COMMAND_SCRIPT_PID) {
            RemoveFile(strFileName);
        }
        if (iCommandID == ROOT_COMMAND_SCRIPT_ORACLENATIVE_BACKUPDATA) {
            RemoveFile("scriptpid_tmp" + strUniqueID);
        }
    }
    return iRet;
}

mp_int32 CRootCaller::RemoveFile(const mp_string& fileName)
{
    mp_string strFilePathFile = CPath::GetInstance().GetStmpFilePath(fileName);
    if (!CMpFile::FileExist(strFilePathFile)) {
        COMMLOG(OS_LOG_WARN, "Can't find file %s.", strFilePathFile.c_str());
        return MP_SUCCESS;
    }
    mp_int32 iRet = Exec(ROOT_COMMAND_RM, strFilePathFile, NULL);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "rm file(%s) failed.", fileName.c_str());
    }
    return iRet;
}
