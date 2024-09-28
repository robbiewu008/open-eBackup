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
#include "CommonCancelLivemountJob.h"
#include "log/Log.h"
#include "LocalCmdExector.h"
#include "ReportJobDetailHandler.h"

using namespace GeneralDB;
using namespace AppProtect;

namespace {
    const mp_string MODULE = "CommonCancelLivemountJob";
    const mp_string CANCELLIVEMOUNT_HANDLE_MAP = "CancelLivemountHandleMap";
    using defer = std::shared_ptr<void>;
}

int CommonCancelLivemountJob::PrerequisiteJob()
{
    return MP_SUCCESS;
}

int CommonCancelLivemountJob::GenerateSubJob()
{
    auto jobInfo = std::dynamic_pointer_cast<AppProtect::CancelLivemountJob>(GetJobInfo()->GetJobInfo());
    if (jobInfo == nullptr) {
        JobLogDetail jobLogDetail = {m_jobId, "", SubJobStatus::FAILED, LOG_LABEL_TYPE::EXEC_GENJOB_FAIL};
        ReportJobDetailHandler::GetInstance()->ReportJobDetailToFrame(jobLogDetail);
        HCP_Log(ERR, MODULE) << "Failed to get cancellivemount gen job info, jobId=" << m_jobId << HCPENDLOG;
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
    if (MP_SUCCESS != GenSubJob(CANCELLIVEMOUNT_HANDLE_MAP, appType, extendInfo, jobInfo->targetEnv.nodes)) {
        JobLogDetail jobLogDetail = {m_jobId, "", SubJobStatus::FAILED, LOG_LABEL_TYPE::EXEC_GENJOB_FAIL};
        ReportJobDetailHandler::GetInstance()->ReportJobDetailToFrame(jobLogDetail);
        HCP_Log(ERR, MODULE) << "Failed to generate cancellivemount subjob, jobId=. " << m_jobId << HCPENDLOG;
    }
    return MP_SUCCESS;
}

int CommonCancelLivemountJob::ExecuteSubJob()
{
    HCP_Log(DEBUG, MODULE) << "Enter cancel livemount execute sub job" << HCPENDLOG;
    mp_bool isReportFailDetail = MP_FALSE;
    defer _(nullptr, [&](...) {
        SetJobToFinish();
        if (isReportFailDetail) {
            JobLogDetail jobDetail = {m_parentJobId, m_jobId, SubJobStatus::FAILED,
                LOG_LABEL_TYPE::EXEC_CANCELLIVEMOUNT_SUBJOB_FAIL, JOB_INTERNAL_ERROR};
            ReportJobDetailHandler::GetInstance()->ReportJobDetailToFrame(jobDetail);
        }
    });

    std::shared_ptr<AppProtect::CancelLivemountJob> cancelLivemountJobPtr =
        std::dynamic_pointer_cast<AppProtect::CancelLivemountJob>(GetJobInfo()->GetJobInfo());
    if (cancelLivemountJobPtr == nullptr || m_subJobInfo == nullptr) {
        isReportFailDetail = MP_TRUE;
        HCP_Log(ERR, MODULE) << "Failed to get cancellivemount job info, jobId=" << m_parentJobId << ", subJobId=" <<
            m_jobId << HCPENDLOG;
        return MP_FAILED;
    }
    Json::Value jobParam;
    Json::Value subJobParam;
    StructToJson(*cancelLivemountJobPtr, jobParam);
    StructToJson(*m_subJobInfo, subJobParam);
    Json::Value jsValue;
    jsValue["job"] = jobParam;
    jsValue["subJob"] = subJobParam;
    HCP_Log(DEBUG, MODULE) << "Execute cancel livemount sub Job, jobId=" << m_parentJobId << ", subJobId=" <<
        m_jobId << HCPENDLOG;

    Json::Value retValue;
    Param param = {jsValue, (*cancelLivemountJobPtr).targetObject.subType, "CancelLivemount", m_parentJobId, m_jobId};
    LocalCmdExector::GetInstance().GetGeneralDBScriptDir((*cancelLivemountJobPtr).targetObject.subType,
        jobParam["targetObject"], param.scriptDir);
    if (MP_SUCCESS != LocalCmdExector::GetInstance().Exec(param, retValue)) {
        isReportFailDetail = MP_TRUE;
        HCP_Log(ERR, MODULE) << "Exec failed, jobId=" << m_parentJobId << ",subJobId=" <<
            m_jobId << HCPENDLOG;
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

int CommonCancelLivemountJob::PostJob()
{
    return MP_SUCCESS;
}
