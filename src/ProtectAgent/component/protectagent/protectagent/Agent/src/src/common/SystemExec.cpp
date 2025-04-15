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
#ifndef WIN32
#include <csignal>
#include <libgen.h>
#include <sys/wait.h>
#endif
#include <sstream>
#include "securec.h"
#include "common/Log.h"
#include "common/Utils.h"
#include "common/Path.h"
#include "common/ErrorCode.h"
#include "common/CSystemExec.h"

using namespace std;
namespace {
const mp_int32 SYSTEM_EXEC_NUM_256 = 256;
const mp_int32 SYSTEM_EXEC_NUM_1000 = 1000;
#ifdef WIN32
const mp_int32 SYSTEM_EXEC_NUM_3600 = 3600;
const mp_int32 MAX_READ_BUF_SIZE = 4096;
#endif
}

/* ------------------------------------------------------------
Function Name: ExecSystemWithoutEcho
Description  : 执行系统调用，不获取回显信息
               bNeedRedirect，默认为true，表示需要将系统执行结果重定向到日志文件
               bNeedRedirect，可以显示设为false，向echo等命令无需将执行结果重定向到日志文件
Others       :-------------------------------------------------------- */
mp_int32 CSystemExec::ExecSystemWithoutEcho(const mp_string& strCommand, const mp_string& strEnv, mp_bool bNeedRedirect)
{
    mp_string strLogCmd;
    RemoveFullPathForLog(strCommand, strLogCmd);
#ifdef WIN32
    return ExecSystemWithoutEchoEnvWin(strLogCmd, strCommand, strEnv, bNeedRedirect);
#else
    return ExecSystemWithoutEchoEnvNoWin(strLogCmd, strCommand, strEnv, bNeedRedirect);
#endif
}

/* ------------------------------------------------------------
Function Name: ExecSystemWithoutEcho
Description  : 执行系统调用，不获取回显信息
               bNeedRedirect，默认为true，表示需要将系统执行结果重定向到日志文件
               bNeedRedirect，可以显示设为false，向echo等命令无需将执行结果重定向到日志文件
Others       :-------------------------------------------------------- */
mp_int32 CSystemExec::ExecSystemWithoutEcho(const mp_string& strCommand, mp_bool bNeedRedirect)
{
    mp_string strLogCmd;
    RemoveFullPathForLog(strCommand, strLogCmd);
#ifdef WIN32
    return ExecSystemWithoutEchoWin(strLogCmd, strCommand, bNeedRedirect);
#else
    return ExecSystemWithoutEchoNoWin(strLogCmd, strCommand, bNeedRedirect);
#endif
}

/* ------------------------------------------------------------
Function Name: ExecSystemWithEcho
Description  : 执行系统调用，获取回显信息
               bNeedRedirect，默认为true，表示需要将系统执行结果重定向到日志文件
               bNeedRedirect，可以显示设为false，向echo等命令无需将执行结果重定向到日志文件
Others       :-------------------------------------------------------- */
mp_int32 CSystemExec::ExecSystemWithEcho(const mp_string& strCommand, vector<mp_string>& strEcho, mp_bool bNeedRedirect)
{
    mp_string strLogCmd;
    RemoveFullPathForLog(strCommand, strLogCmd);
#ifdef WIN32
    return ExecSystemWithEchoWin(strLogCmd, strCommand, strEcho, bNeedRedirect);
#else
    return ExecSystemWithEchoNoWin(strLogCmd, strCommand, strEcho, bNeedRedirect);
#endif
}

