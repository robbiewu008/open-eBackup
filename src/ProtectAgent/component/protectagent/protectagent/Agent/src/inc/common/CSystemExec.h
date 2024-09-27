#ifndef AGENT_SYSTEM_EXEC_H
#define AGENT_SYSTEM_EXEC_H

#include "common/Types.h"
#include "common/Defines.h"
#include "common/CMpThread.h"
#include <vector>

static const mp_int32 MAX_PATH_SIZE = 1024;

class CSystemExec {
public:
    // 系统调用
    AGENT_API static mp_int32 ExecSystemWithoutEcho(const mp_string& strCommand, mp_bool bNeedRedirect = MP_TRUE);
    AGENT_API static mp_int32 ExecSystemWithoutEcho(
        const mp_string& strCommand, const mp_string& strEnv, mp_bool bNeedRedirect = MP_TRUE);
    AGENT_API static mp_int32 ExecSystemWithEcho(
        const mp_string& strCommand, std::vector<mp_string>& strEcho, mp_bool bNeedRedirect = MP_TRUE);
#ifdef WIN32
    AGENT_API static mp_int32 ExecSystemWithStdWin(
        const mp_string& strCommand, const mp_string& strParam, std::vector<mp_string>& vecOutput);
#endif
#ifndef WIN32
    AGENT_API static mp_int32 ExecSystemWithSecurityParam(
        const mp_string& strCommand, const std::vector<mp_string>& vecParam,
        mp_bool bNeedRedirect = MP_TRUE,  mp_string resultFile = "");
#endif

private:
#ifdef WIN32
    AGENT_API static mp_int32 ExecSystemWithoutEchoWin(
        const mp_string& strLogCmd, const mp_string& strCommand, mp_bool bNeedRedirect = MP_TRUE);
    AGENT_API static mp_int32 ExecSystemWithoutEchoEnvWin(
        const mp_string& strLogCmd, const mp_string& strCommand, mp_string strEnv, mp_bool bNeedRedirect = MP_TRUE);
    AGENT_API static mp_int32 ExecSystemWithEchoWin(
        const mp_string& strLogCmd, const mp_string& strCommand, std::vector<mp_string>& strEcho,
        mp_bool bNeedRedirect = MP_TRUE);
    AGENT_API static mp_int32 CreateProcPipe(
        HANDLE& hStdInputRd, HANDLE& hStdInputWr, DWORD handleType);
    AGENT_API static mp_int32 CreateChildProcess(
        HANDLE& hStdInputRd, HANDLE& hStdOutputWr, const mp_string& strCommand, PROCESS_INFORMATION& piProcInfo);
    AGENT_API static void WriteToPipe(
        HANDLE& hStdInputWr, std::vector<mp_string>& vecStdin);
    AGENT_API static void ReadFromPipe(
        HANDLE& hStdOutputRd, std::vector<mp_string>& vecStdout);
#else
    AGENT_API static mp_int32 ExecSystemWithoutEchoNoWin(
        const mp_string& strLogCmd, const mp_string& strCommand, mp_bool bNeedRedirect = MP_TRUE);
    AGENT_API static mp_int32 ExecSystemWithoutEchoEnvNoWin(
        const mp_string& strLogCmd, const mp_string& strCommand, mp_string strEnv, mp_bool bNeedRedirect = MP_TRUE);
    AGENT_API static mp_int32 ExecSystemWithEchoNoWin(
        const mp_string& strLogCmd, const mp_string& strCommand, std::vector<mp_string>& strEcho,
        mp_bool bNeedRedirect = MP_TRUE);
#endif
};

#endif  // __AGENT_SYSTEM_EXEC_H__
