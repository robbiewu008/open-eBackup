#include <gperftools/malloc_extension.h>
#endif

#include "agent/AgentExitFlag.h"
#include "agent/TaskPool.h"
#include "agent/Communication.h"
#include "agent/Authentication.h"
#include "agent/FTExceptionHandle.h"
#include "agent/CheckCertValidity.h"
#include "host/CheckConnectStatus.h"
#include "message/tcp/TCPClientHandler.h"
#include "common/AppVersion.h"
#include "common/Log.h"
#include "common/Utils.h"
#include "common/Path.h"
#include "securecom/UniqueId.h"
#include "common/MpString.h"
#include "common/ConfigXmlParse.h"
#include "common/CSystemExec.h"
#include "securecom/SecureUtils.h"
#include "securecom/CryptAlg.h"
#include "securec.h"
#ifdef WIN32
#include "common/ServiceHandler.h"
#else
#include "common/StackTracer.h"
#endif

namespace {
    const int SLEEP_1000_MS = 1000;
    const int RELEASE_MEM_FRE = 10;

/* ------------------------------------------------------------
Description  : 在线手册
------------------------------------------------------------- */
mp_void ShowOnlineManual()
{
    printf("rdagent -v : show version.\n");
    printf("rdagent -h : show online manual.\n");
}

#ifdef SUPPORT_SSL
mp_void initSSL()
{
    SSL_library_init();            // 初始化OpenSSL
    OpenSSL_add_all_algorithms();  // 载入所有SSL算法
    SSL_load_error_strings();      // 载入所有SSL错误信息
}
#endif

/* ------------------------------------------------------------
Description  : agent初始化
Output       : pszFullBinPath---全路径，taskPool---任务池
Return       :  MP_SUCCESS---初始化成功
                iRet---对应的错误码
------------------------------------------------------------- */
mp_int32 AgentInit(const mp_string& pszFullBinPath, TaskPool& taskPool)
{
    mp_int32 iRet = InitCommonModules(pszFullBinPath);
    if (iRet != MP_SUCCESS) {
        return iRet;
    }

    // 初始化通信模块
    iRet = Communication::GetInstance().Init();
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Init Communication failed.");
        return iRet;
    }

#ifdef SUPPORT_SSL
    initSSL();
#endif

    // 初始化KMC
    iRet = InitCrypt(KMC_ROLE_TYPE_MASTER);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Init KMC failed.");
        return iRet;
    }

    // 初始化task
    iRet = taskPool.Init();
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Init task pool failed.");
        return iRet;
    }

    // 初始化冻结解冻处理
    iRet = FTExceptionHandle::GetInstance().Init();
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Init ftexception handle failed.");
        return iRet;
    }

    // 初始化备份场景的信息
    iRet = TCPClientHandler::GetInstance().Init();
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Init backup model failed.");
        return iRet;
    }
    // 初始化Agent连通性检查
    iRet = CheckConnectStatus::GetInstance().Init();
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Start check connectivity failed.");
        return iRet;
    }

    // init check cert validity
    iRet = CheckCertValidity::GetInstance().Init();
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Start check certificate validity failed.");
        return iRet;
    }
#if (defined LINUX) && (!defined ENABLE_TSAN)
    // tcmalloc 释放频率
    MallocExtension::instance()->SetMemoryReleaseRate(RELEASE_MEM_FRE);
#endif
    return MP_SUCCESS;
}
}

#ifdef WIN32
class WinServerStatus;
/* ------------------------------------------------------------
Description  : agent服务处理
Input        : request---服务请求类型
Return       :  MP_SUCCESS---成功
------------------------------------------------------------- */
DWORD WINAPI AgentServiceHandler(DWORD request, DWORD dwEventType, LPVOID lpEventData, LPVOID lpContext)
{
    switch (request) {
        case SERVICE_CONTROL_STOP:
            CWinServiceHanlder::UpdateServiceStatus(WinServerStatus::GetInstace().GetServiceHandle(),
                SERVICE_STOPPED, START_SERVICE_TIMEOUT);
            AgentExitFlag::GetInstace().SetExitFlag(MP_TRUE);
            break;
        default:
            break;
    }
    return MP_SUCCESS;
}
/* ------------------------------------------------------------
Description  : agent服务
------------------------------------------------------------- */
mp_void WINAPI AgentServiceMain(DWORD dwArgc, LPTSTR* lpszArgv)
{
    LOGGUARD("");
    SERVICE_STATUS_HANDLE sHandle = RegisterServiceCtrlHandlerExW((LPWSTR)AGENT_SERVICE_NAME.c_str(),
        AgentServiceHandler, NULL);
    WinServerStatus::GetInstace().InitServiceHandle(sHandle);
    if (!WinServerStatus::GetInstace().GetServiceHandle()) {
        COMMLOG(OS_LOG_CRI, "Registe agent service failed.");
        return;
    }

    CWinServiceHanlder::UpdateServiceStatus(WinServerStatus::GetInstace().GetServiceHandle(),
        SERVICE_RUNNING, START_SERVICE_TIMEOUT);
}
/* ------------------------------------------------------------
Description  : 运行agent服务
Return       :  MP_SUCCESS---成功
                MP_FAILED---失败，启动控制调度服务失败
------------------------------------------------------------- */
mp_int32 RunAgentService()
{
    LOGGUARD("");
    SERVICE_TABLE_ENTRY st[] = { {(LPSTR)AGENT_SERVICE_NAME.c_str(), AgentServiceMain}, {NULL, NULL} };

    mp_bool bRet = StartServiceCtrlDispatcher(st);
    if (!bRet) {
        COMMLOG(OS_LOG_ERROR, "StartServiceCtrlDispatcher failed, errorno[%d].", GetLastError());
        return MP_FAILED;
    }

    return MP_SUCCESS;
}
#endif
/* ------------------------------------------------------------
Description  : agent 主函数
Return       :  MP_SUCCESS---成功
                iRet---失败，返回对应错误码
------------------------------------------------------------- */
mp_int32 main(mp_int32 argc, mp_char** argv)
{
    mp_bool bService = MP_FALSE;

    if (argc > 1) {
        // 当前只支持-v和-s
        mp_char* pszOp = argv[1];
        if (strcmp(pszOp, "-v") == 0) {
            AgentVersion();
            return MP_SUCCESS;
        } else if (strcmp(pszOp, "-s") == 0) {
            bService = MP_TRUE;
        } else {
            ShowOnlineManual();
            return MP_SUCCESS;
        }
    }

#ifndef WIN32
    StackTracer stackTracer;
#endif

    TaskPool taskPool;
    mp_int32 iRet = AgentInit(argv[0], taskPool);
    if (iRet != MP_SUCCESS) {
        return iRet;
    }

    SecureCom::CryptoThreadSetup();

#ifdef WIN32
    if (bService) {
        iRet = RunAgentService();
        if (iRet != MP_SUCCESS) {
            printf("Run agent windows service failed.\n");
            return iRet;
        }
    }
#else
    if (bService) {
        printf("Not windows isn't support running as service.\n");
    }
#endif

    for (;;) {
        if (AgentExitFlag::GetInstace().GetExitFlag()) {
            exit(0);
        }
        DoSleep(SLEEP_1000_MS);
    }

    SecureCom::CryptoThreadCleanup();
#ifndef WIN32
    (mp_void)ChangeGmonDir();  // change profile out put dir
#endif

    return iRet;
}