#ifdef WIN32
mp_int32 CSystemExec::ExecSystemWithStdWin(const mp_string& strCommand, const mp_string& strParam,
    std::vector<mp_string>& vecOutput)
{
    LOGGUARD("");
    std::vector<mp_string> vecParam;
    CMpString::StrSplitEx(vecParam, strParam, NODE_COLON);
    HANDLE hStdInputRd;
    HANDLE hStdInputWr;
    mp_int32 iRet = CreateProcPipe(hStdInputRd, hStdInputWr, STD_INPUT_HANDLE);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Create write pipe failed.");
        return iRet;
    }

    HANDLE hStdOutputRd;
    HANDLE hStdOutputWr;
    iRet = CreateProcPipe(hStdOutputRd, hStdOutputWr, STD_OUTPUT_HANDLE);
    if (iRet != MP_SUCCESS) {
        CloseHandle(hStdInputRd);
        CloseHandle(hStdInputWr);
        ERRLOG("Create read pipe failed.");
        return iRet;
    }

    PROCESS_INFORMATION piProcInfo = {};
    ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));
    iRet = CreateChildProcess(hStdInputRd, hStdOutputWr, strCommand, piProcInfo);
    if (iRet != MP_SUCCESS) {
        CloseHandle(hStdInputRd);
        CloseHandle(hStdInputWr);
        CloseHandle(hStdOutputRd);
        CloseHandle(hStdOutputWr);
        ERRLOG("Create child process failed.");
        return iRet;
    }

    WriteToPipe(hStdInputWr, vecParam);
    ReadFromPipe(hStdOutputRd, vecOutput);
    DWORD exitCode = 0;
    GetExitCodeProcess(piProcInfo.hProcess, &exitCode);
    CloseHandle(piProcInfo.hProcess);
    CloseHandle(hStdInputRd);
    CloseHandle(hStdInputWr);
    CloseHandle(hStdOutputRd);
    CloseHandle(hStdOutputWr);
    iRet = (mp_int32)exitCode;
    DBGLOG("Execute script exit code is %d.", iRet);
    return iRet;
}
#endif

#ifndef WIN32
mp_int32 CSystemExec::ExecSystemWithSecurityParam(const mp_string& strCommand,
    const std::vector<mp_string>& vecParam, mp_bool bNeedRedirect, mp_string resFilePath)
{
    mp_int32 iErr;
    mp_char szErr[SYSTEM_EXEC_NUM_256] = {0};
    std::ostringstream oss;
    mp_string strLogFilePath;
    oss << strCommand << " " << vecParam.size();
    if (bNeedRedirect && resFilePath.empty()) {
        if (getuid() == 0) {
            strLogFilePath = CMpString::BlankComma(CPath::GetInstance().GetSlogFilePath(ROOT_EXEC_LOG_NAME));
        } else {
            strLogFilePath = CMpString::BlankComma(CPath::GetInstance().GetLogFilePath(AGENT_LOG_NAME));
        }
        oss << " 1>>" << strLogFilePath << " 2>&1";
    } else if (bNeedRedirect && !resFilePath.empty()) {
        oss << " 1>>" << resFilePath << " 2>&1";
    }

    DBGLOG("Exec cmd=%s.", oss.str().c_str());
    FILE* fStream = popen(oss.str().c_str(), "w");
    if (NULL == fStream) {
        iErr = GetOSError();
        ERRLOG("Popen call failed, errno[%d]:%s.", iErr, GetOSStrErr(iErr, szErr, sizeof(szErr)));
        return ERROR_COMMON_SYSTEM_CALL_FAILED;
    }

    mp_string strParam = CMpString::StrJoin(vecParam, NODE_COLON) + NODE_COLON;
    mp_int32 ret = fputs(strParam.c_str(), fStream);
    if (ret == EOF) {
        iErr = GetOSError();
        ERRLOG("Fputs excute failed, errno[%d]:%s.", iErr, GetOSStrErr(iErr, szErr, sizeof(szErr)));
        mp_int32 iRet = pclose(fStream);
        if (MP_FAILED == iRet) {
            iErr = GetOSError();
            ERRLOG("Pclose excute failed, errno[%d]:%s.", iErr, GetOSStrErr(iErr, szErr, sizeof(szErr)));
        }
        return ERROR_COMMON_SYSTEM_CALL_FAILED;
    }

    mp_int32 iRet = pclose(fStream);
    if (MP_FAILED == iRet) {
        iErr = GetOSError();
        ERRLOG("Pclose excute failed, errno[%d]:%s.", iErr, GetOSStrErr(iErr, szErr, sizeof(szErr)));
        return ERROR_COMMON_SYSTEM_CALL_FAILED;
    }

    // pclose返回的是shell最终状态，需要获取子进程的返回值
    // 还需要考虑WIFSIGNALED和WIFSTOPPED的影响
    if (WIFEXITED(iRet)) {
        return WEXITSTATUS(iRet);
    } else {
        iErr = GetOSError();
        ERRLOG("Sub process exit abnormally, errno[%d]:%s.", iErr, GetOSStrErr(iErr, szErr, sizeof(szErr)));
        return ERROR_COMMON_SYSTEM_CALL_FAILED;
    }
}
#endif

