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
#ifndef LOCAL_EXECALLER_H
#define LOCAL_EXECALLER_H
#include <vector>
#include "common/Defines.h"
#include "define/Types.h"
#include "common/JsonHelper.h"
#include "BasicJob.h"

namespace GeneralDB {
struct Param {
    Json::Value param;
    mp_string appType;
    mp_string cmdType;
    mp_string jobId;
    mp_string subJobId;
    mp_bool isAsyncInterface = MP_TRUE;
    mp_bool isProgessScript = MP_FALSE;
    mp_string scriptDir;
};

class LocalCmdExector {
public:
    ~LocalCmdExector() {}
    static LocalCmdExector &GetInstance()
    {
        static LocalCmdExector execaller;
        return execaller;
    }

public:
    int Exec(const Param &comParam, Json::Value &result, std::shared_ptr<BasicJob> job = {nullptr});
#ifdef WIN32
    int ExecuteCmd(const mp_string &combinStr, std::map<mp_string, mp_string>& sensitiveInfos,
    const mp_string& uniqueId, DWORD timeOut = INFINITE);
#else
    int ExecuteCmd(const mp_string &combinStr, std::map<mp_string, mp_string>& sensitiveInfos,
        const mp_string& uniqueId);
#endif
    int ExecuteCmd(const std::vector<mp_string> &cmdParams, std::map<mp_string, mp_string>& sensitiveInfos,
        const mp_string& uniqueId);
    int ReadResultFile(const mp_string &filePath, mp_string &ret);
    int WriteInfoToParamFile(const mp_string &filePath, const Json::Value &param);
    void GetGeneralDBScriptDir(const mp_string &appType, const Json::Value &value, mp_string &scriptDir);
private:
    int CheckCmdDelimiter(const mp_string& str);
#ifdef WIN32
    bool CheckTaskRunningWithWin(const std::vector<mp_string> &cmdParams);
#else
    bool CheckTaskRunningWithNoWin(const std::vector<mp_string> &cmdParams);
#endif
private:
    LocalCmdExector() {}
    bool CheckTaskRunning(const std::vector<mp_string> &cmdParams);
    int ExecActual(const Param &comParam, std::map<mp_string, mp_string>& sensitiveInfos,
        const mp_string &uniqueId, Json::Value &result, std::shared_ptr<BasicJob> job = {nullptr});
    int SplitParam(Json::Value &inputValue, mp_string &uniqueId,
        std::map<mp_string, mp_string> &sensitiveInfos);
    int ReWriteFile(const mp_string &filePath, const mp_string &content);
    void ConsolidateSenInfo(std::map<mp_string, mp_string> &sensitiveInfos, const mp_string& uniqueId,
        mp_string &strSensitiveInfo);
    int DeleteResultFile(const mp_string &filePath);
    void CleanMemorySenInfo(mp_string &senInfo);
    int GetScriptExecResult(const Param &comParam, const mp_string &uniqueId, Json::Value &result);
    int GetScriptExecResultWarn(const Param &comParam, const mp_string &uniqueId, Json::Value &result);
#ifdef WIN32
    int CreateProcPipe(HANDLE &hStdInputRd, HANDLE &hStdInputWr);
#endif
};
}
#endif