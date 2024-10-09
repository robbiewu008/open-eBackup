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
#ifdef WIN32
#include <winsock2.h>
#include <windows.h>
#else
#include <unistd.h>
#include <sys/wait.h>
#endif
#include <iostream>
#include <cerrno>
#include <memory>
#include <fstream>
#include "log/Log.h"
#include "Utils.h"
#include "UniqueId.h"
#include "ParseConfigFile.h"
#include "ParamHandler.h"
#include "common/File.h"
#include "JobDetail.h"
#include "DBPluginPath.h"
#include "securec.h"
#include "LocalCmdExector.h"

using namespace GeneralDB;

namespace {
    enum class CmdParam {
        CMD_SCRIPT_NAME = 0,
        CMD_MAIN_JOB_INDEX,
        CMD_SUB_JOB_INDEX
    };
    const mp_string MODULE_NAME = "LocalCmdExector";
    const mp_string SEMICOLON_STR = ";";
    const mp_string VERTICAL_LINE_STR = "|";
    const mp_string ADDRESS_STR = "&";
    const mp_string STR_QUOTES = "\"";
    const mp_string GENERAL_APP_TYPE = "GeneralDb";
    const mp_string GENERALDN_LOG_NAME = "generaldbplugin.log";
    constexpr int SYSTEM_ERR_LEHGTH = 256;
    constexpr int RETRY_TIME = 3;
    constexpr int SENINFOWIPETIMES = 3;
    using Defer = std::shared_ptr<void>;
}