#ifdef WIN32
mp_int32 CSystemExec::ExecSystemWithoutEchoWin(const mp_string& strLogCmd,
    const mp_string& strCommand, mp_bool bNeedRedirect)
{
    mp_int32 iRet = MP_FAILED;
    // windows下多个进程会调用此函数，不能将执行结果重定向到某一个日志文件下
    COMMLOG(OS_LOG_DEBUG, "Command is %s", strLogCmd.c_str());

    STARTUPINFO stStartupInfo;
    PROCESS_INFORMATION stProcessInfo;
    DWORD dwCodeWin = 0;

    ZeroMemory(&stStartupInfo, sizeof(stStartupInfo));
    stStartupInfo.cb = sizeof(stStartupInfo);
    stStartupInfo.dwFlags = STARTF_USESHOWWINDOW;
    stStartupInfo.wShowWindow = SW_HIDE;

    ZeroMemory(&stProcessInfo, sizeof(stProcessInfo));
    // No module name (use command line).
    // Command line.
    // Process handle not inheritable.
    // Thread handle not inheritable.
    // Set handle inheritance to ISSP_FALSE.
    // No creation flags.
    // Use parent's environment block.
    // Use parent's starting directory.
    // Pointer to STARTUPINFO structure.
    // Pointer to PROCESS_INFORMATION structure.
    if (!CreateProcess(NULL, TEXT((LPTSTR)strCommand.c_str()), NULL, NULL, MP_FALSE,
        0, NULL, NULL, &stStartupInfo, &stProcessInfo)) {
        mp_int32 errCode = GetLastError();
        mp_char szErr[SYSTEM_EXEC_NUM_256] = {0};
        COMMLOG(OS_LOG_ERROR,
            "ExecSystemWithoutEchoWin:CreateProcess failed, errono[%d]: %s",
            errCode,
            GetOSStrErr(errCode, szErr, sizeof(szErr)));
        return iRet;
    }

    COMMLOG(OS_LOG_DEBUG, "ExecSystemWithoutEchoWin:Begin WaitForSingleObject");

    // time is 1000 hours
    if (WAIT_TIMEOUT == WaitForSingleObject(stProcessInfo.hProcess,
        SYSTEM_EXEC_NUM_1000 * SYSTEM_EXEC_NUM_3600 * SYSTEM_EXEC_NUM_1000)) {
        COMMLOG(OS_LOG_ERROR, "WaitForSingleObject timeout");
        iRet = MP_FAILED;
    } else {
        GetExitCodeProcess(stProcessInfo.hProcess, &dwCodeWin);
        iRet = dwCodeWin;
        COMMLOG(OS_LOG_DEBUG, "GetExitCodeProcess is %d", iRet);
    }

    COMMLOG(OS_LOG_DEBUG, "ExecSystemWithoutEchoWin:end WaitForSingleObject");
    // CodeDex误报，SECURE_CODING
    CloseHandle(stProcessInfo.hProcess);
    CloseHandle(stProcessInfo.hThread);
    COMMLOG(OS_LOG_DEBUG, "Leave ExecSystemWithoutEchoWin, command is %s", strLogCmd.c_str());
    return iRet;
}

