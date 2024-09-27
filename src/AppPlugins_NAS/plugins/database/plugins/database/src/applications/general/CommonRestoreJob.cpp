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
#include "CommonRestoreJob.h"
#include "ReportJobDetailHandler.h"

using namespace GeneralDB;
using namespace AppProtect;

namespace {
    const mp_string MODULE = "CommonRestoreJob";
    const mp_string RESTORE_HANDLE_MAP = "RestoreHandleMap";
    using defer = std::shared_ptr<void>;
}

int CommonRestoreJob::PrerequisiteJob()
{
    bool status = true;
    defer _(nullptr, [&](...) {
        SetJobToFinish();
        if (!status) {
            JobLogDetail jobLogDetail = {m_jobId, "", SubJobStatus::FAILED, LOG_LABEL_TYPE::UNDEFIND_LABEL};
            ReportJobDetailHandler::GetInstance()->ReportJobDetailToFrame(jobLogDetail);
        }
    });

    auto restoreJobPtr = std::dynamic_pointer_cast<AppProtect::RestoreJob>(GetJobInfo()->GetJobInfo());
    if (restoreJobPtr == nullptr) {
        status = false;
        HCP_Log(ERR, MODULE) << "Get restore job pointer failed." << HCPENDLOG;
        return MP_FAILED;
    }
    Json::Value restoreJobStr;
    StructToJson(*restoreJobPtr, restoreJobStr);
    Json::Value jsValue;
    jsValue["job"] = restoreJobStr;
    Json::Value retValue;
    Param param = {jsValue, restoreJobPtr->targetObject.subType, "RestorePrerequisite", restoreJobPtr->jobId};
    LocalCmdExector::GetInstance().GetGeneralDBScriptDir(restoreJobPtr->targetObject.subType,
        restoreJobStr["targetObject"], param.scriptDir);
    if (LocalCmdExector::GetInstance().Exec(param, retValue, shared_from_this()) != MP_SUCCESS) {
        status = false;
        HCP_Log(ERR, MODULE) << "Exec failed, jobId=" << restoreJobPtr->jobId << HCPENDLOG;
        return MP_FAILED;
    }
    HCP_Log(INFO, MODULE) << "PrerequisiteJob execute success, jobId=" << restoreJobPtr->jobId << HCPENDLOG;
    return MP_SUCCESS;
}

int CommonRestoreJob::GenerateSubJobManually()
{
    auto restoreJobPtr = std::dynamic_pointer_cast<AppProtect::RestoreJob>(GetJobInfo()->GetJobInfo());
    if (restoreJobPtr == nullptr) {
        HCP_Log(ERR, MODULE) << "Get restore job pointer failed." << HCPENDLOG;
        return MP_FAILED;
    }
    Json::Value restoreJobStr;
    StructToJson(*restoreJobPtr, restoreJobStr);
    Json::Value jsValue;
    jsValue["job"] = restoreJobStr;
    Param param = {jsValue, restoreJobPtr->targetObject.subType, "RestoreGenSubJob", restoreJobPtr->jobId, "",
        MP_FALSE};
    LocalCmdExector::GetInstance().GetGeneralDBScriptDir(restoreJobPtr->targetObject.subType,
        restoreJobStr["targetObject"], param.scriptDir);
    if (LocalCmdExector::GetInstance().Exec(param, m_manualResult, shared_from_this()) != MP_SUCCESS) {
        HCP_Log(ERR, MODULE) << "Exec failed, jobId=" << restoreJobPtr->jobId << HCPENDLOG;
        return MP_FAILED;
    }
    HCP_Log(INFO, MODULE) << "RestoreGenSubJob execute success, jobId=" << restoreJobPtr->jobId << HCPENDLOG;
    return MP_SUCCESS;
}

