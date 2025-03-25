#include "tools/agentcli/ShowStatus.h"
#include "common/Defines.h"
#include "common/Path.h"
#include "common/File.h"
#include "common/AppVersion.h"
#include "common/Log.h"
#include "common/CSystemExec.h"
using namespace std;
/* ------------------------------------------------------------
Description  : 各进程的运行状态
Return       : MP_SUCCESS -- 成功
------------------------------------------------------------- */
mp_int32 ShowStatus::Handle()
{
#ifndef WIN32
    // 打印编译时间
    printf("Compile at  : %s\n", COMPILE_TIME.c_str());
#endif
    // 打印版本号信息
    printf("Version     : %s\n", AGENT_VERSION.c_str());
    printf("Build Number: %s\n", AGENT_BUILD_NUM.c_str());
    printf("UpdateVersion:%lld\n", AGENT_UPDATE_VERSION);

    // 打印rdagent运行状态
    mp_bool bStartted = IsStartted(PROCESS_RDAGENT);
    if (bStartted) {
        printf("rdagent     : Running\n");
    } else {
        printf("rdagent     : Stopped\n");
    }

    // 打印nginx运行状态
    bStartted = IsStartted(PROCESS_NGINX);
    if (bStartted) {
        printf("nginx       : Running\n");
    } else {
        printf("nginx       : Stopped\n");
    }

    // 打印monitor运行状态
    bStartted = IsStartted(PROCESS_MONITOR);
    if (bStartted) {
        printf("monitor     : Running\n");
    } else {
        printf("monitor     : Stopped\n");
    }
    return MP_SUCCESS;
}
/* ------------------------------------------------------------
Description  : rdagent运行状态
Return       : MP_TRUE -- 成功
               非MP_TRUE -- 失败
------------------------------------------------------------- */
#ifdef WIN32
mp_bool ShowStatus::IsStartted(PROCESS_TYPE eType)
{
    mp_string strServiceName;
    switch (eType) {
        case PROCESS_RDAGENT:
            strServiceName = AGENT_SERVICE_NAME;
            break;
        case PROCESS_NGINX:
            strServiceName = NGINX_SERVICE_NAME;
            break;
        case PROCESS_MONITOR:
            strServiceName = MONITOR_SERVICE_NAME;
            break;
        default:
            return MP_FALSE;
    }

    mp_string strCmd = "sc query " + strServiceName;
    vector<mp_string> vecRlt;
    // strCmd无注入
    mp_int32 iRet = CSystemExec::ExecSystemWithEcho(strCmd, vecRlt);
    if (iRet != MP_SUCCESS || vecRlt.empty()) {
        COMMLOG(OS_LOG_ERROR, "sc query %s failed.", strServiceName.c_str());
        return MP_FALSE;
    }

    if (vecRlt.front().find(RUNNING_TAG) != mp_string::npos) {
        return MP_TRUE;
    } else {
        return MP_FALSE;
    }
}
#else
mp_bool ShowStatus::IsStartted(PROCESS_TYPE eType)
{
    mp_string strPidFilePath;
    mp_string strProcessName;
    switch (eType) {
        case PROCESS_RDAGENT:
            strPidFilePath = CPath::GetInstance().GetLogFilePath(AGENT_PID);
            strProcessName = AGENT_EXEC_NAME;
            break;
        case PROCESS_NGINX:
            strPidFilePath = CPath::GetInstance().GetNginxLogsFilePath(NGINX_PID);
            strProcessName = NGINX_EXEC_NAME;
            break;
        case PROCESS_MONITOR:
            strPidFilePath = CPath::GetInstance().GetLogFilePath(MONITOR_PID);
            strProcessName = MONITOR_EXEC_NAME;
            break;
        default:
            return MP_FALSE;
    }

    // 从pid文件读取进程号
    vector<mp_string> vecRlt;
    mp_string strPid = " ";
    if (CMpFile::ReadFile(strPidFilePath, vecRlt) == MP_SUCCESS && !vecRlt.empty()) {
        strPid = vecRlt.front();
    }
    // strCmd 无注入
    mp_string strCmd = "ps -aef | grep '" + strPid + "' | grep " + strProcessName +
                       " | grep -v grep | grep -v gdb | grep -v vi | grep -v tail";
    if (CSystemExec::ExecSystemWithoutEcho(strCmd) != MP_SUCCESS) {
        return MP_FALSE;
    } else {
        return MP_TRUE;
    }
}
#endif

mp_void ShowStatus::ShowSvn()
{
    mp_string svnPath = CPath::GetInstance().GetConfFilePath(SVN_CONF);
    vector<mp_string> vecs;

    mp_int32 iRet = CMpFile::ReadFile(svnPath, vecs);
    if (iRet != MP_SUCCESS || vecs.size() != 1 || vecs.begin()->size() == 0) {
        COMMLOG(OS_LOG_INFO, "read %s failed.", SVN_CONF.c_str());
        printf("SVN         : Unknown\n");
    } else {
        printf("SVN         : %s\n", vecs.begin()->c_str());
    }
}