mp_int32 CSystemExec::ExecSystemWithoutEchoEnvWin(
    const mp_string& strLogCmd, const mp_string& strCommand, mp_string strEnv, mp_bool bNeedRedirect)
{
    mp_int32 iRet = MP_FAILED;
    // windows下多个进程会调用此函数，不能将执行结果重定向到某一个日志文件下
    COMMLOG(OS_LOG_DEBUG, "Command is %s", Sensitive::WipeSensitive(strLogCmd, strLogCmd).c_str());
    TCHAR chNewEnv[MAX_PATH_SIZE];
    if (strEnv.length() > MAX_PATH_SIZE) {
        COMMLOG(OS_LOG_INFO, "Env file path size wrong.");
        return MP_FAILED;
    }
    memset_s(chNewEnv, MAX_PATH_SIZE, 0, MAX_PATH_SIZE);
    errno_t ret = strcpy_s(chNewEnv, MAX_PATH_SIZE, strEnv.c_str());
    if (ret != EOK) {
        COMMLOG(OS_LOG_INFO, "copy chNewEnv failed .");
        return MP_FAILED;
    }

    STARTUPINFO stStartupInfo;
    PROCESS_INFORMATION stProcessInfo;
    DWORD dwCode = 0;

    ZeroMemory(&stStartupInfo, sizeof(stStartupInfo));
    stStartupInfo.cb = sizeof(stStartupInfo);
    stStartupInfo.dwFlags = STARTF_USESHOWWINDOW;
    stStartupInfo.wShowWindow = SW_HIDE;
    ZeroMemory(&stProcessInfo, sizeof(stProcessInfo));

    // No module name (use command line).
    // Command line.
    // Process handle not inheritable.
    // Thread handle not inheritable.
    // Set handle inheritance to ISSP_FALSE.
    // No creation flags.
    // Use parent's environment block.
    // Use parent's starting directory.
    // Pointer to STARTUPINFO structure.
    // Pointer to PROCESS_INFORMATION structure.
    if (!CreateProcess(NULL, TEXT((LPTSTR)strCommand.c_str()), NULL, NULL, MP_FALSE,
        0, (LPVOID)chNewEnv, NULL, &stStartupInfo, &stProcessInfo)) {
        mp_int32 iErr = GetLastError();
        mp_char szErr[SYSTEM_EXEC_NUM_256] = {0};
        COMMLOG(OS_LOG_ERROR, "CreateProcess failed, errono[%d]: %s", iErr, GetOSStrErr(iErr, szErr, sizeof(szErr)));

        return iRet;
    }

    COMMLOG(OS_LOG_DEBUG, "ExecSystemWithoutEchoEnvWin：Begin WaitForSingleObject");

    if (WAIT_TIMEOUT == WaitForSingleObject(stProcessInfo.hProcess, SYSTEM_EXEC_NUM_1000 * SYSTEM_EXEC_NUM_3600)) {
        COMMLOG(OS_LOG_ERROR, "WaitForSingleObject timeout");
        iRet = MP_FAILED;
    } else {
        GetExitCodeProcess(stProcessInfo.hProcess, &dwCode);
        iRet = dwCode;
        COMMLOG(OS_LOG_DEBUG, "GetExitCodeProcess is %d", iRet);
    }

    COMMLOG(OS_LOG_DEBUG, "ExecSystemWithoutEchoEnvWin:end WaitForSingleObject");
    // CodeDex误报，SECURE_CODING
    CloseHandle(stProcessInfo.hProcess);
    CloseHandle(stProcessInfo.hThread);
    COMMLOG(OS_LOG_DEBUG, "Leave ExecSystemWithoutEchoEnvWin, command is %s",
        Sensitive::WipeSensitive(strLogCmd, strLogCmd).c_str());
    return iRet;
}

