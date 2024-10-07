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
#include "CommonCreateIndexJob.h"
#include "log/Log.h"
#include "CTime.h"
#include "LocalCmdExector.h"
#include "trjsontostruct.h"
#include "ReportJobDetailHandler.h"

using namespace GeneralDB;
using namespace AppProtect;

namespace {
    const mp_string MODULE = "CommonCreateIndexJob";
    const mp_string CREATE_INDEX_MAP = "CommonCreateIndexHandleMap";
    const mp_uint32 SEND_ADDNEWJOB_DELAY_TIME = 10 * 1000;  // 10s delay
    constexpr uint32_t SEND_ADDNEWJOB_RETRY_TIMES = 30;
    const mp_uint32 ADDNEWJOB_TIMEOUT = 5 * 60;  // 5min
    using defer = std::shared_ptr<void>;
} // namespace

int CommonCreateIndexJob::PrerequisiteJob()
{
    HCP_Log(DEBUG, MODULE) << "Enter prerequisit index execute sub job" << HCPENDLOG;
    return MP_SUCCESS;
}

int CommonCreateIndexJob::GenerateSubJob()
{
    SubJob subJob {};
    subJob.__set_jobId(m_jobId);
    subJob.__set_jobType(SubJobType::BUSINESS_SUB_JOB);
    subJob.__set_jobName("CreateIndex");
    subJob.__set_policy(ExecutePolicy::LOCAL_NODE);
    vector<SubJob> vec {};
    vec.push_back(subJob);

    ActionResult ret {};
    int retryTimes = SEND_ADDNEWJOB_RETRY_TIMES;
    while (retryTimes > 0) {
        JobService::AddNewJob(ret, vec);
        if (ret.code == MP_SUCCESS) {
            break;
        }

        // 重试阶段上报任务状态为Running
        JobLogDetail jobLogDetail = {m_jobId, "", SubJobStatus::RUNNING, LOG_LABEL_TYPE::UNDEFIND_LABEL};
        ReportJobDetailHandler::GetInstance()->ReportJobDetailToFrame(jobLogDetail);
        WARNLOG("AddNewJob failed, will try again, jobId=%s.", m_jobId.c_str());
        retryTimes--;
    }

    JobLogDetail jobLogDetail = {m_jobId, "", SubJobStatus::COMPLETED, LOG_LABEL_TYPE::UNDEFIND_LABEL};
    ReportJobDetailHandler::GetInstance()->ReportJobDetailToFrame(jobLogDetail);

    return retryTimes == 0 ? MP_FAILED : MP_SUCCESS;
}

int CommonCreateIndexJob::ExecuteSubJob()
{
    HCP_Log(DEBUG, MODULE) << "Enter create index execute sub job" << HCPENDLOG;
    defer _(nullptr, [&](...) {
        SetJobToFinish();
    });

    DBGLOG("JobName=%s, jobId=%s, subJobId=%s.", m_subJobInfo->jobName.c_str(), m_parentJobId.c_str(),
        m_jobId.c_str());

    std::shared_ptr<AppProtect::BuildIndexJob> buildIndexJobPtr =
        std::dynamic_pointer_cast<AppProtect::BuildIndexJob>(GetJobInfo()->GetJobInfo());
    if (buildIndexJobPtr == nullptr || m_subJobInfo == nullptr) {
        HCP_Log(ERR, MODULE) << "Failed to build index sub job info, jobId=" << m_parentJobId << ", subJobId=" <<
            m_jobId << HCPENDLOG;
        return MP_FAILED;
    }
    Json::Value jobParam;
    Json::Value subJobParam;
    StructToJson(*buildIndexJobPtr, jobParam);
    StructToJson(*m_subJobInfo, subJobParam);
    Json::Value jsValue;
    jsValue["job"] = jobParam;
    jsValue["subJob"] = subJobParam;
    HCP_Log(DEBUG, MODULE) << "Execute build index sub Job, jobId=" << (*buildIndexJobPtr).jobId << ", subJobId=" <<
        (*m_subJobInfo).subJobId << HCPENDLOG;

    Json::Value retValue;
    Param param = {jsValue, (*buildIndexJobPtr).indexProtectObject.subType, "CreateIndex", (*buildIndexJobPtr).jobId,
        (*m_subJobInfo).subJobId};
    LocalCmdExector::GetInstance().GetGeneralDBScriptDir((*buildIndexJobPtr).indexProtectObject.subType,
        jobParam["indexProtectObject"], param.scriptDir);
    if (LocalCmdExector::GetInstance().Exec(param, retValue, shared_from_this()) != MP_SUCCESS) {
        JobLogDetail jobLogDetail = {(*buildIndexJobPtr).jobId, (*m_subJobInfo).subJobId, SubJobStatus::FAILED,
            LOG_LABEL_TYPE::EXEC_BACKUP_SUBJOB_FAIL, JOB_INTERNAL_ERROR};
        ReportJobDetailHandler::GetInstance()->ReportJobDetailToFrame(jobLogDetail);
        ERRLOG("Exec failed, jobId=%s, subJobId=%s.", m_parentJobId.c_str(), m_jobId.c_str());
        return MP_FAILED;
    }

    return MP_SUCCESS;
}

int CommonCreateIndexJob::PostJob()
{
    return MP_SUCCESS;
}

