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
#include "CommonDelCopyJob.h"
#include "log/Log.h"
#include "LocalCmdExector.h"
#include "ReportJobDetailHandler.h"

using namespace GeneralDB;
using namespace AppProtect;

namespace {
    const mp_string MODULE = "CommonDelCopyJob";
    const mp_string DELCOPY_HANDLE_MAP = "DelCopyHandleMap";
    using defer = std::shared_ptr<void>;
} // namespace

mp_int32 CommonDelCopyJob::PrerequisiteJob()
{
    return MP_SUCCESS;
}

mp_int32 CommonDelCopyJob::GenerateSubJob()
{
    auto jobInfo = std::dynamic_pointer_cast<AppProtect::DelCopyJob>(GetJobInfo()->GetJobInfo());
    if (jobInfo == nullptr) {
        JobLogDetail jobLogDetail = {m_jobId, "", SubJobStatus::FAILED, LOG_LABEL_TYPE::EXEC_GENJOB_FAIL};
        ReportJobDetailHandler::GetInstance()->ReportJobDetailToFrame(jobLogDetail);
        HCP_Log(ERR, MODULE) << "Failed to get del copy gen job info, jobId=" << m_jobId << HCPENDLOG;
        return MP_FAILED;
    }
    mp_string appType = jobInfo->protectObject.subType;
    Json::Value extendInfo;
    if (!Module::JsonHelper::JsonStringToJsonValue(jobInfo->protectEnv.extendInfo, extendInfo)) {
        JobLogDetail jobLogDetail = {m_jobId, "", SubJobStatus::FAILED, LOG_LABEL_TYPE::EXEC_GENJOB_FAIL};
        ReportJobDetailHandler::GetInstance()->ReportJobDetailToFrame(jobLogDetail);
        HCP_Log(ERR, MODULE) << "Failed to convert extendInfo, jobId=. " << m_jobId << HCPENDLOG;
        return MP_FAILED;
    }
    if (GenSubJob(DELCOPY_HANDLE_MAP, appType, extendInfo, jobInfo->protectEnv.nodes) != MP_SUCCESS) {
        JobLogDetail jobLogDetail = {m_jobId, "", SubJobStatus::FAILED, LOG_LABEL_TYPE::EXEC_GENJOB_FAIL};
        ReportJobDetailHandler::GetInstance()->ReportJobDetailToFrame(jobLogDetail);
        HCP_Log(ERR, MODULE) << "Failed to generate subjob, jobId=. " << m_jobId << HCPENDLOG;
    }
    return MP_SUCCESS;
}

mp_int32 CommonDelCopyJob::ExecuteSubJob()
{
    HCP_Log(DEBUG, MODULE) << "Enter del copy execute sub job" << HCPENDLOG;
    mp_bool reportJobFailed = MP_FALSE;
    defer _(nullptr, [&](...) {
        SetJobToFinish();
        if (reportJobFailed) {
            JobLogDetail jobLogDetail = {m_parentJobId, m_jobId, SubJobStatus::FAILED,
                LOG_LABEL_TYPE::EXEC_DELCOPY_SUBJOB_FAIL, JOB_INTERNAL_ERROR};
            ReportJobDetailHandler::GetInstance()->ReportJobDetailToFrame(jobLogDetail);
        }
    });

    std::shared_ptr<AppProtect::DelCopyJob> delCopyJobPtr =
        std::dynamic_pointer_cast<AppProtect::DelCopyJob>(GetJobInfo()->GetJobInfo());
    if (delCopyJobPtr == nullptr || m_subJobInfo == nullptr) {
        reportJobFailed = MP_TRUE;
        HCP_Log(ERR, MODULE) << "Failed to get del copy job info, jobId=" << m_parentJobId << ", subJobId=" <<
            m_jobId << HCPENDLOG;
        return MP_FAILED;
    }
    Json::Value jobParam;
    Json::Value subJobParam;
    StructToJson(*delCopyJobPtr, jobParam);
    StructToJson(*m_subJobInfo, subJobParam);
    Json::Value jsValue;
    jsValue["job"] = jobParam;
    jsValue["subJob"] = subJobParam;
    HCP_Log(DEBUG, MODULE) << "Execute del copy sub Job, jobId=" << (*delCopyJobPtr).jobId << ", subJobId=" <<
        (*m_subJobInfo).subJobId << HCPENDLOG;

    Json::Value retValue;
    Param param = {jsValue, (*delCopyJobPtr).protectObject.subType, "DelCopy", (*delCopyJobPtr).jobId,
        (*m_subJobInfo).subJobId};
    LocalCmdExector::GetInstance().GetGeneralDBScriptDir((*delCopyJobPtr).protectObject.subType,
        jobParam["protectObject"], param.scriptDir);
    if (LocalCmdExector::GetInstance().Exec(param, retValue) != MP_SUCCESS) {
        reportJobFailed = MP_TRUE;
        HCP_Log(ERR, MODULE) << "Exec del copy Job failed, jobId=" << (*delCopyJobPtr).jobId << ",subJobId=" <<
            (*m_subJobInfo).subJobId << HCPENDLOG;
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

mp_int32 CommonDelCopyJob::PostJob()
{
    return MP_SUCCESS;
}