int CommonRestoreJob::GenerateSubJob()
{
    auto jobInfo = std::dynamic_pointer_cast<AppProtect::RestoreJob>(GetJobInfo()->GetJobInfo());
    if (jobInfo == nullptr) {
        JobLogDetail jobLogDetail = {m_jobId, "", SubJobStatus::FAILED, LOG_LABEL_TYPE::EXEC_GENJOB_FAIL};
        ReportJobDetailHandler::GetInstance()->ReportJobDetailToFrame(jobLogDetail);
        HCP_Log(ERR, MODULE) << "GenerateSub job failed for get restore failed." << HCPENDLOG;
        return MP_FAILED;
    }
    mp_string appType = jobInfo->targetObject.subType;
    Json::Value extendInfo;
    if (!Module::JsonHelper::JsonStringToJsonValue(jobInfo->targetEnv.extendInfo, extendInfo)) {
        HCP_Log(ERR, MODULE) << "JsonStringToJsonValue Failed! " << DBG(m_jobId) << HCPENDLOG;
        return MP_FAILED;
    }
    if (GenSubJob(RESTORE_HANDLE_MAP, appType, extendInfo, jobInfo->targetEnv.nodes) != MP_SUCCESS) {
        JobLogDetail jobLogDetail = {m_jobId, "", SubJobStatus::FAILED, LOG_LABEL_TYPE::EXEC_GENJOB_FAIL};
        ReportJobDetailHandler::GetInstance()->ReportJobDetailToFrame(jobLogDetail);
        HCP_Log(ERR, MODULE) << "Failed to generate subjob, jobId=. " << m_jobId << HCPENDLOG;
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

int CommonRestoreJob::ExecuteSubJob()
{
    defer _(nullptr, [&](...) {
        SetJobToFinish();
    });

    auto restoreJobPtr = std::dynamic_pointer_cast<AppProtect::RestoreJob>(GetJobInfo()->GetJobInfo());
    if (restoreJobPtr == nullptr || m_subJobInfo == nullptr) {
        HCP_Log(ERR, MODULE) << "ExecuteSubJob job failed for get restore failed." << HCPENDLOG;
        return MP_FAILED;
    }
    Json::Value restoreJobStr;
    Json::Value subJobStr;
    StructToJson(*restoreJobPtr, restoreJobStr);
    StructToJson(*m_subJobInfo, subJobStr);
    Json::Value jsValue;
    jsValue["job"] = restoreJobStr;
    jsValue["subJob"] = subJobStr;
    Json::Value retValue;
    Param param = {jsValue, restoreJobPtr->targetObject.subType, "Restore", restoreJobPtr->jobId,
        m_subJobInfo->subJobId};
    LocalCmdExector::GetInstance().GetGeneralDBScriptDir(restoreJobPtr->targetObject.subType,
        restoreJobStr["targetObject"], param.scriptDir);
    if (LocalCmdExector::GetInstance().Exec(param, retValue, shared_from_this()) != MP_SUCCESS) {
        int errorCode = JOB_INTERNAL_ERROR;
        if (retValue.isMember("bodyErr") && retValue["bodyErr"].isInt()) {
            INFOLOG("bodyErr is:%d", retValue["bodyErr"].asInt());
            errorCode = retValue["bodyErr"].asInt();
        }
        JobLogDetail jobLogDetail = {restoreJobPtr->jobId, m_subJobInfo->subJobId, SubJobStatus::FAILED,
            LOG_LABEL_TYPE::EXEC_RESTORE_SUBJOB_FAIL, errorCode};
        ReportJobDetailHandler::GetInstance()->ReportJobDetailToFrame(jobLogDetail);
        HCP_Log(ERR, MODULE) << "Exec failed, jobId=" << restoreJobPtr->jobId << ",subJobId=" <<
            m_subJobInfo->subJobId << HCPENDLOG;
        return MP_FAILED;
    }

    HCP_Log(INFO, MODULE) << "ExecuteSubJob execute success, jobId=" << restoreJobPtr->jobId << HCPENDLOG;
    return MP_SUCCESS;
}

int CommonRestoreJob::PostJob()
{
    defer _(nullptr, [&](...) {
        SetJobToFinish();
    });

    auto restoreJobPtr = std::dynamic_pointer_cast<AppProtect::RestoreJob>(GetJobInfo()->GetJobInfo());
    if (restoreJobPtr == nullptr || m_subJobInfo == nullptr) {
        HCP_Log(ERR, MODULE) << "PostJob job failed for get restore failed." << HCPENDLOG;
        return MP_FAILED;
    }

    Json::Value restoreJobStr;
    Json::Value subJobStr;
    StructToJson(*restoreJobPtr, restoreJobStr);
    StructToJson(*m_subJobInfo, subJobStr);
    Json::Value jsValue;
    jsValue["job"] = restoreJobStr;
    jsValue["subJob"] = subJobStr;
    jsValue["restoreJobResult"] = m_jobResult;
    Json::Value retValue;
    Param param = {jsValue, restoreJobPtr->targetObject.subType, "RestorePostJob", restoreJobPtr->jobId,
        m_subJobInfo->subJobId};
    LocalCmdExector::GetInstance().GetGeneralDBScriptDir(restoreJobPtr->targetObject.subType,
        restoreJobStr["targetObject"], param.scriptDir);
    if (LocalCmdExector::GetInstance().Exec(param, retValue, shared_from_this()) != MP_SUCCESS) {
        JobLogDetail jobLogDetail = {restoreJobPtr->jobId, m_subJobInfo->subJobId, SubJobStatus::FAILED,
            LOG_LABEL_TYPE::UNDEFIND_LABEL};
        ReportJobDetailHandler::GetInstance()->ReportJobDetailToFrame(jobLogDetail);
        HCP_Log(ERR, MODULE) << "Exec failed, jobId=" << restoreJobPtr->jobId << HCPENDLOG;
        return MP_FAILED;
    }

    HCP_Log(INFO, MODULE) << "PostJob execute success, jobId=" << restoreJobPtr->jobId << HCPENDLOG;
    return MP_SUCCESS;
}