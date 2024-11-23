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
#include "CommonBackupJob.h"
#include "ReportJobDetailHandler.h"

using namespace GeneralDB;
using namespace AppProtect;

namespace {
    const mp_string MODULE = "CommonBackupJob";
    const mp_string BACKUP_HANDLE_MAP = "BackupHandleMap";
    using defer = std::shared_ptr<void>;
    const mp_string QUERY_COPY = "queryCopy";
} // namespace

int CommonBackupJob::PrerequisiteJob()
{
    defer _(nullptr, [&](...) {
        SetJobToFinish();
    });

    std::shared_ptr<AppProtect::BackupJob> backupJobPtr =
        std::dynamic_pointer_cast<AppProtect::BackupJob>(GetJobInfo()->GetJobInfo());
    if (backupJobPtr == nullptr) {
        HCP_Log(ERR, MODULE) << "Prerequisite job failed for get backup failed." << HCPENDLOG;
        return MP_FAILED;
    }
    Json::Value backupJobStr;
    StructToJson(*backupJobPtr, backupJobStr);

    Json::Value jsValue;
    jsValue["job"] = backupJobStr;
    Json::Value retValue;
    Param param = {jsValue, (*backupJobPtr).protectObject.subType, "BackupPrerequisite", (*backupJobPtr).jobId};
    LocalCmdExector::GetInstance().GetGeneralDBScriptDir((*backupJobPtr).protectObject.subType,
        backupJobStr["protectObject"], param.scriptDir);
    if (LocalCmdExector::GetInstance().Exec(param, retValue, shared_from_this()) != MP_SUCCESS) {
        JobLogDetail jobLogDetail = {(*backupJobPtr).jobId, "", SubJobStatus::FAILED,
            LOG_LABEL_TYPE::UNDEFIND_LABEL};
        ReportJobDetailHandler::GetInstance()->ReportJobDetailToFrame(jobLogDetail);
        HCP_Log(ERR, MODULE) << "Exec failed, jobId=" << (*backupJobPtr).jobId << HCPENDLOG;
        return MP_FAILED;
    }

    return MP_SUCCESS;
}

