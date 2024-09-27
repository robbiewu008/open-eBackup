/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file WinService.cpp
 * @brief  The implemention about windows service
 * @version 1.0.0.0
 * @date 2020-08-01
 * @author wangguitao 00510599
 */
#ifdef WIN32

#include "tools/winservice/WinService.h"
#include "common/Path.h"
#include "common/Log.h"
#include "common/Defines.h"
#include "common/ConfigXmlParse.h"
#include "common/ServiceHandler.h"
#include "common/CSystemExec.h"
#include "securecom/CryptAlg.h"

const mp_uchar WINSERVICE_NUM_2  = 2;
const mp_uchar WINSERVICE_NUM_3  = 3;
const mp_uchar WINSERVICE_NUM_4  = 4;

// Function Name: CheckBinName
// Description  : 检查进程名字
mp_bool CheckBinName(mp_string strBinName)
{
    if (strcmp(strBinName.c_str(), AGENT_EXEC_NAME.c_str()) == 0 ||
        strcmp(strBinName.c_str(), MONITOR_EXEC_NAME.c_str()) == 0 ||
        strcmp(strBinName.c_str(), NGINX_AS_PARAM_NAME.c_str()) == 0) {
        return MP_TRUE;
    } else {
        return MP_FALSE;
    }
}

// Function Name: CheckOperate
// Description  : 检查操作类型
mp_bool CheckOperate(mp_string strOperate)
{
    if (strcmp(strOperate.c_str(), INSTALL_OPERATOR.c_str()) == 0 ||
        strcmp(strOperate.c_str(), UNINSTALL_OPERATOR.c_str()) == 0 ||
        strcmp(strOperate.c_str(), RUN_OPERATOR.c_str()) == 0 ||
        strcmp(strOperate.c_str(), CHANGE_USER_OPERATOR.c_str()) == 0) {
        return MP_TRUE;
    } else {
        return MP_FALSE;
    }
}

// Function Name: NginxServiceHandler
// Description  : nginx服务处理函数
DWORD WINAPI NginxServiceHandler(DWORD request, DWORD dwEventType, LPVOID lpEventData, LPVOID lpContext)
{
    LOGGUARD("");
    mp_string strCmd = CPath::GetInstance().GetBinFilePath(PROCESS_STOP_SCRIPT);
    // 校验脚本签名
    mp_int32 iRet = InitCrypt(KMC_ROLE_TYPE_MASTER);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Init crypt failed, ret = %d.", iRet);
        return iRet;
    }
    // 程序即将退出，此处不判断返回值
    (mp_void) FinalizeCrypt();
    strCmd = CMpString::BlankComma(strCmd);
    strCmd = strCmd + " " + NGINX_AS_PARAM_NAME;

    switch (request) {
        case SERVICE_CONTROL_STOP:
            iRet = CSystemExec::ExecSystemWithoutEcho(strCmd);
            if (iRet != MP_SUCCESS) {
                COMMLOG(OS_LOG_ERROR, "Stop nginx failed.");
                CWinServiceHanlder::UpdateServiceStatus(WinService::GetInstace().GetServiceHandle(),
                    SERVICE_RUNNING, START_SERVICE_TIMEOUT);
                break;
            }

            CWinServiceHanlder::UpdateServiceStatus(WinService::GetInstace().GetServiceHandle(),
                SERVICE_STOPPED, START_SERVICE_TIMEOUT);
            break;
        default:
            break;
    }

    return iRet;
}

// Function Name: NginxServiceMain
// Description  : nginx服务处理函数
mp_void WINAPI NginxServiceMain(DWORD dwArgc, LPTSTR* lpszArgv)
{
    LOGGUARD("");
    // 注册服务处理函数
    WinService::GetInstace().InitServiceHandle();
    if (!WinService::GetInstace().GetServiceHandle()) {
        COMMLOG(OS_LOG_ERROR, "Register nginx service failed.");
        return;
    }

    // 启动Nginx
    mp_string strCmd = CPath::GetInstance().GetBinFilePath(AGENTCLI_EXE);

    strCmd = CMpString::BlankComma(strCmd);
    strCmd = strCmd + " " + NGINX_START;
    mp_int32 iRet = CSystemExec::ExecSystemWithoutEcho(strCmd);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Start nginx failed, iRet = %d.", iRet);
        CWinServiceHanlder::UpdateServiceStatus(WinService::GetInstace().GetServiceHandle(),
            SERVICE_STOPPED, START_SERVICE_TIMEOUT);
        return;
    }

    // 设置运行状态
    CWinServiceHanlder::UpdateServiceStatus(WinService::GetInstace().GetServiceHandle(),
        SERVICE_RUNNING, START_SERVICE_TIMEOUT);
}

