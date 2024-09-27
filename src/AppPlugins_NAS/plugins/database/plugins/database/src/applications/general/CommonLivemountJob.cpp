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
#include "CommonLivemountJob.h"
#include "log/Log.h"
#include "LocalCmdExector.h"
#include "ReportJobDetailHandler.h"

using namespace GeneralDB;
using namespace AppProtect;

namespace {
    const mp_string MODULE = "CommonLivemountJob";
    const mp_string LIVEMOUNT_HANDLE_MAP = "LivemountHandleMap";
    using defer = std::shared_ptr<void>;
} // namespace

int CommonLivemountJob::PrerequisiteJob()
{
    return MP_SUCCESS;
}

int CommonLivemountJob::GenerateSubJob()
{
    auto jobInfo = std::dynamic_pointer_cast<AppProtect::LivemountJob>(GetJobInfo()->GetJobInfo());
    if (jobInfo == nullptr) {
        JobLogDetail jobLogDetail = {m_jobId, "", SubJobStatus::FAILED, LOG_LABEL_TYPE::EXEC_GENJOB_FAIL};
        ReportJobDetailHandler::GetInstance()->ReportJobDetailToFrame(jobLogDetail);
        HCP_Log(ERR, MODULE) << "Failed to get livemount gen job info, jobId=" << m_jobId << HCPENDLOG;
        return MP_FAILED;
    }
    mp_string appType = jobInfo->targetObject.subType;
    Json::Value extendInfo;
    if (!Module::JsonHelper::JsonStringToJsonValue(jobInfo->targetEnv.extendInfo, extendInfo)) {
        JobLogDetail jobLogDetail = {m_jobId, "", SubJobStatus::FAILED, LOG_LABEL_TYPE::EXEC_GENJOB_FAIL};
        ReportJobDetailHandler::GetInstance()->ReportJobDetailToFrame(jobLogDetail);
        HCP_Log(ERR, MODULE) << "Failed to convert extendInfo, jobId=. " << m_jobId << HCPENDLOG;
        return MP_FAILED;
    }
    if (GenSubJob(LIVEMOUNT_HANDLE_MAP, appType, extendInfo, jobInfo->targetEnv.nodes) != MP_SUCCESS) {
        JobLogDetail jobLogDetail = {m_jobId, "", SubJobStatus::FAILED, LOG_LABEL_TYPE::EXEC_GENJOB_FAIL};
        ReportJobDetailHandler::GetInstance()->ReportJobDetailToFrame(jobLogDetail);
        HCP_Log(ERR, MODULE) << "Failed to generate subjob, jobId=. " << m_jobId << HCPENDLOG;
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

int CommonLivemountJob::ExecuteSubJob()
{
    HCP_Log(DEBUG, MODULE) << "Enter livemount execute sub job" << HCPENDLOG;
    mp_bool reportJobFailed = MP_FALSE;
    defer _(nullptr, [&](...) {
        SetJobToFinish();
        if (reportJobFailed) {
            JobLogDetail jobLogDetail = {m_parentJobId, m_jobId, SubJobStatus::FAILED,
                LOG_LABEL_TYPE::EXEC_LIVEMOUNT_SUBJOB_FAIL, JOB_INTERNAL_ERROR};
            ReportJobDetailHandler::GetInstance()->ReportJobDetailToFrame(jobLogDetail);
        }
    });

    std::shared_ptr<AppProtect::LivemountJob> livemountJobPtr =
        std::dynamic_pointer_cast<AppProtect::LivemountJob>(GetJobInfo()->GetJobInfo());
    if (livemountJobPtr == nullptr || m_subJobInfo == nullptr) {
        reportJobFailed = MP_TRUE;
        HCP_Log(ERR, MODULE) << "Failed to get livemount job info, jobId=" << m_parentJobId << ", subJobId=" <<
            m_jobId << HCPENDLOG;
        return MP_FAILED;
    }
    Json::Value jobParam;
    Json::Value subJobParam;
    StructToJson(*livemountJobPtr, jobParam);
    StructToJson(*m_subJobInfo, subJobParam);
    Json::Value jsValue;
    jsValue["job"] = jobParam;
    jsValue["subJob"] = subJobParam;
    HCP_Log(DEBUG, MODULE) << "Execute livemount sub Job, jobId=" << (*livemountJobPtr).jobId << ", subJobId=" <<
        (*m_subJobInfo).subJobId << HCPENDLOG;

    Json::Value retValue;
    Param param = {jsValue, (*livemountJobPtr).targetObject.subType, "Livemount", (*livemountJobPtr).jobId,
        (*m_subJobInfo).subJobId};
    LocalCmdExector::GetInstance().GetGeneralDBScriptDir((*livemountJobPtr).targetObject.subType,
        jobParam["targetObject"], param.scriptDir);
    if (LocalCmdExector::GetInstance().Exec(param, retValue) != MP_SUCCESS) {
        reportJobFailed = MP_TRUE;
        HCP_Log(ERR, MODULE) << "Exec failed, jobId=" << (*livemountJobPtr).jobId << ",subJobId=" <<
            (*m_subJobInfo).subJobId << HCPENDLOG;
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

int CommonLivemountJob::PostJob()
{
    return MP_SUCCESS;
}