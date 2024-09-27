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
#include "CommonBackupService.h"
#include "log/Log.h"
#include "LocalCmdExector.h"
#include "ParseConfigFile.h"

using namespace GeneralDB;

namespace {
    constexpr auto MODULE = "CommonBackupService";
    constexpr auto RETUTN_CODE = "code";
    constexpr auto SUCCESS_CODE = 0;
    constexpr auto GENERAL_APP_TYPE = "GeneralDb";
    constexpr auto ALLOW_CHECK_COPY_IN_LOCAL_NODE = "AllowCheckCopyInLocalNode";
    constexpr auto ALLOW_CHECK_COPY_SUBJOB_IN_LOCAL_NODE = "AllowCheckCopySubJobInLocalNode";
} // namespace


void CommonBackupService::CheckBackupJobType(ActionResult& returnValue, const AppProtect::BackupJob& job)
{
    Json::Value backupJobStr;
    StructToJson(job, backupJobStr);
    // 调用执行器
    Json::Value jsValue;
    jsValue["job"] = backupJobStr;
    Json::Value retValue;
    Param param = {jsValue, job.protectObject.subType, "CheckBackupJobType", job.jobId, "", MP_FALSE};
    LocalCmdExector::GetInstance().GetGeneralDBScriptDir(job.protectObject.subType,
        backupJobStr["protectObject"], param.scriptDir);
    if (LocalCmdExector::GetInstance().Exec(param, retValue) != MP_SUCCESS) {
        returnValue.__set_code(INNER_ERROR);
        HCP_Log(ERR, MODULE) << "Exec failed, jobId=" << job.jobId << HCPENDLOG;
        return;
    }
    if (!retValue.isObject() || !retValue.isMember(RETUTN_CODE)) {
        returnValue.__set_code(INNER_ERROR);
        HCP_Log(ERR, MODULE) << "Exec failed, jobId=" << job.jobId << HCPENDLOG;
        return;
    }
    JsonToStruct(retValue, returnValue);
}

void CommonBackupService::AllowBackupInLocalNode(ActionResult& returnValue, const AppProtect::BackupJob& job,
    const AppProtect::BackupLimit::type limit)
{
    Json::Value backupJobStr;
    StructToJson(job, backupJobStr);
    Json::Value jsValue;
    jsValue["job"] = backupJobStr;
    jsValue["BackupLimit"] = limit;
    // 调用执行器
    Json::Value retValue;
    Param param = {jsValue, job.protectObject.subType, "AllowBackupInLocalNode", job.jobId, "", MP_FALSE};
    LocalCmdExector::GetInstance().GetGeneralDBScriptDir(job.protectObject.subType,
        backupJobStr["protectObject"], param.scriptDir);
    if (LocalCmdExector::GetInstance().Exec(param, retValue) != MP_SUCCESS) {
        returnValue.__set_code(INNER_ERROR);
        HCP_Log(ERR, MODULE) << "Exec failed, jobId=" << job.jobId << HCPENDLOG;
        return;
    }
    if (!retValue.isObject() || !retValue.isMember(RETUTN_CODE)) {
        returnValue.__set_code(INNER_ERROR);
        HCP_Log(ERR, MODULE) << "Exec failed, jobId=" << job.jobId << HCPENDLOG;
        return;
    }
    JsonToStruct(retValue, returnValue);
}

void CommonBackupService::AllowBackupSubJobInLocalNode(ActionResult& returnValue, const AppProtect::BackupJob& job,
    const AppProtect::SubJob& subJob)
{
    if (subJob.jobType == SubJobType::type::POST_SUB_JOB) {
        returnValue.__set_code(SUCCESS_CODE);
        HCP_Log(INFO, MODULE) << "Post backup job return success jobId=" << job.jobId
            << "subJobId=" << subJob.jobId << HCPENDLOG;
        return;
    }
    Json::Value backupJobStr;
    Json::Value subJobStr;
    StructToJson(job, backupJobStr);
    StructToJson(subJob, subJobStr);
    Json::Value jsValue;
    jsValue["job"] = backupJobStr;
    jsValue["subJob"] = subJobStr;
    // 调用执行器
    Json::Value retValue;
    Param param = {jsValue, job.protectObject.subType, "AllowBackupInLocalNode", job.jobId, "", MP_FALSE};
    LocalCmdExector::GetInstance().GetGeneralDBScriptDir(job.protectObject.subType,
        backupJobStr["protectObject"], param.scriptDir);
    if (LocalCmdExector::GetInstance().Exec(param, retValue) != MP_SUCCESS) {
        returnValue.__set_code(INNER_ERROR);
        HCP_Log(ERR, MODULE) << "Exec failed, jobId=" << job.jobId << " subJobId=" << subJob.jobId << HCPENDLOG;
        return;
    }
    if (!retValue.isObject() || !retValue.isMember(RETUTN_CODE)) {
        returnValue.__set_code(INNER_ERROR);
        HCP_Log(ERR, MODULE) << "Exec failed, jobId=" << job.jobId << " subJobId=" << subJob.jobId << HCPENDLOG;
        return;
    }
    JsonToStruct(retValue, returnValue);
}

