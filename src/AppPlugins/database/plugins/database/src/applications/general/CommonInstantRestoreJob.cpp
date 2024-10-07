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
#include "CommonInstantRestoreJob.h"
#include "ReportJobDetailHandler.h"

using namespace GeneralDB;
using namespace AppProtect;

namespace {
    const mp_string INSTANT_RESTORE_HANDLE_MAP = "InstantRestoreHandleMap";
    const mp_string INSTANT_RESTORE_PRE = "InstantRestorePrerequisite";
    const mp_string INSTANT_RESTORE_GEN = "InstantRestoreGenSubJob";
    const mp_string INSTANT_RESTORE_SUB = "InstantRestore";
    const mp_string INSTANT_RESTORE_POST = "InstantRestorePostJob";
    using defer = std::shared_ptr<void>;
}

int CommonInstantRestoreJob::PrerequisiteJob()
{
    bool status = true;
    defer _(nullptr, [&](...) {
        SetJobToFinish();
        if (!status) {
            JobLogDetail jobLogDetail = {m_jobId, "", SubJobStatus::FAILED, LOG_LABEL_TYPE::UNDEFIND_LABEL};
            ReportJobDetailHandler::GetInstance()->ReportJobDetailToFrame(jobLogDetail);
        }
    });

    auto jobInfo = std::dynamic_pointer_cast<AppProtect::RestoreJob>(GetJobInfo()->GetJobInfo());
    if (jobInfo == nullptr) {
        status = false;
        ERRLOG("Get instant restore pre job pointer failed, jobId=%s.", m_jobId.c_str());
        return MP_FAILED;
    }
    Json::Value insrestoreJobVal;
    StructToJson(*jobInfo, insrestoreJobVal);
    Json::Value jsValue;
    jsValue["job"] = insrestoreJobVal;
    Json::Value retValue;
    Param param = {jsValue, jobInfo->targetObject.subType, INSTANT_RESTORE_PRE, m_jobId};
    if (LocalCmdExector::GetInstance().Exec(param, retValue, shared_from_this()) != MP_SUCCESS) {
        status = false;
        ERRLOG("Instant restore pre job execute failed, jobId=%s.", m_jobId.c_str());
        return MP_FAILED;
    }
    INFOLOG("Instant restore pre job execute success, jobId=%s.", m_jobId.c_str());
    return MP_SUCCESS;
}

int CommonInstantRestoreJob::GenerateSubJobManually()
{
    auto jobInfo = std::dynamic_pointer_cast<AppProtect::RestoreJob>(GetJobInfo()->GetJobInfo());
    if (jobInfo == nullptr) {
        ERRLOG("Get instant restore gen job pointer failed, jobId=%s.", m_jobId.c_str());
        return MP_FAILED;
    }
    Json::Value insrestoreJobVal;
    StructToJson(*jobInfo, insrestoreJobVal);
    Json::Value jsValue;
    jsValue["job"] = insrestoreJobVal;
    Param param = {jsValue, jobInfo->targetObject.subType, INSTANT_RESTORE_GEN, m_jobId};
    if (LocalCmdExector::GetInstance().Exec(param, m_manualResult, shared_from_this()) != MP_SUCCESS) {
        ERRLOG("Instant restore gen job execute failed, jobId=%s.", m_jobId.c_str());
        return MP_FAILED;
    }
    INFOLOG("Instant restore gen job execute success, jobId=%s.", m_jobId.c_str());
    return MP_SUCCESS;
}