int CommonBackupJob::GenerateSubJobManually()
{
    std::shared_ptr<AppProtect::BackupJob> backupJobPtr =
        std::dynamic_pointer_cast<AppProtect::BackupJob>(GetJobInfo()->GetJobInfo());
    if (backupJobPtr == nullptr) {
        HCP_Log(ERR, MODULE) << "GenerateSubJob failed for get backup failed." << HCPENDLOG;
        return MP_FAILED;
    }
    Json::Value backupJobStr;
    StructToJson(*backupJobPtr, backupJobStr);
    Json::Value jsValue;
    jsValue["job"] = backupJobStr;
    Param param = {jsValue, (*backupJobPtr).protectObject.subType, "BackupGenSubJob", (*backupJobPtr).jobId, "",
        MP_FALSE};
    LocalCmdExector::GetInstance().GetGeneralDBScriptDir((*backupJobPtr).protectObject.subType,
        backupJobStr["protectObject"], param.scriptDir);
    if (LocalCmdExector::GetInstance().Exec(param, m_manualResult, shared_from_this()) != MP_SUCCESS) {
        JobLogDetail jobLogDetail = {(*backupJobPtr).jobId, "", SubJobStatus::FAILED,
            LOG_LABEL_TYPE::UNDEFIND_LABEL};
        ReportJobDetailHandler::GetInstance()->ReportJobDetailToFrame(jobLogDetail);
        HCP_Log(ERR, MODULE) << "Exec failed, jobId=" << (*backupJobPtr).jobId << HCPENDLOG;
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

int CommonBackupJob::GenerateSubJob()
{
    auto jobInfo = std::dynamic_pointer_cast<AppProtect::BackupJob>(GetJobInfo()->GetJobInfo());
    if (jobInfo == nullptr) {
        HCP_Log(ERR, MODULE) << "GenerateSubJob failed for get backupJob Info failed." << HCPENDLOG;
        return MP_FAILED;
    }
    mp_string appType = jobInfo->protectObject.subType;
    Json::Value extendInfo;
    if (!Module::JsonHelper::JsonStringToJsonValue(jobInfo->protectEnv.extendInfo, extendInfo)) {
        HCP_Log(ERR, MODULE) << "JsonStringToJsonValue Failed! " << DBG(m_jobId) << HCPENDLOG;
        return MP_FAILED;
    }
    if (GenSubJob(BACKUP_HANDLE_MAP, appType, extendInfo, jobInfo->protectEnv.nodes) != MP_SUCCESS) {
        JobLogDetail jobLogDetail = {m_jobId, "", SubJobStatus::FAILED, LOG_LABEL_TYPE::EXEC_GENJOB_FAIL};
        ReportJobDetailHandler::GetInstance()->ReportJobDetailToFrame(jobLogDetail);
        HCP_Log(ERR, MODULE) << "Failed to generate subjob, jobId=" << m_jobId << HCPENDLOG;
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

int CommonBackupJob::ExecuteSubJob()
{
    defer _(nullptr, [&](...) {
        SetJobToFinish();
    });

    std::shared_ptr<AppProtect::BackupJob> backupJobPtr =
        std::dynamic_pointer_cast<AppProtect::BackupJob>(GetJobInfo()->GetJobInfo());
    if (backupJobPtr == nullptr || m_subJobInfo == nullptr) {
        ERRLOG("Get backup job info failed, jobId=%s, subJobId=%s.", m_parentJobId.c_str(), m_jobId.c_str());
        return MP_FAILED;
    }
    JobLogDetail startDetail = {(*backupJobPtr).jobId, (*m_subJobInfo).subJobId, SubJobStatus::RUNNING,
        LOG_LABEL_TYPE::START_EXEC_SUBJOB};
    ReportJobDetailHandler::GetInstance()->ReportJobDetailToFrame(startDetail);
    DBGLOG("JobName=%s, jobId=%s, subJobId=%s.", m_subJobInfo->jobName.c_str(), m_parentJobId.c_str(),
        m_jobId.c_str());
    // 查询副本子任务，调QueryBackupCopy接口，暂时为必拆分备份子任务，且为最低优先级
    if (m_subJobInfo->jobName == QUERY_COPY) {
        if (QueryBackupCopy() != MP_SUCCESS) {
            JobLogDetail jobLogDetail = {(*backupJobPtr).jobId, (*m_subJobInfo).subJobId, SubJobStatus::FAILED,
                LOG_LABEL_TYPE::UNDEFIND_LABEL};
            ReportJobDetailHandler::GetInstance()->ReportJobDetailToFrame(jobLogDetail);
            ERRLOG("Query backup copy failed, jobId=%s, subJobId=%s.", m_parentJobId.c_str(), m_jobId.c_str());
            return MP_FAILED;
        }
        JobLogDetail jobLogDetail = {(*backupJobPtr).jobId, (*m_subJobInfo).subJobId, SubJobStatus::COMPLETED,
            LOG_LABEL_TYPE::UNDEFIND_LABEL};
        ReportJobDetailHandler::GetInstance()->ReportJobDetailToFrame(jobLogDetail);
        INFOLOG("Report backup copy success, jobId=%s, subJobId=%s.", m_parentJobId.c_str(), m_jobId.c_str());
        return MP_SUCCESS;
    }

    Json::Value backupJobStr;
    Json::Value subJobStr;
    StructToJson(*backupJobPtr, backupJobStr);
    StructToJson(*m_subJobInfo, subJobStr);
    Json::Value jsValue;
    jsValue["job"] = backupJobStr;
    jsValue["subJob"] = subJobStr;
    Json::Value retValue;
    Param param = {jsValue, (*backupJobPtr).protectObject.subType, "Backup", (*backupJobPtr).jobId,
        (*m_subJobInfo).subJobId};
    LocalCmdExector::GetInstance().GetGeneralDBScriptDir((*backupJobPtr).protectObject.subType,
        backupJobStr["protectObject"], param.scriptDir);
    if (LocalCmdExector::GetInstance().Exec(param, retValue, shared_from_this()) != MP_SUCCESS) {
        JobLogDetail jobLogDetail = {(*backupJobPtr).jobId, (*m_subJobInfo).subJobId, SubJobStatus::FAILED,
            LOG_LABEL_TYPE::EXEC_BACKUP_SUBJOB_FAIL, JOB_INTERNAL_ERROR};
        ReportJobDetailHandler::GetInstance()->ReportJobDetailToFrame(jobLogDetail);
        ERRLOG("Exec failed, jobId=%s, subJobId=%s.", m_parentJobId.c_str(), m_jobId.c_str());
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

int CommonBackupJob::QueryBackupCopy()
{
    std::shared_ptr<AppProtect::BackupJob> backupJobPtr =
        std::dynamic_pointer_cast<AppProtect::BackupJob>(GetJobInfo()->GetJobInfo());
    if (backupJobPtr == nullptr) {
        HCP_Log(ERR, MODULE) << "QueryBackupCopy job failed for get backup failed." << HCPENDLOG;
        return MP_FAILED;
    }

    // 判断QueryBackupCopy查询副本接口是否配置，不配置则不执行，适配后面应用插件调用RPC工具进行副本上报
    mp_string actionScript;
    mp_string processScript;
    Param param;
    param.appType = (*backupJobPtr).protectObject.subType;
    param.cmdType = "QueryBackupCopy";
    if (ParseConfigFile::GetInstance()->GetExectueCmd(param, actionScript, processScript) != MP_SUCCESS) {
        HCP_Log(WARN, MODULE) << "Get script from config failed. cmdType = QueryBackupCopy, subType = " <<
            (*backupJobPtr).protectObject.subType << HCPENDLOG;
        return MP_SUCCESS;
    }

    Json::Value backupJobStr;
    StructToJson(*backupJobPtr, backupJobStr);
    Json::Value jsValue;
    jsValue["job"] = backupJobStr;

    Json::Value retValue;
    param = {jsValue, (*backupJobPtr).protectObject.subType, "QueryBackupCopy", (*backupJobPtr).jobId, "",
        MP_FALSE};
    if (LocalCmdExector::GetInstance().Exec(param, retValue) != MP_SUCCESS) {
        HCP_Log(ERR, MODULE) << "Exec failed, jobId=" << (*backupJobPtr).jobId << HCPENDLOG;
        return MP_FAILED;
    }

    Copy image;
    JsonToStruct(retValue, image);
    ActionResult reportResult;
    JobService::ReportCopyAdditionalInfo(reportResult, (*backupJobPtr).jobId, image);
    if (reportResult.code != MP_SUCCESS) {
        HCP_Log(ERR, MODULE) << "ReportCopyAdditionalInfo Failed, jobId=" << (*backupJobPtr).jobId << HCPENDLOG;
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

int CommonBackupJob::PostJob()
{
    defer _(nullptr, [&](...) {
        SetJobToFinish();
    });

    std::shared_ptr<AppProtect::BackupJob> backupJobPtr =
        std::dynamic_pointer_cast<AppProtect::BackupJob>(GetJobInfo()->GetJobInfo());
    if (backupJobPtr == nullptr || m_subJobInfo == nullptr) {
        HCP_Log(ERR, MODULE) << "PostJob job failed for get backup failed." << HCPENDLOG;
        return MP_FAILED;
    }

    Json::Value backupJobStr;
    Json::Value subJobStr;
    StructToJson(*backupJobPtr, backupJobStr);
    StructToJson(*m_subJobInfo, subJobStr);
    Json::Value jsValue;
    jsValue["job"] = backupJobStr;
    jsValue["subJob"] = subJobStr;
    jsValue["backupJobResult"] = m_jobResult;
    Json::Value retValue;
    Param param = {jsValue, (*backupJobPtr).protectObject.subType, "BackupPostJob", (*backupJobPtr).jobId,
        (*m_subJobInfo).subJobId};
    LocalCmdExector::GetInstance().GetGeneralDBScriptDir((*backupJobPtr).protectObject.subType,
        backupJobStr["protectObject"], param.scriptDir);
    if (LocalCmdExector::GetInstance().Exec(param, retValue, shared_from_this()) != MP_SUCCESS) {
        JobLogDetail jobLogDetail = {(*backupJobPtr).jobId, (*m_subJobInfo).subJobId, SubJobStatus::FAILED,
            LOG_LABEL_TYPE::UNDEFIND_LABEL};
        ReportJobDetailHandler::GetInstance()->ReportJobDetailToFrame(jobLogDetail);
        HCP_Log(ERR, MODULE) << "Exec failed, jobId=" << (*backupJobPtr).jobId << HCPENDLOG;
        return MP_FAILED;
    }
    return MP_SUCCESS;
}