mp_int32 CSystemExec::ExecSystemWithEchoWin(
    const mp_string& strLogCmd, const mp_string& strCommand, vector<mp_string>& strEcho, mp_bool bNeedRedirect)
{
    mp_int32 iRet = MP_FAILED;
    SECURITY_ATTRIBUTES sa;
    HANDLE hRead, hWrite;

    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.lpSecurityDescriptor = NULL;
    sa.bInheritHandle = TRUE;
    if (!CreatePipe(&hRead, &hWrite, &sa, 0)) {  // 创建匿名管道
        iRet = GetLastError();
        COMMLOG(OS_LOG_DEBUG, "CreatePipe failed, iRet = %d", iRet);
        return iRet;
    }

    STARTUPINFO stStartupInfo;
    PROCESS_INFORMATION stProcessInfo;
    DWORD dwCode = 0;

    ZeroMemory(&stStartupInfo, sizeof(stStartupInfo));
    stStartupInfo.cb = sizeof(stStartupInfo);
    GetStartupInfo(&stStartupInfo);
    stStartupInfo.hStdError = hWrite;   // 把创建进程的标准错误输出重定向到管道输入
    stStartupInfo.hStdOutput = hWrite;  // 把创建进程的标准输出重定向到管道输入
    stStartupInfo.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
    stStartupInfo.wShowWindow = SW_HIDE;

    ZeroMemory(&stProcessInfo, sizeof(stProcessInfo));

    /* No module name (use command line). */
    /* Command line. */
    /* Process handle not inheritable. */
    /* Thread handle not inheritable. */
    /* Set handle inheritance to ISSP_FALSE. */
    /* No creation flags. */
    /* Use parent's environment block. */
    /* Use parent's starting directory. */
    /* Pointer to STARTUPINFO structure. */
    /* Pointer to PROCESS_INFORMATION structure. */
    if (!CreateProcess(NULL, TEXT((LPTSTR)strCommand.c_str()),  NULL, NULL, MP_TRUE, 0, NULL,
        NULL, &stStartupInfo, &stProcessInfo)) {
        iRet = GetLastError();
        COMMLOG(OS_LOG_DEBUG, "CreateProcess failed, iRet = %d", iRet);
        CloseHandle(hWrite);
        CloseHandle(hRead);
        return iRet;
    }
    // CodeDex误报，SECURE_CODING
    CloseHandle(hWrite);

    while (MP_TRUE) {
        mp_char tmpBuf[SYSTEM_EXEC_NUM_1000] = {0};
        DWORD bytesRead = 0;
        if (ReadFile(hRead, tmpBuf, SYSTEM_EXEC_NUM_1000, &bytesRead, NULL) == NULL) {
            break;
        }
        strEcho.push_back(tmpBuf);
    }

    CloseHandle(stProcessInfo.hProcess);
    CloseHandle(stProcessInfo.hThread);
    CloseHandle(hRead);
    COMMLOG(OS_LOG_DEBUG, "Leave ExecSystemWithEcho, command is %s", strLogCmd.c_str());
    return MP_SUCCESS;
}

mp_int32 CSystemExec::CreateProcPipe(HANDLE &hStdInputRd, HANDLE &hStdInputWr, DWORD handleType)
{
    SECURITY_ATTRIBUTES saAttr = {0};
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;
    saAttr.lpSecurityDescriptor = nullptr;
    if (!CreatePipe(&hStdInputRd, &hStdInputWr, &saAttr, 0)) { // 创建匿名管道
        mp_int32 retCode = GetLastError();
        mp_char szErr[SYSTEM_EXEC_NUM_256] = {0};
        ERRLOG("CreatePipe failed, errorCode=%d, %s", retCode, GetOSStrErr(retCode, szErr, sizeof(szErr)));
        return retCode;
    }

    mp_bool bRet = FALSE;
    if (handleType == STD_INPUT_HANDLE) {
        bRet = SetHandleInformation(hStdInputWr, HANDLE_FLAG_INHERIT, 0);
    } else if (handleType == STD_OUTPUT_HANDLE) {
        bRet = SetHandleInformation(hStdInputRd, HANDLE_FLAG_INHERIT, 0);
    } else {
        ERRLOG("Unknow handle type[%d]", handleType);
        return MP_FAILED;
    }
    if (!bRet) {
        mp_int32 retCode = GetLastError();
        mp_int32 iErr = GetLastError();
        mp_char szErr[SYSTEM_EXEC_NUM_256] = { 0 };
        ERRLOG("CreateProcess failed, errono[%d]: %s", iErr, GetOSStrErr(iErr, szErr, sizeof(szErr)));
        ERRLOG("Set handle info failed, errorCode=%d", retCode);
        return retCode;
    }
    return MP_SUCCESS;
}