bool CommonBackupService::IsScriptExist(const mp_string &appType, const mp_string &cmdType)
{
    mp_string actionScript;
    mp_string processScript;
    Param param;
    param.appType = appType;
    param.cmdType = cmdType;
    if (ParseConfigFile::GetInstance()->GetExectueCmd(param, actionScript, processScript) != MP_SUCCESS) {
        HCP_Log(ERR, MODULE) << "Get script from config failed.cmdType=" << cmdType << HCPENDLOG;
        return false;
    }
    return true;
}

void CommonBackupService::AllowRestoreInLocalNode(ActionResult& returnValue, const AppProtect::RestoreJob& job)
{
    const mp_string cmdType = "AllowRestoreInLocalNode";
    if (!IsScriptExist(job.targetObject.subType, cmdType)) {
        HCP_Log(ERR, MODULE) << "Script config is not exist, appType=" << job.targetObject.subType
            << " cmdType=" << cmdType << HCPENDLOG;
        return;
    }
    Json::Value restoreJobStr;
    StructToJson(job, restoreJobStr);
    Json::Value jsValue;
    jsValue["job"] = restoreJobStr;
    // 调用执行器
    Json::Value retValue;
    Param param = {jsValue, job.targetObject.subType, cmdType, job.jobId, "", MP_FALSE};
    LocalCmdExector::GetInstance().GetGeneralDBScriptDir(job.targetObject.subType,
        restoreJobStr["targetObject"], param.scriptDir);
    if (LocalCmdExector::GetInstance().Exec(param, retValue) != MP_SUCCESS) {
        returnValue.__set_code(INNER_ERROR);
        HCP_Log(ERR, MODULE) << "Exec failed, jobId=" << job.jobId << HCPENDLOG;
        return;
    }
    if (!retValue.isObject() || !retValue.isMember(RETUTN_CODE)) {
        returnValue.__set_code(INNER_ERROR);
        HCP_Log(ERR, MODULE) << "Exec failed, jobId=" << job.jobId << HCPENDLOG;
        return;
    }
    JsonToStruct(retValue, returnValue);
}

void CommonBackupService::AllowRestoreSubJobInLocalNode(ActionResult& returnValue, const AppProtect::RestoreJob& job,
    const AppProtect::SubJob& subJob)
{
    const mp_string cmdType = "AllowRestoreInLocalNode";
    if (!IsScriptExist(job.targetObject.subType, cmdType)) {
        HCP_Log(ERR, MODULE) << "Script config is not exist, appType=" << job.targetObject.subType
            << " cmdType=" << cmdType << HCPENDLOG;
        returnValue.__set_code(SUCCESS_CODE);
        return;
    }
    if (subJob.jobType == SubJobType::type::POST_SUB_JOB) {
        returnValue.__set_code(SUCCESS_CODE);
        HCP_Log(INFO, MODULE) << "Post restore job return success jobId=" << job.jobId
            << "subJobId=" << subJob.jobId << HCPENDLOG;
        return;
    }
    Json::Value restoreJobStr;
    Json::Value subJobStr;
    StructToJson(job, restoreJobStr);
    StructToJson(subJob, subJobStr);
    Json::Value jsValue;
    jsValue["job"] = restoreJobStr;
    jsValue["subJob"] = subJobStr;

    // 调用执行器
    Json::Value retValue;
    Param param = {jsValue, job.targetObject.subType, cmdType, job.jobId, "", MP_FALSE};
    LocalCmdExector::GetInstance().GetGeneralDBScriptDir(job.targetObject.subType,
        restoreJobStr["targetObject"], param.scriptDir);
    if (LocalCmdExector::GetInstance().Exec(param, retValue) != MP_SUCCESS) {
        returnValue.__set_code(INNER_ERROR);
        HCP_Log(ERR, MODULE) << "Exec failed, jobId=" << job.jobId << HCPENDLOG;
        return;
    }
    if (!retValue.isObject() || !retValue.isMember(RETUTN_CODE)) {
        returnValue.__set_code(INNER_ERROR);
        HCP_Log(ERR, MODULE) << "Exec failed, jobId=" << job.jobId << HCPENDLOG;
        return;
    }
    JsonToStruct(retValue, returnValue);
}

