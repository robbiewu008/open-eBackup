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
#include "CommonCheckCopyJob.h"
#include "log/Log.h"
#include "LocalCmdExector.h"
#include "ReportJobDetailHandler.h"

using namespace GeneralDB;
using namespace AppProtect;

namespace {
    const mp_string CHECKCOPY_HANDLE_MAP = "CheckCopyHandleMap";
    using defer = std::shared_ptr<void>;
} // namespace

mp_int32 CommonCheckCopyJob::PrerequisiteJob()
{
    return MP_SUCCESS;
}

mp_int32 CommonCheckCopyJob::GenerateSubJob()
{
    auto jobInfo = std::dynamic_pointer_cast<AppProtect::CheckCopyJob>(GetJobInfo()->GetJobInfo());
    if (jobInfo == nullptr) {
        JobLogDetail jobLogDetail = {m_jobId, "", SubJobStatus::FAILED, LOG_LABEL_TYPE::EXEC_GENJOB_FAIL};
        ReportJobDetailHandler::GetInstance()->ReportJobDetailToFrame(jobLogDetail);
        ERRLOG("Failed to get check copy gen job info, jobId=%s.", m_jobId.c_str());
        return MP_FAILED;
    }
    Json::Value extendInfo;
    if (!Module::JsonHelper::JsonStringToJsonValue(jobInfo->protectEnv.extendInfo, extendInfo)) {
        JobLogDetail jobLogDetail = {m_jobId, "", SubJobStatus::FAILED, LOG_LABEL_TYPE::EXEC_GENJOB_FAIL};
        ReportJobDetailHandler::GetInstance()->ReportJobDetailToFrame(jobLogDetail);
        ERRLOG("Failed to convert extendInfo, jobId=%s.", m_jobId.c_str());
        return MP_FAILED;
    }
    mp_string appType = jobInfo->protectObject.subType;
    if (GenSubJob(CHECKCOPY_HANDLE_MAP, appType, extendInfo, jobInfo->protectEnv.nodes) != MP_SUCCESS) {
        JobLogDetail jobLogDetail = {m_jobId, "", SubJobStatus::FAILED, LOG_LABEL_TYPE::EXEC_GENJOB_FAIL};
        ReportJobDetailHandler::GetInstance()->ReportJobDetailToFrame(jobLogDetail);
        ERRLOG("Failed to generate subjob, jobId=%s.", m_jobId.c_str());
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

mp_int32 CommonCheckCopyJob::ExecuteSubJob()
{
    LOGGUARD("");
    mp_bool reportJobFailed = MP_FALSE;
    defer _(nullptr, [&](...) {
        SetJobToFinish();
        if (reportJobFailed) {
            JobLogDetail jobLogDetail = {m_parentJobId, m_jobId, SubJobStatus::FAILED, LOG_LABEL_TYPE::UNDEFIND_LABEL};
            ReportJobDetailHandler::GetInstance()->ReportJobDetailToFrame(jobLogDetail);
        }
    });

    std::shared_ptr<AppProtect::CheckCopyJob> checkCopyJobPtr =
        std::dynamic_pointer_cast<AppProtect::CheckCopyJob>(GetJobInfo()->GetJobInfo());
    if (checkCopyJobPtr == nullptr || m_subJobInfo == nullptr) {
        reportJobFailed = MP_TRUE;
        ERRLOG("Failed to get check copy job info, jobId=%s, subJobId=%s.", m_parentJobId.c_str(), m_jobId.c_str());
        return MP_FAILED;
    }
    Json::Value jobParam;
    Json::Value subJobParam;
    StructToJson(*checkCopyJobPtr, jobParam);
    StructToJson(*m_subJobInfo, subJobParam);
    Json::Value jsValue;
    jsValue["job"] = jobParam;
    jsValue["subJob"] = subJobParam;
    DBGLOG("Execute check copy sub Job, jobId=%s, subJobId=%s.", m_parentJobId.c_str(), m_jobId.c_str());

    Json::Value retValue;
    Param param = {jsValue, (*checkCopyJobPtr).protectObject.subType, "CheckCopy", (*checkCopyJobPtr).jobId,
        (*m_subJobInfo).subJobId};
    if (LocalCmdExector::GetInstance().Exec(param, retValue, shared_from_this()) != MP_SUCCESS) {
        reportJobFailed = MP_TRUE;
        ERRLOG("Exec check copy Job failed, jobId=%s, subJobId=%s.", m_parentJobId.c_str(), m_jobId.c_str());
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

mp_int32 CommonCheckCopyJob::PostJob()
{
    return MP_SUCCESS;
}