// Function Name: RunNginxService
// Description  : nginx服务处理函数
mp_int32 RunNginxService()
{
    LOGGUARD("");
    // 注册服务响应函数
    SERVICE_TABLE_ENTRY st[] = {{(LPSTR)NGINX_SERVICE_NAME.c_str(), NginxServiceMain}, {NULL, NULL}};

    mp_bool bRet = StartServiceCtrlDispatcher(st);
    if (!bRet) {
        COMMLOG(OS_LOG_ERROR, "StartServiceCtrlDispatcher failed, errorno[%d].", GetLastError());
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

mp_int32 GetUserPwd(std::string &outStr)
{
    std::string inStr;
    if (MP_SUCCESS !=
        CConfigXmlParser::GetInstance().GetValueString(CFG_SYSTEM_SECTION, WORKING_USER_PASSWORD, inStr)) {
        COMMLOG(OS_LOG_ERROR, "Get value of working_user_passward failed, get ssl_key_password instead.");

        if (MP_SUCCESS != CConfigXmlParser::GetInstance().GetValueString(
            CFG_MONITOR_SECTION, CFG_NGINX_SECTION, CFG_SSL_KEY_PASSWORD, inStr)) {
            COMMLOG(OS_LOG_ERROR, "Get value of ssl_key_password failed");
            return MP_FAILED;
        }
    }
    DecryptStr(inStr, outStr);
    return MP_SUCCESS;
}

mp_int32 WriteUserPwd(const std::string &inStr)
{
    if (inStr == "") {
        COMMLOG(OS_LOG_INFO, "The pwd is empty!");
        return MP_SUCCESS;
    }
    std::string strGet;
    if (MP_SUCCESS != CConfigXmlParser::GetInstance().GetValueString(
        CFG_SYSTEM_SECTION, WORKING_USER_PASSWORD, strGet)) {
        COMMLOG(OS_LOG_ERROR, "Get value of working_user_passward failed");
        return MP_FAILED;
    }
    if (strGet != "") {
        COMMLOG(OS_LOG_INFO, "The user pwd is already set!");
        return MP_SUCCESS;
    }
    std::string outStr;
    EncryptStr(inStr, outStr);
    if (MP_SUCCESS != CConfigXmlParser::GetInstance().SetValue(CFG_SYSTEM_SECTION, WORKING_USER_PASSWORD, outStr)) {
        COMMLOG(OS_LOG_ERROR, "Set user pwd failed!");
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

// Function Name: ProcessAgentService
// Description  : 通过操作码响应不同agent服务处理函数
mp_int32 ProcessAgentService(mp_string strOperator, mp_string strWorkingUser, mp_string strWorkingUserPwd)
{
    mp_int32 iRet = MP_SUCCESS;
    mp_bool bRet = MP_FALSE;
    if (!strWorkingUser.empty()) {
        if (GetUserPwd(strWorkingUserPwd) != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "Get working user pwd failed!");
            return MP_FAILED;
        }
    }
    // 安装agent服务
    if (strcmp(strOperator.c_str(), INSTALL_OPERATOR.c_str()) == 0) {
        COMMLOG(OS_LOG_INFO, "Install \"%s\" Service.", AGENT_EXEC_NAME);
        mp_string strExecName = AGENT_EXEC_NAME + ".exe";
        mp_string strBinPath = CPath::GetInstance().GetBinFilePath(AGENT_EXEC_NAME);
        strBinPath = CMpString::BlankComma(strBinPath);
        strBinPath = strBinPath + " -s";
        bRet = CWinServiceHanlder::InstallService(strBinPath, AGENT_SERVICE_NAME, strWorkingUser, strWorkingUserPwd);
        if (!bRet) {
            COMMLOG(OS_LOG_ERROR, "Install service \"%s\" failed.", AGENT_SERVICE_NAME);
            iRet = MP_FAILED;
        }
    } else if (strcmp(strOperator.c_str(), UNINSTALL_OPERATOR.c_str()) == 0) { // 卸载Agent服务
        COMMLOG(OS_LOG_INFO, "Uninstall \"%s\" Service.", AGENT_EXEC_NAME);
        bRet = CWinServiceHanlder::UninstallService(AGENT_SERVICE_NAME);
        if (!bRet) {
            COMMLOG(OS_LOG_ERROR, "Uninstall service \"%s\" failed.", AGENT_SERVICE_NAME);
            iRet = MP_FAILED;
        }
    } else if (strcmp(strOperator.c_str(), CHANGE_USER_OPERATOR.c_str()) == 0) {
        COMMLOG(OS_LOG_INFO, "Change user of \"%s\" Service.", AGENT_SERVICE_NAME);
        bRet = CWinServiceHanlder::ChangeServiceUser(AGENT_SERVICE_NAME, strWorkingUser, strWorkingUserPwd);
        if (!bRet) {
            COMMLOG(OS_LOG_ERROR, "Change user of service \"%s\" failed.", AGENT_SERVICE_NAME);
            iRet = MP_FAILED;
        }
    } else {
        COMMLOG(OS_LOG_ERROR, "Unsupported param %s", strOperator.c_str());
        return MP_FAILED;
    }

    return iRet;
}

// Function Name: ProcessMonitorService
// Description  : 通过操作码响应不同monitor服务处理函数
mp_int32 ProcessMonitorService(mp_string strOperator, mp_string strWorkingUser, mp_string strWorkingUserPwd)
{
    mp_int32 iRet = MP_SUCCESS;
    mp_bool bRet = MP_FALSE;
    if (!strWorkingUser.empty()) {
        if (GetUserPwd(strWorkingUserPwd) != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "Get working user pwd failed!");
            return MP_FAILED;
        }
    }
    // 安装monitor服务
    if (strcmp(strOperator.c_str(), INSTALL_OPERATOR.c_str()) == 0) {
        COMMLOG(OS_LOG_INFO, "Install \"%s\" Service.", MONITOR_EXEC_NAME);
        mp_string strExecName = MONITOR_EXEC_NAME + ".exe";
        mp_string strBinPath = CPath::GetInstance().GetBinFilePath(strExecName);
        strBinPath = CMpString::BlankComma(strBinPath);
        strBinPath = strBinPath + " -s";
        bRet = CWinServiceHanlder::InstallService(strBinPath, MONITOR_SERVICE_NAME, strWorkingUser, strWorkingUserPwd);
        if (!bRet) {
            COMMLOG(OS_LOG_ERROR, "Install service \"%s\" failed.", MONITOR_SERVICE_NAME);
            iRet = MP_FAILED;
        }
    } else if (strcmp(strOperator.c_str(), UNINSTALL_OPERATOR.c_str()) == 0) {  // 卸载monitor服务
        COMMLOG(OS_LOG_INFO, "Uninstall \"%s\" Service.", MONITOR_EXEC_NAME);
        bRet = CWinServiceHanlder::UninstallService(MONITOR_SERVICE_NAME);
        if (!bRet) {
            COMMLOG(OS_LOG_ERROR, "Uninstall service \"%s\" failed.", MONITOR_SERVICE_NAME);
            iRet = MP_FAILED;
        }
    } else if (strcmp(strOperator.c_str(), CHANGE_USER_OPERATOR.c_str()) == 0) {
        COMMLOG(OS_LOG_INFO, "Change user of \"%s\" Service.", MONITOR_SERVICE_NAME);
        bRet = CWinServiceHanlder::ChangeServiceUser(MONITOR_SERVICE_NAME, strWorkingUser, strWorkingUserPwd);
        if (!bRet) {
            COMMLOG(OS_LOG_ERROR, "Change user of service \"%s\" failed.", MONITOR_SERVICE_NAME);
            iRet = MP_FAILED;
        }
    } else {
        COMMLOG(OS_LOG_ERROR, "Unsupported param %s", strOperator.c_str());
        return MP_FAILED;
    }

    return iRet;
}

// Function Name: ProcessNginxService
// Description  : 通过操作码响应不同nginx服务处理函数
mp_int32 ProcessNginxService(mp_string strOperator, mp_string strWorkingUser, mp_string strWorkingUserPwd)
{
    mp_int32 iRet = MP_SUCCESS;
    mp_bool bRet = MP_FALSE;
    if (!strWorkingUser.empty()) {
        if (GetUserPwd(strWorkingUserPwd) != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "Get working user pwd failed!");
            return MP_FAILED;
        }
    }
    // 启动nginx服务
    if (strcmp(strOperator.c_str(), RUN_OPERATOR.c_str()) == 0) {
        COMMLOG(OS_LOG_INFO, "Run Nginx Service.");
        iRet = RunNginxService();
        if (iRet != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "Run Nginx service failed, Ret = %d.", iRet);
        }
    } else if (strcmp(strOperator.c_str(), INSTALL_OPERATOR.c_str()) == 0) { // 安装nginx服务
        COMMLOG(OS_LOG_INFO, "Install \"%s\" Service.", NGINX_SERVICE_NAME);
        mp_string strExecName = WIN_SERVICE_EXEC_NAME + ".exe";
        mp_string strBinPath = CPath::GetInstance().GetBinFilePath(strExecName);
        strBinPath = CMpString::BlankComma(strBinPath);
        strBinPath = strBinPath + " " + NGINX_AS_PARAM_NAME + " " + RUN_OPERATOR;
        bRet = CWinServiceHanlder::InstallService(strBinPath, NGINX_SERVICE_NAME, strWorkingUser, strWorkingUserPwd);
        if (!bRet) {
            COMMLOG(OS_LOG_ERROR, "Install service \"%s\" failed.", NGINX_SERVICE_NAME);
            iRet = MP_FAILED;
        }
    } else if (strcmp(strOperator.c_str(), CHANGE_USER_OPERATOR.c_str()) == 0) {
        COMMLOG(OS_LOG_INFO, "Change user of \"%s\" Service.", NGINX_SERVICE_NAME);
        bRet = CWinServiceHanlder::ChangeServiceUser(NGINX_SERVICE_NAME, strWorkingUser, strWorkingUserPwd);
        if (!bRet) {
            COMMLOG(OS_LOG_ERROR, "Change user of service \"%s\" failed.", NGINX_SERVICE_NAME);
            iRet = MP_FAILED;
        }
    } else { // 卸载nginx服务
        COMMLOG(OS_LOG_INFO, "Uninstall \"%s\" Service.", NGINX_SERVICE_NAME);
        bRet = CWinServiceHanlder::UninstallService(NGINX_SERVICE_NAME);
        if (!bRet) {
            COMMLOG(OS_LOG_ERROR, "Uninstall service \"%s\" failed.", NGINX_SERVICE_NAME);
            iRet = MP_FAILED;
        }
    }

    return iRet;
}

