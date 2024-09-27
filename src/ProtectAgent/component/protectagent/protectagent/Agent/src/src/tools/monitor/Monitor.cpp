#include "tools/monitor/Monitor.h"
#include "tools/monitor/AbnormalHandler.h"
#include "common/Path.h"
#include "common/Log.h"
#include "common/ConfigXmlParse.h"
#include "securecom/CryptAlg.h"
#include "securecom/SecureUtils.h"
#include "common/Utils.h"

#ifndef WIN32
#include "common/StackTracer.h"
#endif

#ifdef WIN32
#include "common/ServiceHandler.h"
const mp_uchar MONITOR_NUM_2 = 2;
#endif

#ifndef WIN32

#else
/* ------------------------------------------------------------
Function Name: MonitorServiceHandler
Description  : monitor windows服务处理函数，对停止服务操作做出响应
Others       :-------------------------------------------------------- */
DWORD WINAPI MonitorServiceHandler(DWORD request, DWORD dwEventType, LPVOID lpEventData, LPVOID lpContext)
{
    LOGGUARD("");
    switch (request) {
        case SERVICE_CONTROL_STOP:
            CWinServiceHanlder::UpdateServiceStatus(WinServerStatus::GetInstace().GetServiceHandle(),
                SERVICE_STOPPED, START_SERVICE_TIMEOUT);
            Monitor::GetInstace().SetExitFlag(MP_TRUE);
            break;
        default:
            break;
    }

    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Function Name: MonitorServiceMain
Description  : monitor windows服务入口函数
Others       :-------------------------------------------------------- */
mp_void WINAPI MonitorServiceMain(DWORD dwArgc, LPTSTR* lpszArgv)
{
    LOGGUARD("");
    // 注册服务处理函数
    WinServerStatus::GetInstace().InitServiceHandle();
    if (!WinServerStatus::GetInstace().GetServiceHandle()) {
        COMMLOG(OS_LOG_ERROR, "Registe monitor service failed.");
        return;
    }

    // 设置运行状态
    CWinServiceHanlder::UpdateServiceStatus(WinServerStatus::GetInstace().GetServiceHandle(),
        SERVICE_RUNNING, START_SERVICE_TIMEOUT);
}

/* ------------------------------------------------------------
Function Name: MonitorServiceMain
Description  : 注册monitor windows服务入口函数
Others       :-------------------------------------------------------- */
mp_int32 RunMonitorService()
{
    LOGGUARD("");
    // 注册服务响应函数
    SERVICE_TABLE_ENTRY st[] = {{(LPSTR)MONITOR_SERVICE_NAME.c_str(), MonitorServiceMain}, {NULL, NULL}};

    mp_bool bRet = StartServiceCtrlDispatcher(st);
    if (!bRet) {
        COMMLOG(OS_LOG_ERROR, "StartServiceCtrlDispatcher failed, errorno[%d].", GetLastError());
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

#endif

/* ------------------------------------------------------------
Function Name: main
Description  : monitor进程主函数
Others       :-------------------------------------------------------- */
mp_int32 main(mp_int32 argc, mp_char** argv)
{
    (mp_void)argc;

#ifndef WIN32
    StackTracer stackTracer;
#endif

    // 初始化Monitor路径
    mp_int32 iRet = CPath::GetInstance().Init(argv[0]);
    if (iRet != MP_SUCCESS) {
        printf("Init monitor path failed.\n");
        return iRet;
    }

    // 初始化配置文件模块
    iRet = CConfigXmlParser::GetInstance().Init(CPath::GetInstance().GetConfFilePath(AGENT_XML_CONF));
    if (iRet != MP_SUCCESS) {
        printf("Init conf file %s failed.\n", AGENT_XML_CONF.c_str());
        return iRet;
    }

    // 初始化日志模块
    CLogger::GetInstance().Init(MONITOR_LOG_NAME.c_str(), CPath::GetInstance().GetLogPath());

    SecureCom::CryptoThreadSetup();

    // 初始化kmc
    iRet = InitCrypt(KMC_ROLE_TYPE_MASTER);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Init crypt failed, iRet = %d", iRet);
        return iRet;
    }

    // 初始化异常处理模块
    iRet = AbnormalHandler::GetInstance().Init();
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Init AbnormalHandler failed, iRet = %d", iRet);
        return iRet;
    }

#ifdef WIN32
    if (argc == MONITOR_NUM_2 && strcmp(argv[1], "-s") == 0) {
        iRet = RunMonitorService();
        if (iRet != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "Run monitor service failed, iRet = %d", iRet);
            return iRet;
        }
    }
#endif

    for (;;) {
        if (Monitor::GetInstace().GetExitFlag()) {
            exit(0);
        }
        DoSleep(MONITOR_SLEEP_TIME);
    }

    SecureCom::CryptoThreadCleanup();
}