void CommonBackupService::QueryJobPermission(AppProtect::JobPermission& returnJobPermission,
    const ApplicationEnvironment& appEnv, const Application& application)
{
    Json::Value appEnvStr;
    StructToJson(appEnv, appEnvStr);
    Json::Value applicationStr;
    StructToJson(application, applicationStr);
    Json::Value jsValue;
    jsValue["appEnv"] = appEnvStr;
    jsValue["application"] = applicationStr;
    // 调用执行器
    Json::Value retValue;
    Param param = {jsValue, application.subType, "QueryJobPermission", "", "", MP_FALSE};
    LocalCmdExector::GetInstance().GetGeneralDBScriptDir(application.subType, applicationStr, param.scriptDir);
    if (LocalCmdExector::GetInstance().Exec(param, retValue) != MP_SUCCESS) {
        HCP_Log(ERR, MODULE) << "Exec failed, appid=" << application.id << HCPENDLOG;
        return;
    }
    JsonToStruct(retValue, returnJobPermission);
}

mp_void CommonBackupService::DeliverTaskStatus(ActionResult& returnValue, const std::string& status,
    const std::string& taskId, const std::string& script)
{
    Json::Value paramValue;
    paramValue["status"] = status;
    paramValue["taskId"] = taskId;
    paramValue["script"] = script;
    Json::Value retValue;
    Param param;
    param.param = paramValue;
    param.appType = GENERAL_APP_TYPE;
    param.cmdType = "DeliverTaskStatus";
    param.scriptDir = script;
    param.isAsyncInterface = MP_FALSE;
    if (LocalCmdExector::GetInstance().Exec(param, retValue) != MP_SUCCESS) {
        returnValue.code = MP_FAILED;
        ERRLOG("Exec DeliverTaskStatus failed, jobId=%s, script=%s.", taskId.c_str(), script.c_str());
        return;
    }
    JsonToStruct(retValue, returnValue);
}

void CommonBackupService::AllowCheckCopyInLocalNode(ActionResult& returnValue, const AppProtect::CheckCopyJob& job)
{
    LOGGUARD("");
    if (!IsScriptExist(job.protectObject.subType, ALLOW_CHECK_COPY_IN_LOCAL_NODE)) {
        WARNLOG("AllowCheckCopyInLocalNode config is not exist, appType=%s.", job.protectObject.subType.c_str());
        returnValue.__set_code(SUCCESS_CODE);
        return;
    }
    Json::Value checkCopyJobStr;
    StructToJson(job, checkCopyJobStr);
    Json::Value jsValue;
    jsValue["job"] = checkCopyJobStr;
    // 调用执行器
    Json::Value retValue;
    Param param = {jsValue, job.protectObject.subType, ALLOW_CHECK_COPY_IN_LOCAL_NODE, job.jobId, "", MP_FALSE};
    LocalCmdExector::GetInstance().GetGeneralDBScriptDir(job.protectObject.subType,
        checkCopyJobStr["protectObject"], param.scriptDir);
    if (LocalCmdExector::GetInstance().Exec(param, retValue) != MP_SUCCESS) {
        returnValue.__set_code(INNER_ERROR);
        ERRLOG("Exec allow check copy in local node failed, appType=%s.", job.protectObject.subType.c_str());
        return;
    }
    JsonToStruct(retValue, returnValue);
}

void CommonBackupService::AllowCheckCopySubJobInLocalNode(ActionResult& returnValue,
    const AppProtect::CheckCopyJob& job, const AppProtect::SubJob& subJob)
{
    LOGGUARD("");
    if (!IsScriptExist(job.protectObject.subType, ALLOW_CHECK_COPY_SUBJOB_IN_LOCAL_NODE)) {
        WARNLOG("AllowCheckCopySubJobInLocalNode config is not exist, appType=%s.", job.protectObject.subType.c_str());
        returnValue.__set_code(SUCCESS_CODE);
        return;
    }
    Json::Value checkCopyJobStr;
    Json::Value subJobStr;
    StructToJson(job, checkCopyJobStr);
    StructToJson(subJob, subJobStr);
    Json::Value jsValue;
    jsValue["job"] = checkCopyJobStr;
    jsValue["subJob"] = subJobStr;
    // 调用执行器
    Json::Value retValue;
    Param param = {jsValue, job.protectObject.subType, ALLOW_CHECK_COPY_SUBJOB_IN_LOCAL_NODE, job.jobId, "", MP_FALSE};
    LocalCmdExector::GetInstance().GetGeneralDBScriptDir(job.protectObject.subType,
        checkCopyJobStr["protectObject"], param.scriptDir);
    if (LocalCmdExector::GetInstance().Exec(param, retValue) != MP_SUCCESS) {
        returnValue.__set_code(INNER_ERROR);
        ERRLOG("Exec allow check copy subjob in local node failed, appType=%s.", job.protectObject.subType.c_str());
        return;
    }
    JsonToStruct(retValue, returnValue);
}