// Function Name: main
// Description  : windows服务主函数
mp_int32 main(mp_int32 argc, mp_char** argv)
{
    mp_string strWorkingUser;
    mp_string strWorkingUserPwd;

    mp_bool bFlag = (argc != WIN_SERVICE_PRARM_NUM && argc != WIN_SERVICE_PRARM_NUM - WINSERVICE_NUM_2 &&
        argc != WINSERVICE_NUM_4) || !CheckBinName(argv[1]) || !CheckOperate(argv[WINSERVICE_NUM_2]);
    if (bFlag) {
        printf(
            "Usage: [path]winservice.exe rdagent|nginx|monitor run|install|uninstall|changeUser [work_user password]");
        return MP_FAILED;
    }

    if (argc > WINSERVICE_NUM_3) {
        strWorkingUser = argv[WINSERVICE_NUM_3];
    }

    // 初始化WinService路径
    mp_int32 iRet = CPath::GetInstance().Init(argv[0]);
    if (iRet != MP_SUCCESS) {
        printf("Init winservice path failed.\n");
        return iRet;
    }

    // 初始化配置文件模块
    iRet = CConfigXmlParser::GetInstance().Init(CPath::GetInstance().GetConfFilePath(AGENT_XML_CONF));
    if (iRet != MP_SUCCESS) {
        printf("Init conf file %s failed.\n", AGENT_XML_CONF.c_str());
        return iRet;
    }

    // 初始化日志模块
    CLogger::GetInstance().Init(WIN_SERVICE_LOG_NAME.c_str(), CPath::GetInstance().GetLogPath());

    // 初始化KMC
    iRet = InitCrypt(KMC_ROLE_TYPE_MASTER);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "InitKMC failed!");
        return iRet;
    }
    COMMLOG(OS_LOG_INFO, "Init kmc success.");

    if (strcmp(argv[1], AGENT_EXEC_NAME.c_str()) == 0) {
        iRet = ProcessAgentService(argv[WINSERVICE_NUM_2], strWorkingUser, strWorkingUserPwd);
    } else if (strcmp(argv[1], MONITOR_EXEC_NAME.c_str()) == 0) {
        iRet = ProcessMonitorService(argv[WINSERVICE_NUM_2], strWorkingUser, strWorkingUserPwd);
    } else {
        iRet = ProcessNginxService(argv[WINSERVICE_NUM_2], strWorkingUser, strWorkingUserPwd);
    }

    // 程序即将退出，此处不判断返回值
    (mp_void) FinalizeCrypt();
    return iRet;
}

WinService& WinService::GetInstace()
{
    static WinService WinService;
    return WinService;
}

SERVICE_STATUS_HANDLE& WinService::GetServiceHandle()
{
    return hServiceStatus;
}

mp_void WinService::InitServiceHandle()
{
    hServiceStatus = RegisterServiceCtrlHandlerExW((LPWSTR)NGINX_SERVICE_NAME.c_str(), NginxServiceHandler, NULL);
}

WinService::WinService()
{
}

#endif