int LocalCmdExector::SplitParam(Json::Value &inputValue, mp_string &uniqueId,
    std::map<mp_string, mp_string> &sensitiveInfos)
{
    if (UniqueId::GetInstance().GenerateUniqueID(uniqueId) != MP_SUCCESS) {
        ERRLOG("Generate unique id failed.");
        return MP_FAILED;
    }
    if (inputValue.isNull()) {
        INFOLOG("Do not parse param, inputValue is null.");
        return MP_SUCCESS;
    }
    auto handler = std::make_unique<ParamHandler>();
    if (handler == nullptr) {
        ERRLOG("Handler is nullptr.");
        return MP_FAILED;
    }

    if (handler->Exec(inputValue, sensitiveInfos) != MP_SUCCESS) {
        ERRLOG("Param parse failed, uniqueId=%s.", uniqueId.c_str());
        return MP_FAILED;
    }
    mp_string paramPath = DBPluginPath::GetInstance()->GetParamPath() + "param" + uniqueId;
    if (WriteInfoToParamFile(paramPath, inputValue) != MP_SUCCESS) {
        ERRLOG("Failed to write param to file.");
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

int LocalCmdExector::Exec(const Param &comParam, Json::Value &result, std::shared_ptr<BasicJob> job)
{
    LOGGUARD("");
    mp_string uniqueId;
    std::map<mp_string, mp_string> sensitiveInfos;
    Json::Value tempParm = std::move(comParam.param);
    int retRes = SplitParam(tempParm, uniqueId, sensitiveInfos);
    if (retRes != MP_SUCCESS) {
        ERRLOG("Split param failed, jobId=%s, subJobId=%s.", comParam.jobId.c_str(), comParam.subJobId.c_str());
        return retRes;
    }
    return ExecActual(comParam, sensitiveInfos, uniqueId, result, job);
}

int LocalCmdExector::ExecActual(const Param &comParam, std::map<mp_string, mp_string>& sensitiveInfos,
    const mp_string &uniqueId, Json::Value &result, std::shared_ptr<BasicJob> job)
{
    Defer _(nullptr, [&](...) {
        mp_string resultFilePath = DBPluginPath::GetInstance()->GetResultPath() + "result" + uniqueId;
        DeleteResultFile(resultFilePath);
    });
    mp_string actionScript;
    mp_string processScript;
    if (ParseConfigFile::GetInstance()->GetExectueCmd(comParam, actionScript, processScript) != MP_SUCCESS) {
        ERRLOG("Parse config file failed, jobId=%s, subJobId=%s.", comParam.jobId.c_str(), comParam.subJobId.c_str());
        return MP_FAILED;
    }
    // 异步接口启动定时线程上报进度
    std::shared_ptr<JobDetail> timerHanlder = std::make_shared<JobDetail>();
    if (comParam.isAsyncInterface && !processScript.empty()) {
        std::thread reportThreads(&JobDetail::QueryJobDetails, timerHanlder, comParam);
        reportThreads.detach();
    }
    if (comParam.isAsyncInterface) {
        std::thread abortOrPauseThreads(&JobDetail::AbortJobOrPauseJob, timerHanlder, comParam, job);
        abortOrPauseThreads.detach();
    }
    mp_string scriptName = comParam.isProgessScript ? processScript : actionScript;
    std::vector<mp_string> curParam({scriptName, comParam.jobId, comParam.subJobId});
    if (comParam.isAsyncInterface && CheckTaskRunning(curParam)) {
        WARNLOG("Task is running, jobId=%s, subJobId=%s, script %s.", comParam.jobId.c_str(),
            comParam.subJobId.c_str(), scriptName.c_str());
        return MP_SUCCESS;
    }
    std::vector<mp_string> taskParam({scriptName, uniqueId, comParam.jobId, comParam.subJobId});
    if (ExecuteCmd(taskParam, sensitiveInfos, uniqueId) != MP_SUCCESS) {
        ERRLOG("Execute cmd failed, jobId=%s, subJobId=%s, uniqueId=%s.", comParam.jobId.c_str(),
            comParam.subJobId.c_str(), uniqueId.c_str());
        timerHanlder->StopAbortOrPauseTimer();
        timerHanlder->NotifyJobDetailTimer(DetailTimerStatus::STOP_TIMER);
        GetScriptExecResultWarn(comParam, uniqueId, result);
        return MP_FAILED;
    }
    // 同步接口，进度脚本需要读取结果文件
    if (comParam.isAsyncInterface) {
        timerHanlder->StopAbortOrPauseTimer();
        timerHanlder->NotifyJobDetailTimer(DetailTimerStatus::QUERY_DETAIL_NOW);
    }
    return GetScriptExecResult(comParam, uniqueId, result);
}

int LocalCmdExector::CheckCmdDelimiter(const mp_string& str)
{
    if (mp_string::npos != str.find(SEMICOLON_STR, 0) ||
        mp_string::npos != str.find(VERTICAL_LINE_STR, 0) ||
        mp_string::npos != str.find(ADDRESS_STR, 0) ||
        mp_string::npos != str.find(Module::SIGN_IN, 0) ||
        mp_string::npos != str.find(Module::SIGN_OUT, 0) ||
        mp_string::npos != str.find(Module::SIGN_BACKQUOTE, 0) ||
        mp_string::npos != str.find(Module::SIGN_EXCLAMATION, 0) ||
        mp_string::npos != str.find(Module::STR_BACKSLASH, 0) ||
        mp_string::npos != str.find(Module::STR_CODE_WARP, 0) ||
        mp_string::npos != str.find(Module::SIGN_DOLLAR, 0)) {
        return MP_FAILED;
    }

    return MP_SUCCESS;
}

#ifdef WIN32
bool LocalCmdExector::CheckTaskRunningWithWin(const std::vector<mp_string> &cmdParams)
{
    bool isEmpty = true;
    for (auto &cmd : cmdParams) {
        if (CheckCmdDelimiter(cmd) != MP_SUCCESS) {
            ERRLOG("Check input param failed, cmd: %s.", cmd.c_str());
            return false;
        }
        if (!cmd.empty()) {
            isEmpty = false;
        }
    }
    if (isEmpty) {
        DBGLOG("All task id is empty.");
        return false;
    }
    const mp_string CHECK_PREFIX_WMIC = "wmic process where name=";
    const mp_string CHECK_EXEC_NAME = "\"python.exe\"";
    const mp_string CHECK_SUFFIX_PARAM = " get commandline /value";
    mp_string command = CHECK_PREFIX_WMIC;
    command += CHECK_EXEC_NAME;
    command += CHECK_SUFFIX_PARAM;
    mp_string filePath = DBPluginPath::GetInstance()->GetResultPath();
    filePath += "checkResult";
    command += filePath;
    std::map<mp_string, mp_string> sensitiveInfos;
    mp_string uniqueId;
    if (ExecuteCmd(command, sensitiveInfos, uniqueId) != MP_SUCCESS) {
        WARNLOG("Eexucte check task running command failed, cmd: %s.", command.c_str());
        return false;
    }
    mp_string result;
    if (ReadResultFile(filePath, result) != MP_SUCCESS) {
        ERRLOG("Read check result failed, path: %s.", filePath.c_str());
        return false;
    }
    bool status = result.find(cmdParams[static_cast<int>(CmdParam::CMD_MAIN_JOB_INDEX)]) != std::string::npos
        && result.find(cmdParams[static_cast<int>(CmdParam::CMD_SUB_JOB_INDEX)]) != std::string::npos;
    DBGLOG("Task running, status is %d, jobId=%s, subJobId=%s.", status,
        cmdParams[static_cast<int>(CmdParam::CMD_MAIN_JOB_INDEX)].c_str(),
        cmdParams[static_cast<int>(CmdParam::CMD_SUB_JOB_INDEX)].c_str());
    return status;
}

#else
bool LocalCmdExector::CheckTaskRunningWithNoWin(const std::vector<mp_string> &cmdParams)
{
    mp_string combinStr = "ps -axu";
    const mp_string FILTER_GREP_OTHER = " | grep ";
    const mp_string FILTER_GREP = " | grep -v grep";
    std::stringstream sstream;
    sstream << DBPluginPath::GetInstance()->GetResultPath() << "checkResult";
    mp_string filePath = sstream.str();
    bool isEmpty = true;
    for (auto &cmd : cmdParams) {
        if (CheckCmdDelimiter(cmd) != MP_SUCCESS) {
            ERRLOG("Check input param failed, cmd: %s.", cmd.c_str());
            return false;
        }
        if (!cmd.empty()) {
            isEmpty = false;
            combinStr += FILTER_GREP_OTHER;
            combinStr += STR_QUOTES;
            combinStr += cmd;
            combinStr += STR_QUOTES;
        }
    }
    if (isEmpty) {
        DBGLOG("All task id is empty.");
        return false;
    }
    combinStr += FILTER_GREP;
    combinStr += " >> ";
    combinStr += filePath;
    std::map<mp_string, mp_string> sensitiveInfos;
    mp_string uniqueId;
    if (ExecuteCmd(combinStr, sensitiveInfos, uniqueId) != MP_SUCCESS) {
        WARNLOG("Eexucte check task running command failed, cmd: %s.", combinStr.c_str());
        return false;
    }
    mp_string result;
    if (ReadResultFile(filePath, result) != MP_SUCCESS) {
        ERRLOG("Read check result failed, path: %s.", filePath.c_str());
        return false;
    }
    bool status = result.find(cmdParams[static_cast<int>(CmdParam::CMD_MAIN_JOB_INDEX)]) != std::string::npos
        && result.find(cmdParams[static_cast<int>(CmdParam::CMD_SUB_JOB_INDEX)]) != std::string::npos;
    DBGLOG("Task running, status is %d, jobId=%s, subJobId=%s.", status,
        cmdParams[static_cast<int>(CmdParam::CMD_MAIN_JOB_INDEX)].c_str(),
        cmdParams[static_cast<int>(CmdParam::CMD_SUB_JOB_INDEX)].c_str());
    return status;
}
#endif

bool LocalCmdExector::CheckTaskRunning(const std::vector<mp_string> &cmdParams)
{
    if (cmdParams.size() < static_cast<int>(CmdParam::CMD_SUB_JOB_INDEX)) {
        ERRLOG("Param size wrong, size: %d.", cmdParams.size());
        return false;
    }
#ifdef WIN32
    return CheckTaskRunningWithWin(cmdParams);
#else
    return CheckTaskRunningWithNoWin(cmdParams);
#endif
}

int LocalCmdExector::ExecuteCmd(const std::vector<mp_string> &cmdParams,
    std::map<mp_string, mp_string>& sensitiveInfos, const mp_string& uniqueId)
{
    const mp_string SPACE_STR = " ";
#ifdef WIN32
    mp_string combinStr = "cmd /c python ";
#else
    mp_string combinStr = "python3 ";
#endif
    bool isEmpty = true;
    for (auto &cmd : cmdParams) {
        if (CheckCmdDelimiter(cmd) != MP_SUCCESS) {
            ERRLOG("Check input param failed, cmd: %s.", cmd.c_str());
            return MP_FAILED;
        }
        if (!cmd.empty()) {
            isEmpty = false;
            combinStr += cmd;
            combinStr += SPACE_STR;
        }
    }
    if (isEmpty) {
        ERRLOG("Exec input param is empty.");
        return MP_FAILED;
    }
    INFOLOG("Execute command start, cmd: %s.", combinStr.c_str());
#ifndef WIN32
    combinStr += " 1>>" + DBPluginPath::GetInstance()->GetLogPath() + "/" + GENERALDN_LOG_NAME + " 2>&1";
#endif
    mp_int32 ret = ExecuteCmd(combinStr, sensitiveInfos, uniqueId);
    if (ret != MP_SUCCESS) {
        ERRLOG("Eexucte command failed, ret: %d, cmd: %s.", ret, combinStr.c_str());
        return MP_FAILED;
    }
    INFOLOG("Execute command success, cmd: %s.", combinStr.c_str());
    return MP_SUCCESS;
}
#ifdef WIN32
int LocalCmdExector::ExecuteCmd(const mp_string &combinStr, std::map<mp_string, mp_string>& sensitiveInfos,
    const mp_string& uniqueId, DWORD timeOut)
{
    mp_string strSensitiveInfo;
    ConsolidateSenInfo(sensitiveInfos, uniqueId, strSensitiveInfo);
    HANDLE hStdInputRd;
    HANDLE hStdInputWr;
    mp_int32 ret = CreateProcPipe(hStdInputRd, hStdInputWr);
    if (ret != MP_SUCCESS) {
        ERRLOG("Create process pipe failed.");
        return ret;
    }
    PROCESS_INFORMATION piProcInfo;
    STARTUPINFO siStartInfo;
    ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));
    ZeroMemory(&siStartInfo, sizeof(STARTUPINFO));
    siStartInfo.cb = sizeof(STARTUPINFO);
    siStartInfo.hStdInput = hStdInputRd; // 子进程标准输入-管道读端
    siStartInfo.wShowWindow = SW_HIDE;
    siStartInfo.dwFlags = STARTF_USESTDHANDLES;
    if (!CreateProcess(NULL, TEXT((mp_char *)combinStr.c_str()), NULL, NULL, MP_TRUE, 0, NULL, NULL, &siStartInfo,
        &piProcInfo)) {
        int retCode = GetLastError();
        ERRLOG("Create child process failed, errorcode=%d.", retCode);
        return MP_FAILED;
    }
    DWORD dwRead;
    DWORD dwWritten;
    if (!WriteFile(hStdInputWr, strSensitiveInfo.c_str(), strlen(strSensitiveInfo.c_str()), &dwWritten, NULL)) {
        ERRLOG("Write sensitive info to stdInput failed.");
        CloseHandle(piProcInfo.hThread);
        CloseHandle(piProcInfo.hProcess);
        CleanMemorySenInfo(strSensitiveInfo);
        return MP_FAILED;
    }
    CleanMemorySenInfo(strSensitiveInfo);
    CloseHandle(hStdInputWr);
    CloseHandle(hStdInputRd);
    CloseHandle(piProcInfo.hThread);
    if (WaitForSingleObject(piProcInfo.hProcess, timeOut) != WAIT_OBJECT_0) {
        ERRLOG("Wait for single object failed, errno=%d.", errno);
        CloseHandle(piProcInfo.hProcess);
        return MP_FAILED;
    }
    int exitCode = 0;
    GetExitCodeProcess(piProcInfo.hProcess, (LPDWORD)&exitCode);
    CloseHandle(piProcInfo.hProcess);
    DBGLOG("Execute script exit code is %d.", exitCode);
    return exitCode;
}
#else
int LocalCmdExector::ExecuteCmd(const mp_string &combinStr, std::map<mp_string, mp_string>& sensitiveInfos,
    const mp_string& uniqueId)
{
    mp_string strSensitiveInfo;
    ConsolidateSenInfo(sensitiveInfos, uniqueId, strSensitiveInfo);
    mp_string execCmd = combinStr;
    FILE* execStream = popen(execCmd.c_str(), "w");
    mp_char szErr[SYSTEM_ERR_LEHGTH];
    if (execStream == nullptr) {
        mp_int32 err = Module::GetOSError();
        ERRLOG("Popen call failed, errno[%d]:%s.", err, Module::GetOSStrErr(err, szErr, sizeof(szErr)));
        return MP_FAILED;
    }
    fputs(strSensitiveInfo.c_str(), execStream);
    fputc('\n', execStream);
    CleanMemorySenInfo(strSensitiveInfo);
    mp_int32 ret = pclose(execStream);
    if (MP_FAILED == ret) {
        mp_int32 err = Module::GetOSError();
        ERRLOG("Pclose execute failed, errno[%d]:%s.", err, Module::GetOSStrErr(err, szErr, sizeof(szErr)));
        return MP_FAILED;
    }
    if (WIFEXITED(ret)) {
        DBGLOG("Execute script exit code is %d.", WEXITSTATUS(ret));
        return MP_SUCCESS;
    } else {
        mp_int32 err = Module::GetOSError();
        ERRLOG("Sub process exit abnormally, errno[%d]:%s.", err, Module::GetOSStrErr(err, szErr, sizeof(szErr)));
        return MP_FAILED;
    }
}
#endif

