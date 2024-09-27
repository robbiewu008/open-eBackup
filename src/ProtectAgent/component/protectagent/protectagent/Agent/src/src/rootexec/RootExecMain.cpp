#include <iostream>
#include <sys/types.h>
#include <pwd.h>
#include "common/Types.h"
#include "rootexec/SystemCall.h"
#include "common/ConfigXmlParse.h"
#include "common/Log.h"
#include "common/Defines.h"
#include "common/Utils.h"
#include "common/Path.h"
#include "securecom/UniqueId.h"
#include "common/ErrorCode.h"
#include "securecom/RootCaller.h"
#include "securecom/CryptAlg.h"
#ifndef WIN32
#include "common/StackTracer.h"
#endif

/* 定义命令函数处理 */
using SYSTEM_CALL_EXEC_FUNC = mp_int32(*)(const mp_string&, const std::vector<mp_string>&);
static SYSTEM_CALL_EXEC_FUNC g_systemCmdFuncs[ROOT_COMMAND_BUTT];

namespace {
/* ------------------------------------------------------------
Function Name: IsRunManually
Description  : 判断是否是自己启动

Others       :-------------------------------------------------------- */

const mp_string HOST_ENV_INSTALL_PATH = "DATA_BACKUP_AGENT_HOME";

mp_bool IsRunManually()
{
    pid_t myPid;
    pid_t myGid;

    myPid = getpid();
    myGid = getpgrp();
    if (myPid == myGid) {
        return MP_TRUE;
    }

    return MP_FALSE;
}

mp_int32 COMProcessCommand(mp_int32 iCommandID, const mp_string& strUniqueID,
    const std::vector<mp_string>& vecParam)
{
    if (iCommandID > ROOT_COMMAND_SCRIPT_BEGIN && iCommandID < ROOT_COMMAND_SCRIPT_END) {
        switch (iCommandID) {
            case ROOT_COMMAND_SCRIPT_FREEZEAPP:
            case ROOT_COMMAND_SCRIPT_THAWAPP:
                return CSystemCall::ExecAppScript(strUniqueID, iCommandID);
            case ROOT_COMMAND_SCRIPT_ADD_FRIEWALL:
                return CSystemCall::ExecAddFirewall();
            case ROOT_COMMAND_USER_DEFINED:
                return CSystemCall::ExecUserDefineScript(strUniqueID);
            case ROOT_COMMAND_THIRDPARTY:
                return CSystemCall::ExecThirdPartyScript(strUniqueID, vecParam);
            case ROOT_COMMAND_SCRIPT_USER_DEFINED_USER_DO:
                return CSystemCall::ExecScriptByScriptUser(strUniqueID, vecParam);
            default:
                return CSystemCall::ExecScript(strUniqueID, iCommandID, vecParam);
        }
    } else if (iCommandID > ROOT_COMMAND_SYSTEM_BEGIN && iCommandID < ROOT_COMMAND_SYSTEM_END) {
        return CSystemCall::ExecSysCmd(strUniqueID, iCommandID, vecParam);
    }

    for (int i = 0; i < ROOT_COMMAND_BUTT; ++i) {
        g_systemCmdFuncs[i] = NULL;
    }

    // 初始化函数表
    g_systemCmdFuncs[ROOT_COMMAND_80PAGE] = &CSystemCall::GetDisk80Page;
    g_systemCmdFuncs[ROOT_COMMAND_83PAGE] = &CSystemCall::GetDisk83Page;
    g_systemCmdFuncs[ROOT_COMMAND_00PAGE] = &CSystemCall::GetDisk00Page;
    g_systemCmdFuncs[ROOT_COMMAND_C8PAGE] = &CSystemCall::GetDiskC8Page;
    g_systemCmdFuncs[ROOT_COMMAND_CAPACITY] = &CSystemCall::GetDiskCapacity;
    g_systemCmdFuncs[ROOT_COMMAND_VENDORANDPRODUCT] = &CSystemCall::GetVendorAndProduct;
    g_systemCmdFuncs[ROOT_COMMAND_BATCH_GETLUN_INFO] = &CSystemCall::BatchGetLUNInfo;
    g_systemCmdFuncs[ROOT_COMMAND_HOSTLUNID] = &CSystemCall::GetHostLunID;
    g_systemCmdFuncs[ROOT_COMMAND_DATAPROCESS_START] = &CSystemCall::StartDataProcess;
    g_systemCmdFuncs[ROOT_COMMAND_WRITE_CERT_CN] = &CSystemCall::WriteCNToHosts;
    g_systemCmdFuncs[ROOT_COMMAND_ADD_HOSTS] = &CSystemCall::ExecAddHostNameToFile;
    g_systemCmdFuncs[ROOT_COMMAND_SCAN_DIR_FILE] = &CSystemCall::ScanDirAndFileForInstantlyMount;
    g_systemCmdFuncs[ROOT_COMMAND_WRITE_SCAN_RESULT] = &CSystemCall::WriteScanResultForInstantlyMount;
    if (iCommandID >= ROOT_COMMAND_BUTT) {
        COMMLOG(OS_LOG_ERROR, "Unknown Command, ID is %d.", iCommandID);
        return MP_FAILED;
    }

    SYSTEM_CALL_EXEC_FUNC pFunc = g_systemCmdFuncs[iCommandID];
    if (!pFunc) {
        COMMLOG(OS_LOG_ERROR, "Can not get function, ID is %d.", iCommandID);
        return MP_FAILED;
    }

    return pFunc(strUniqueID, vecParam);
}

/* ------------------------------------------------------------
Function Name: Exec
Description  : 根据输入参数执行具体操作
-------------------------------------------------------- */
mp_int32 Exec(mp_int32 iCommandID, const mp_string& strUniqueID, const std::vector<mp_string>& vecParam)
{
    mp_int32 IRet = setuid(0);
    if (IRet != MP_SUCCESS) {
        printf("Set effective user id to root failed, errno[%d]:%s.\n", errno, strerror(errno));
        return IRet;
    }

    LOGGUARD("Command ID is %d, UniqueID = %s", iCommandID, strUniqueID.c_str());
    // CodeDex误报，Often Misused:Privilege Management
    struct passwd* pPwd = getpwuid(0);
    if (pPwd == nullptr) {
        COMMLOG(OS_LOG_ERROR, "Get password database info of root failed, errno[%d]:%s.", errno, strerror(errno));

        return ERROR_COMMON_SYSTEM_CALL_FAILED;
    }

    IRet = setgid(pPwd->pw_gid);
    if (IRet != MP_SUCCESS) {
        COMMLOG(
            OS_LOG_ERROR, "Set effective group id to %d failed, errno[%d]:%s.", pPwd->pw_gid, errno, strerror(errno));
        return IRet;
    }

    mp_string customPath = CPath::GetInstance().GetRootPath() + "/../../..";
    if (!CMpString::FormattingPath(customPath)) {
        ERRLOG("Format path (%s) error.", customPath.c_str());
        return MP_FAILED;
    }

    IRet = setenv(HOST_ENV_INSTALL_PATH.c_str(), customPath.c_str(), 0);
    if (IRet != MP_SUCCESS) {
        ERRLOG("Failed to set the environment, IRet=%d, path=%s.", IRet, customPath.c_str());
        return IRet;
    }

    IRet = COMProcessCommand(iCommandID, strUniqueID, vecParam);
    if (IRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Process Command(ID = %d) failed, ret %d.", iCommandID, IRet);
        return IRet;
    }

    return MP_SUCCESS;
}

mp_int32 InitPath(const mp_string& strExecPath)
{
    // 初始化路径
    mp_int32 IRet = CPath::GetInstance().Init(strExecPath);
    if (IRet != MP_SUCCESS) {
        printf("Init path %s failed.\n", strExecPath.c_str());
        return IRet;
    }
    // 初始化xml配置
    IRet = CConfigXmlParser::GetInstance().Init(CPath::GetInstance().GetConfFilePath(AGENT_XML_CONF));
    if (IRet != MP_SUCCESS) {
        printf("Init xml conf file %s failed.\n", AGENT_XML_CONF.c_str());
        return IRet;
    }

    // 初始化日志路径
    CLogger::GetInstance().Init(ROOT_EXEC_LOG_NAME.c_str(), CPath::GetInstance().GetSlogPath());
    return MP_SUCCESS;
}

mp_int32 CheckParam(mp_int32 iCommandID, const mp_string& strUniqueID,
    mp_int32 lParamNum, std::vector<mp_string>& vecParam)
{
    if (iCommandID == MP_FAILED || strUniqueID.empty()) {
        printf("Usage: rootexec -c <Command_ID> -u <Unique_ID>\n");
        return ERROR_COMMON_INVALID_PARAM;
    }

    if (iCommandID != ROOT_COMMAND_USER_DEFINED) {
        std::regex reg("^\\d{1,}_\\d{1,}$");
        std::smatch match;
        std::regex_search(strUniqueID, match, reg);
        if (match.empty()) {
            printf("Invalid -u parameter %s.\n", strUniqueID.c_str());
            return ERROR_COMMON_INVALID_PARAM;
        }
    }

    if (lParamNum < 0 || lParamNum > MAX_LINE_SIZE) {
        printf("Invalid -n parameter %d.\n", lParamNum);
        return ERROR_COMMON_INVALID_PARAM;
    }

    vecParam.reserve(lParamNum);
    for (int i = 0; i < lParamNum; ++i) {
        mp_string str;
        std::cin >> str;
        vecParam.emplace_back(std::move(str));
    }
    return MP_SUCCESS;
}

mp_int32 CheckParentPid()
{
    mp_string ppid;
    mp_bool bRet = CSystemCall::GetParentPid(ppid);
    if (bRet != MP_TRUE) {
        ERRLOG("Failed to obtain the parent process ID.");
        return MP_FAILED;
    }

    mp_string processInfo;
    bRet = CSystemCall::GetProcessInfo(ppid, processInfo);
    if (bRet != MP_TRUE) {
        ERRLOG("Failed to obtain process information.");
        return MP_FAILED;
    }
    DBGLOG("Parent process is %s.", processInfo.c_str());

    mp_string agentPath = CPath::GetInstance().GetRootPath() + "/..";
    CMpString::FormattingPath(agentPath);
    agentPath.append("/");
    mp_string tmp = processInfo.substr(0, agentPath.length());
    if (tmp != agentPath) {
        mp_string pppid;
        bRet = CSystemCall::GetParentPidImpl(ppid, pppid);
        if (bRet != MP_TRUE) {
            ERRLOG("Failed to obtain the grandfather  process ID.");
            return MP_FAILED;
        }

        bRet = CSystemCall::GetProcessInfo(pppid, processInfo);
        if (bRet != MP_TRUE) {
            ERRLOG("Failed to obtain process information.");
            return MP_FAILED;
        }
        DBGLOG("Parent process is %s.", processInfo.c_str());

        tmp = processInfo.substr(0, agentPath.length());
        if (tmp != agentPath) {
            ERRLOG("Failed to verify the process, the process path  is %s.", tmp.c_str());
            return MP_FAILED;
        }
    }

    if (CSystemCall::CheckParentPath(processInfo) != MP_SUCCESS) {
        ERRLOG("Failed to verify the process, the process is %s.", processInfo.c_str());
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

}
/*------------------------------------------------------------
Function Name: main函数
               Usage: rootexec -r <AGENT_ROOT> -c <命令id> -u <用户id> -n <入参数量>]);
               -a <命令id>    脚本/系统命令内部命令字，定义参见ROOT_COMMAND，必选参数
               -i <全局不重复id>  当前执行用户不重复的id，用于生成临时文件，必选参数
-------------------------------------------------------- */
mp_int32 main(mp_int32 argc, mp_char** argv)
{
    mp_int32 ICommandID = MP_FAILED;
    mp_string strUniqueID;
    mp_int32 lParamNum = 0;

    mp_string pszOptString = "c:u:n:";
    mp_int32 IOpt = getopt(argc, argv, pszOptString.c_str());
    while (-1 != IOpt) {
        switch (IOpt) {
            case 'c':
                ICommandID = atoi(optarg);
                break;
            case 'u':
                strUniqueID = optarg;
                break;
            case 'n':
                lParamNum = atoi(optarg);
                break;
            default:
                return ERROR_COMMON_INVALID_PARAM;
        }

        IOpt = getopt(argc, argv, pszOptString.c_str());
    }
    std::vector<mp_string> vecParam;
    mp_int32 iRet = CheckParam(ICommandID, strUniqueID, lParamNum, vecParam);
    if (iRet != MP_SUCCESS) {
        return iRet;
    }

#ifndef WIN32
    StackTracer stackTracer;
#endif
    iRet = InitPath(argv[0]);
    if (iRet != MP_SUCCESS) {
        return iRet;
    }

    iRet = Exec(ICommandID, strUniqueID, vecParam);
#ifndef WIN32
    (mp_void) ChangeGmonDir();  // change profile out put dir
#endif
    return iRet;
}