mp_int32 CSystemExec::CreateChildProcess(HANDLE& hStdInputRd, HANDLE& hStdOutputWr, const mp_string& strCommand,
    PROCESS_INFORMATION& piProcInfo)
{
    DWORD dwCode = 0;
    STARTUPINFO siStartInfo = {0};
    ZeroMemory(&siStartInfo, sizeof(STARTUPINFO));
    siStartInfo.hStdError = hStdOutputWr;
    siStartInfo.hStdOutput = hStdOutputWr;
    siStartInfo.hStdInput = hStdInputRd;
    siStartInfo.dwFlags |= STARTF_USESTDHANDLES;
    if (!CreateProcess(nullptr, TEXT((mp_char *)strCommand.c_str()), nullptr, nullptr, TRUE, 0, nullptr, nullptr,
        &siStartInfo, &piProcInfo)) {
        int retCode = GetLastError();
        ERRLOG("Create child process failed, errorcode=%d.", retCode);
        return MP_FAILED;
    } else {
        /* piProcInfo.hProcess close outside */
        CloseHandle(piProcInfo.hThread);
        CloseHandle(hStdOutputWr);
        CloseHandle(hStdInputRd);
    }
    return MP_SUCCESS;
}

void CSystemExec::WriteToPipe(HANDLE& hStdInputWr, std::vector<mp_string>& vecStdin)
{
    std::vector<mp_string>::iterator iter = vecStdin.begin();
    mp_bool bSuccess = TRUE;
    DWORD dwWritten = 0;
    for (; iter != vecStdin.end(); ++iter) {
        std::string &strIn = *iter;
        bSuccess = WriteFile(hStdInputWr, strIn.c_str(), strIn.length(), &dwWritten, nullptr);
        ClearString(strIn);
        if (!bSuccess) {
            ERRLOG("Failed to write the input stream.bSuccess=%d", bSuccess);
            break;
        }
    }
    CloseHandle(hStdInputWr);
}

void CSystemExec::ReadFromPipe(HANDLE& hStdOutputRd, std::vector<mp_string>& vecStdout)
{
    DWORD dwRead = 0;
    mp_char chBuf[MAX_READ_BUF_SIZE] = {0};
    mp_bool bSuccess = FALSE;

    for (;;) {
        bSuccess = ReadFile(hStdOutputRd, chBuf, MAX_READ_BUF_SIZE, &dwRead, nullptr);
        if (!bSuccess || dwRead == 0) {
            break;
        }
        std::string str(chBuf, dwRead);
        vecStdout.push_back(str);
    }
    CloseHandle(hStdOutputRd);
}