int CommonInstantRestoreJob::GenerateSubJob()
{
    auto jobInfo = std::dynamic_pointer_cast<AppProtect::RestoreJob>(GetJobInfo()->GetJobInfo());
    if (jobInfo == nullptr) {
        JobLogDetail jobLogDetail = {m_jobId, "", SubJobStatus::FAILED, LOG_LABEL_TYPE::EXEC_GENJOB_FAIL};
        ReportJobDetailHandler::GetInstance()->ReportJobDetailToFrame(jobLogDetail);
        ERRLOG("Get instant restore gen job pointer failed, jobId=%s.", m_jobId.c_str());
        return MP_FAILED;
    }
    mp_string appType = jobInfo->targetObject.subType;
    Json::Value extendInfo;
    if (!Module::JsonHelper::JsonStringToJsonValue(jobInfo->targetEnv.extendInfo, extendInfo)) {
        ERRLOG("Json string to json value failed, jobId=%s.", m_jobId.c_str());
        return MP_FAILED;
    }
    if (GenSubJob(INSTANT_RESTORE_HANDLE_MAP, appType, extendInfo, jobInfo->targetEnv.nodes) != MP_SUCCESS) {
        JobLogDetail jobLogDetail = {m_jobId, "", SubJobStatus::FAILED, LOG_LABEL_TYPE::EXEC_GENJOB_FAIL};
        ReportJobDetailHandler::GetInstance()->ReportJobDetailToFrame(jobLogDetail);
        ERRLOG("Failed to generate subjob, jobId=%s.", m_jobId.c_str());
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

int CommonInstantRestoreJob::ExecuteSubJob()
{
    defer _(nullptr, [&](...) {
        SetJobToFinish();
    });

    auto jobInfo = std::dynamic_pointer_cast<AppProtect::RestoreJob>(GetJobInfo()->GetJobInfo());
    if (jobInfo == nullptr || m_subJobInfo == nullptr) {
        ERRLOG("Get instant restore subjob pointer failed, jobId=%s, subJobId=%s.", m_parentJobId.c_str(),
            m_jobId.c_str());
        return MP_FAILED;
    }
    Json::Value insrestoreJobVal;
    Json::Value subJobVal;
    StructToJson(*jobInfo, insrestoreJobVal);
    StructToJson(*m_subJobInfo, subJobVal);
    Json::Value jsValue;
    jsValue["job"] = insrestoreJobVal;
    jsValue["subJob"] = subJobVal;
    Json::Value retValue;
    Param param = {jsValue, jobInfo->targetObject.subType, INSTANT_RESTORE_SUB, m_parentJobId, m_jobId};
    if (LocalCmdExector::GetInstance().Exec(param, retValue, shared_from_this()) != MP_SUCCESS) {
        JobLogDetail jobLogDetail = {m_parentJobId, m_jobId, SubJobStatus::FAILED,
            LOG_LABEL_TYPE::EXEC_RESTORE_SUBJOB_FAIL, JOB_INTERNAL_ERROR};
        ReportJobDetailHandler::GetInstance()->ReportJobDetailToFrame(jobLogDetail);
        ERRLOG("Exec instant restore subjob failed, jobId=%s, subJobId=%s.", m_parentJobId.c_str(), m_jobId.c_str());
        return MP_FAILED;
    }
    INFOLOG("Exec instant restore subjob success, jobId=%s, subJobId=%s.", m_parentJobId.c_str(), m_jobId.c_str());
    return MP_SUCCESS;
}

int CommonInstantRestoreJob::PostJob()
{
    defer _(nullptr, [&](...) {
        SetJobToFinish();
    });

    auto jobInfo = std::dynamic_pointer_cast<AppProtect::RestoreJob>(GetJobInfo()->GetJobInfo());
    if (jobInfo == nullptr || m_subJobInfo == nullptr) {
        ERRLOG("Get instant restore post job pointer failed, jobId=%s, subJobId=%s.", m_parentJobId.c_str(),
            m_jobId.c_str());
        return MP_FAILED;
    }

    Json::Value insrestoreJobVal;
    Json::Value subJobVal;
    StructToJson(*jobInfo, insrestoreJobVal);
    StructToJson(*m_subJobInfo, subJobVal);
    Json::Value jsValue;
    jsValue["job"] = insrestoreJobVal;
    jsValue["subJob"] = subJobVal;
    jsValue["restoreJobResult"] = m_jobResult;
    Json::Value retValue;
    Param param = {jsValue, jobInfo->targetObject.subType, INSTANT_RESTORE_POST, m_jobId,
        m_subJobInfo->subJobId};
    if (LocalCmdExector::GetInstance().Exec(param, retValue, shared_from_this()) != MP_SUCCESS) {
        JobLogDetail jobLogDetail = {m_jobId, m_subJobInfo->subJobId, SubJobStatus::FAILED,
            LOG_LABEL_TYPE::UNDEFIND_LABEL};
        ReportJobDetailHandler::GetInstance()->ReportJobDetailToFrame(jobLogDetail);
        ERRLOG("Exec instant restore post job failed, jobId=%s, subJobId=%s.", m_parentJobId.c_str(), m_jobId.c_str());
        return MP_FAILED;
    }
    INFOLOG("Exec instant restore post job success, jobId=%s, subJobId=%s.", m_parentJobId.c_str(), m_jobId.c_str());
    return MP_SUCCESS;
}
