/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 *
 * @file SecureCmdExecutor.cpp
 * @brief  The implemention about security system call
 * @version 1.0
 * @date 2023-09-07
 * @author wangyunlong w30045225
 */

#include "common/SecureCmdExecutor.h"
#include "common/CSystemExec.h"
#include "common/Utils.h"
#include "common/Log.h"

using namespace std;
namespace {
    const static mp_string CMD_FMT_STR = "%s";
}

/*
@Function Name: ExecuteWithoutEcho
@Description  : 执行安全的系统调用，不获取回显信息
@Parameter cmdTemplate: 命令模板，例如"ls -l %s | grep 'sth'"
@Parameter templateParam: 命令模板参数，例如{scriptPath}
@Parameter bNeedRedirect，默认为true，表示需要将系统执行结果重定向到日志文件，
           可以显示设为false，向echo等命令无需将执行结果重定向到日志文件
*/
mp_int32 SecureCmdExecutor::ExecuteWithoutEcho(const mp_string& cmdTemplate,
    const std::vector<mp_string>& templateParam, mp_bool bNeedRedirect)
{
    mp_string cmdStr;
    if (FormatCmd(cmdTemplate, templateParam, cmdStr) == MP_FAILED) {
        return MP_FAILED;
    }
    return CSystemExec::ExecSystemWithoutEcho(cmdStr, bNeedRedirect);
}

/*
@Function Name: ExecuteWithEcho
@Description  : 执行安全的系统调用，获取回显信息
@Parameter cmdTemplate: 命令模板，例如"ls -l %s | grep 'sth'"
@Parameter templateParam: 命令模板参数，例如{scriptPath}
@Parameter echoOutput: 回显内容
@Parameter bNeedRedirect，默认为true，表示需要将系统执行结果重定向到日志文件，
           可以显示设为false，向echo等命令无需将执行结果重定向到日志文件
*/
mp_int32 SecureCmdExecutor::ExecuteWithEcho(
    const mp_string& cmdTemplate, const std::vector<mp_string>& templateParam,
    std::vector<mp_string>& echoOutput, mp_bool bNeedRedirect)
{
    mp_string cmdStr;
    if (FormatCmd(cmdTemplate, templateParam, cmdStr) == MP_FAILED) {
        return MP_FAILED;
    }
    return CSystemExec::ExecSystemWithEcho(cmdStr, echoOutput, bNeedRedirect);
}
#ifdef WIN32
/*
@Function Name: ExecuteWithEchoWin
@Description  : 执行安全的系统调用，带安全参数输入
@Parameter cmdTemplate: 命令模板，例如"ls -l %s | grep 'sth'"
@Parameter templateParam: 命令模板参数，例如{scriptPath}
@Parameter strParam: 输入的参数
@Parameter echoOutput，回显内容
*/
mp_int32 SecureCmdExecutor::ExecuteWithEchoWin(
    const mp_string& cmdTemplate, const std::vector<mp_string>& templateParam,
    const mp_string& strParam, std::vector<mp_string>& echoOutput)
{
    mp_string cmdStr;
    if (FormatCmd(cmdTemplate, templateParam, cmdStr) == MP_FAILED) {
        return MP_FAILED;
    }
    return CSystemExec::ExecSystemWithStdWin(cmdStr, strParam, echoOutput);
}
#endif

#ifndef WIN32
/*
@Function Name: ExecuteWithSecurityParam
@Description  : 执行安全的系统调用，带安全参数输入
@Parameter cmdTemplate: 命令模板，例如"ls -l %s | grep 'sth'"
@Parameter templateParam: 命令模板参数，例如{scriptPath}
@Parameter vecParam: 输入的参数
@Parameter bNeedRedirect，默认为true，表示需要将系统执行结果重定向到日志文件，
           可以显示设为false，向echo等命令无需将执行结果重定向到日志文件
*/
mp_int32 SecureCmdExecutor::ExecuteWithSecurityParam(
    const mp_string& cmdTemplate, const std::vector<mp_string>& templateParam,
    const std::vector<mp_string>& vecParam, mp_bool bNeedRedirect)
{
    mp_string cmdStr;
    if (FormatCmd(cmdTemplate, templateParam, cmdStr) == MP_FAILED) {
        return MP_FAILED;
    }
    return CSystemExec::ExecSystemWithSecurityParam(cmdStr, vecParam, bNeedRedirect);
}
#endif


mp_int32 SecureCmdExecutor::CheckTemplateParamSecurity(const std::vector<mp_string>& templateParam)
{
    for (auto iter = templateParam.begin(); iter != templateParam.end(); ++iter) {
        if (CheckCmdDelimiter(*iter) != MP_SUCCESS) {
            ERRLOG("Check cmd parameter security failed.");
            return MP_FAILED;
        }
    }
    return MP_SUCCESS;
}

mp_int32 SecureCmdExecutor::FormatCmd(const mp_string& cmdTemplate, const std::vector<mp_string>& templateParam,
    mp_string& outCmdStr)
{
    if (SecureCmdExecutor::CheckTemplateParamSecurity(templateParam) == MP_FAILED) {
        return MP_FAILED;
    }
    outCmdStr = mp_string(cmdTemplate);
    int findIdx;
    int fmtCnt = 0;
    for (auto iter = templateParam.begin(); iter != templateParam.end(); ++iter) {
        if ((findIdx = outCmdStr.find(CMD_FMT_STR)) != -1) {
            outCmdStr.replace(findIdx, CMD_FMT_STR.size(), *iter);
            ++fmtCnt;
        }
    }
    return fmtCnt;
}