#else
mp_int32 CSystemExec::ExecSystemWithoutEchoNoWin(const mp_string& strLogCmd,
    const mp_string& strCommand, mp_bool bNeedRedirect)
{
    // 在命令末尾添加重定向命令
    // 信息全部重定向到agent日志文件
    mp_string strLogFilePath;
    mp_int32 iErr;
    mp_char szErr[SYSTEM_EXEC_NUM_256] = {0};
    uid_t uid = getuid();
    strLogFilePath = CPath::GetInstance().GetLogFilePath(AGENT_LOG_NAME);
    if (uid == 0) {
        strLogFilePath = CPath::GetInstance().GetSlogFilePath(ROOT_EXEC_LOG_NAME);
    }

    strLogFilePath = CMpString::BlankComma(strLogFilePath);
    mp_string strNewCommand, strLogNewCmd;
    strNewCommand = bNeedRedirect ? (strCommand + " 1>>" + strLogFilePath + " 2>&1") : strCommand;
    strLogNewCmd = bNeedRedirect ? (strLogCmd + " 1>>" + strLogFilePath + " 2>&1") : strLogCmd;

    // Coverity&Fortify误报:FORTIFY.Command_Injection
    // 已经按修复建议增加命令分隔符判断CheckCmdDelimiter
    FILE* fStream = popen(strNewCommand.c_str(), "r");
    if (NULL == fStream) {
        iErr = GetOSError();
        COMMLOG(OS_LOG_ERROR, "popen call failed, errno[%d]:%s.", iErr, GetOSStrErr(iErr, szErr, sizeof(szErr)));
        return ERROR_COMMON_SYSTEM_CALL_FAILED;
    }

    while (!feof(fStream)) {
        mp_char tmpBuf[SYSTEM_EXEC_NUM_1000] = {0};
        mp_char* cRet = fgets(tmpBuf, sizeof(tmpBuf), fStream);
        if (NULL == cRet) {}
    }

    mp_int32 iRet = pclose(fStream);
    if (-1 == iRet) {
        iErr = GetOSError();
        COMMLOG(OS_LOG_ERROR, "pclose excute failed, errno[%d]:%s.", iErr, GetOSStrErr(iErr, szErr, sizeof(szErr)));
        return ERROR_COMMON_SYSTEM_CALL_FAILED;
    }

    // pclose返回的是shell最终状态，需要获取子进程的返回值
    // 还需要考虑WIFSIGNALED和WIFSTOPPED的影响
    if (WIFEXITED(iRet)) {
        return WEXITSTATUS(iRet);
    } else {
        iErr = GetOSError();
        COMMLOG(
            OS_LOG_ERROR, "Sub process exit abnormally, errno[%d]:%s.", iErr, GetOSStrErr(iErr, szErr, sizeof(szErr)));
        return ERROR_COMMON_SYSTEM_CALL_FAILED;
    }
}

mp_int32 CSystemExec::ExecSystemWithoutEchoEnvNoWin(
    const mp_string& strLogCmd, const mp_string& strCommand, mp_string strEnv, mp_bool bNeedRedirect)
{
    (mp_void) strEnv;
    // 在命令末尾添加重定向命令
    // 信息全部重定向到agent日志文件
    mp_string strLogFilePath;

    mp_int32 errCode;
    mp_char szErr[SYSTEM_EXEC_NUM_256];
    uid_t uid = getuid();
    strLogFilePath = CPath::GetInstance().GetLogFilePath(AGENT_LOG_NAME);
    if (uid == 0) {
        strLogFilePath = CPath::GetInstance().GetSlogFilePath(ROOT_EXEC_LOG_NAME);
    }

    strLogFilePath = CMpString::BlankComma(strLogFilePath);
    mp_string strNewCmd, strLogNewCmd;
    strNewCmd = bNeedRedirect ? (strCommand + " 1>>" + strLogFilePath + " 2>&1") : strCommand;
    strLogNewCmd = bNeedRedirect ? (strLogCmd + " 1>>" + strLogFilePath + " 2>&1") : strLogCmd;

    // Coverity&Fortify误报:FORTIFY.Command_Injection
    // 已经按修复建议增加命令分隔符判断CheckCmdDelimiter
    FILE* execStream = popen(strNewCmd.c_str(), "r");
    if (NULL == execStream) {
        errCode = GetOSError();
        COMMLOG(OS_LOG_ERROR, "popen call failed, errno[%d]:%s.", errCode, GetOSStrErr(errCode, szErr, sizeof(szErr)));
        return ERROR_COMMON_SYSTEM_CALL_FAILED;
    }

    while (!feof(execStream)) {
        mp_char tmpBuf[SYSTEM_EXEC_NUM_1000] = {0};
        mp_char* cRet = fgets(tmpBuf, sizeof(tmpBuf), execStream);
        if (NULL == cRet) {}
    }

    COMMLOG(OS_LOG_DEBUG, "Leave ExecSystemWithoutEcho, command is %s", strLogNewCmd.c_str());
    mp_int32 iRet = pclose(execStream);
    if (-1 == iRet) {
        errCode = GetOSError();
        COMMLOG(
            OS_LOG_ERROR, "pclose excute failed, errno[%d]:%s.", errCode, GetOSStrErr(errCode, szErr, sizeof(szErr)));
        return ERROR_COMMON_SYSTEM_CALL_FAILED;
    }

    // pclose返回的是shell最终状态，需要获取子进程的返回值
    // 还需要考虑WIFSIGNALED和WIFSTOPPED的影响
    if (WIFEXITED(iRet)) {
        COMMLOG(OS_LOG_DEBUG, "Sub process return %d.", WEXITSTATUS(iRet));
        return WEXITSTATUS(iRet);
    } else {
        errCode = GetOSError();
        COMMLOG(OS_LOG_ERROR,
            "Sub process exit abnormally, errno[%d]:%s.",
            errCode,
            GetOSStrErr(errCode, szErr, sizeof(szErr)));
        return ERROR_COMMON_SYSTEM_CALL_FAILED;
    }
}

