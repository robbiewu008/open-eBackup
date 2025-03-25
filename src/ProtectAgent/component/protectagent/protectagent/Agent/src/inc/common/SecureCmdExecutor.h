#ifndef AGENT_SECURE_CMD_EXECUTOR_H
#define AGENT_SECURE_CMD_EXECUTOR_H
#include "common/CSystemExec.h"

class SecureCmdExecutor {
public:
    AGENT_API static mp_int32 ExecuteWithoutEcho(const mp_string& cmdTemplate,
        const std::vector<mp_string>& templateParam, mp_bool bNeedRedirect = MP_TRUE);
    AGENT_API static mp_int32 ExecuteWithEcho(const mp_string& cmdTemplate,
        const std::vector<mp_string>& templateParam, std::vector<mp_string>& echoOutput,
        mp_bool bNeedRedirect = MP_TRUE);
#ifdef WIN32
    AGENT_API static mp_int32 ExecuteWithEchoWin(
        const mp_string& cmdTemplate, const std::vector<mp_string>& templateParam,
        const mp_string& strParam, std::vector<mp_string>& echoOutput);
#endif
#ifndef WIN32
    AGENT_API static mp_int32 ExecuteWithSecurityParam(const mp_string& cmdTemplate,
        const std::vector<mp_string>& templateParam, const std::vector<mp_string>& vecParam,
        mp_bool bNeedRedirect = MP_TRUE);
#endif
private:
    AGENT_API static mp_int32 CheckTemplateParamSecurity(const std::vector<mp_string>& templateParam);
    AGENT_API static mp_int32 FormatCmd(const mp_string& cmdTemplate, const std::vector<mp_string>& templateParam,
        mp_string& outCmdStr);
};

#endif  // __AGENT_SECURE_CMD_EXECUTOR_H__