int LocalCmdExector::ReadResultFile(const mp_string &filePath, mp_string &ret)
{
    if (Module::CFile::ReadFile(filePath, ret) != MP_SUCCESS) {
        ERRLOG("Read result file failed, path: %s.", filePath.c_str());
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

int LocalCmdExector::DeleteResultFile(const mp_string &filePath)
{
#ifdef WIN32
        if (DeleteFile(filePath.c_str()) != MP_SUCCESS) {
            WARNLOG("Delete result file failed, path: %s.", filePath.c_str());
            return MP_FAILED;
        }
#else
        int result = remove(filePath.c_str());
        if (result != MP_SUCCESS) {
            WARNLOG("Delete result file failed, path: %s.", filePath.c_str());
            return MP_FAILED;
        }
#endif
    return MP_SUCCESS;
}

int LocalCmdExector::ReWriteFile(const mp_string &filePath, const mp_string &content)
{
    int retryCnt = 0;
    bool success = false;
    do {
        DBGLOG("Start rewrite file, rewrite count: %d.", retryCnt + 1);
        if (Module::CFile::CreateFile(filePath) != MP_SUCCESS) {
            WARNLOG("Create file failed, path: %s.", filePath.c_str());
            continue;
        }
        std::vector<mp_string> vecInput({content});
        mp_string tempFilePath = filePath;
        if (Module::CIPCFile::WriteFile(tempFilePath, vecInput) != MP_SUCCESS) {
            ERRLOG("Write file failed, path: %s.", filePath.c_str());
            continue;
        }
        success = true;
        if (success) {
            break;
        }
    } while (++retryCnt < RETRY_TIME);
    if (retryCnt == RETRY_TIME) {
        ERRLOG("Rewrite param file failed, path: %s.", filePath.c_str());
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

int LocalCmdExector::WriteInfoToParamFile(const mp_string &filePath, const Json::Value &param)
{
    Json::FastWriter jsonWriter;
    mp_string inputStr = jsonWriter.write(param);
    std::ofstream file(filePath, std::ios::out | std::ios::binary);
    file.write(inputStr.c_str(), inputStr.size());
    if (!file.good()) {
        WARNLOG("Write file failed, will to rewrite, path: %s.", filePath.c_str());
        if (ReWriteFile(filePath, inputStr) != MP_SUCCESS) {
            return MP_FAILED;
        }
    }
    file.close();
    if (!Module::CFile::FileExist(filePath.c_str())) {
        ERRLOG("File not exist, path: %s.", filePath.c_str());
        return MP_FAILED;
    }
    DBGLOG("Write param file sucess, path: %s.", filePath.c_str());
    return MP_SUCCESS;
}

void LocalCmdExector::ConsolidateSenInfo(std::map<mp_string, mp_string> &sensitiveInfos,
    const mp_string& uniqueId, mp_string &strSensitiveInfo)
{
    Json::Value sensitiveValue;
    for (auto &it : sensitiveInfos) {
        mp_string sensitiveKey = it.first + "_" + uniqueId;
        sensitiveValue[sensitiveKey] = std::move(it.second);
        CleanMemorySenInfo(it.second);
    }
    strSensitiveInfo = Json::FastWriter().write(sensitiveValue);
}

#ifdef WIN32
int LocalCmdExector::CreateProcPipe(HANDLE &hStdInputRd, HANDLE &hStdInputWr)
{
    SECURITY_ATTRIBUTES saAttr;
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;
    saAttr.lpSecurityDescriptor = NULL;
    if (!CreatePipe(&hStdInputRd, &hStdInputWr, &saAttr, 0)) { // 创建匿名管道
        mp_int32 retCode = GetLastError();
        ERRLOG("CreatePipe failed, errorCode=%d", retCode);
        return retCode;
    }
    // 设置子进程不能继承接收输入管道的另一端：ChildIn_Write
    if (!SetHandleInformation(hStdInputWr, HANDLE_FLAG_INHERIT, 0)) {
        mp_int32 retCode = GetLastError();
        ERRLOG("Set handle info failed, errorCode=%d", retCode);
        return retCode;
    }
    return MP_SUCCESS;
}
#endif

void LocalCmdExector::GetGeneralDBScriptDir(const mp_string &appType, const Json::Value &value, mp_string &scriptDir)
{
    if (appType != GENERAL_APP_TYPE) {
        return;
    }
    if (!value.isObject() || !value.isMember("extendInfo")) {
        WARNLOG("Json value have no extendInfo key.");
        return;
    }
    if (value["extendInfo"].isObject() && value["extendInfo"].isMember("script")
        && !value["extendInfo"]["script"].isNull() && value["extendInfo"]["script"].isString()) {
        scriptDir = value["extendInfo"]["script"].asString();
    }
}

int LocalCmdExector::GetScriptExecResult(const Param &comParam, const mp_string &uniqueId, Json::Value &result)
{
    // 同步接口，进度脚本需要读取结果文件
    if (!comParam.isAsyncInterface || comParam.isProgessScript) {
        std::stringstream sstream;
        sstream << DBPluginPath::GetInstance()->GetResultPath() << "result" << uniqueId;
        mp_string actionResult;
        mp_string resultFilePath = sstream.str();
        if (ReadResultFile(resultFilePath, actionResult) != MP_SUCCESS || actionResult.empty()) {
            ERRLOG("Read action result file failed, jobId=%s, subJobId=%s, path: %s.", comParam.jobId.c_str(),
                comParam.subJobId.c_str(), sstream.str().c_str());
            return MP_FAILED;
        }
        if (!Module::JsonHelper::JsonStringToJsonValue(actionResult, result)) {
            ERRLOG("Failed to convert string to json, jobId=%s, subJobId=%s.", comParam.jobId.c_str(),
                comParam.subJobId.c_str());
            return MP_FAILED;
        }
    }
    return MP_SUCCESS;
}

int LocalCmdExector::GetScriptExecResultWarn(const Param &comParam, const mp_string &uniqueId, Json::Value &result)
{
    // 读取结果文件
    std::stringstream sstream;
    sstream << DBPluginPath::GetInstance()->GetResultPath() << "result" << uniqueId;
    mp_string actionResult;
    mp_string resultFilePath = sstream.str();
    if (ReadResultFile(resultFilePath, actionResult) != MP_SUCCESS || actionResult.empty()) {
        ERRLOG("Read action result file failed, jobId=%s, subJobId=%s, path: %s.",
            comParam.jobId.c_str(),
            comParam.subJobId.c_str(),
            sstream.str().c_str());
        return MP_FAILED;
    }
    if (!Module::JsonHelper::JsonStringToJsonValue(actionResult, result)) {
        ERRLOG("Failed to convert string to json, jobId=%s, subJobId=%s.",
            comParam.jobId.c_str(),
            comParam.subJobId.c_str());
        return MP_FAILED;
    }
    INFOLOG("GetScriptExecResultWarn success,data:%s", actionResult.c_str());
    return MP_SUCCESS;
}

void LocalCmdExector::CleanMemorySenInfo(mp_string &senInfo)
{
    // 敏感数据处理
    if (senInfo.empty()) {
        return;
    }
    memset_s(&senInfo[0], senInfo.length(), 0, senInfo.length());
}