mp_int32 CSystemExec::ExecSystemWithEchoNoWin(
    const mp_string& strLogCmd, const mp_string& strCommand, vector<mp_string>& strEcho, mp_bool bNeedRedirect)
{
    uid_t uid = getuid();
    // root用户，将错误重定向至rootexec日志，否则，将错误重定向至rdagent日志
    mp_string strLogFilePath;
    if (uid == 0) {
        strLogFilePath = CPath::GetInstance().GetSlogFilePath(ROOT_EXEC_LOG_NAME);
    } else {
        strLogFilePath = CPath::GetInstance().GetLogFilePath(AGENT_LOG_NAME);
    }

    strLogFilePath = CMpString::BlankComma(strLogFilePath);
    mp_string strNewExecCmd, strLogNewCmd;
    strNewExecCmd = bNeedRedirect ? (strCommand + " 2>>" + strLogFilePath) : strCommand;
    strLogNewCmd = bNeedRedirect ? (strLogCmd + " 1>>" + strLogFilePath + " 2>&1") : strLogCmd;

    DBGLOG("Exec cmd=%s.", strNewExecCmd.c_str());
    // Coverity&Fortify误报:FORTIFY.Command_Injection
    // 已经按修复建议增加命令分隔符判断CheckCmdDelimiter
    FILE* pStream = popen(strNewExecCmd.c_str(), "r");
    if (NULL == pStream) {
        COMMLOG(OS_LOG_ERROR, "popen failed.");
        return ERROR_COMMON_SYSTEM_CALL_FAILED;
    }

    while (!feof(pStream)) {
        mp_char tmpBuf[SYSTEM_EXEC_NUM_1000] = {0};
        mp_char* cRet = fgets(tmpBuf, sizeof(tmpBuf), pStream);
        if (NULL == cRet) {}
        if (strlen(tmpBuf) > 0) {
            tmpBuf[strlen(tmpBuf) - 1] = 0;  // 去掉获取出来的字符串末尾的'\n'
        }

        mp_bool bFlag = (tmpBuf[0] == 0) || (tmpBuf[0] == '\n');
        if (bFlag) {
            continue;
        }

        mp_string tmpStr = tmpBuf;
        strEcho.emplace_back(tmpStr);
    }

    COMMLOG(OS_LOG_DEBUG, "Leave ExecSystemWithEcho, command is %s", strLogNewCmd.c_str());
    mp_int32 exitStatus = pclose(pStream);
    if (WIFEXITED(exitStatus)) {
        mp_int32 status = WEXITSTATUS(exitStatus);
        COMMLOG(OS_LOG_DEBUG, "Command exited with status %d.", status);
        return status;
    } else {
        COMMLOG(OS_LOG_ERROR, "Command did not exit normally.");
    }
    return exitStatus;
}
#endif
