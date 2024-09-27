/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 *
 * @brief  plugin sub job
 * @version 1.1.0
 * @date 2021-11-19
 * @author kWX884906
 */
#include "securecom/RootCaller.h"
#include "common/JsonUtils.h"
#include "common/Log.h"
#include "common/File.h"
#include "message/curlclient/DmeRestClient.h"
#include "message/curlclient/RestClientCommon.h"
#include "pluginfx/ExternalPluginManager.h"
#include "apps/appprotect/plugininterface/ProtectService.h"
#include "servicecenter/thriftservice/JsonToStruct/trjsonandstruct.h"
#include "taskmanager/externaljob/AppProtectJobHandler.h"
#include "taskmanager/externaljob/PluginSubPostJob.h"

namespace {
    constexpr mp_int32 MAIN_STATUS_ABORT_FAILED = 5;
    constexpr mp_int32 MAIN_STATUS_ABORTED = 4;
    constexpr mp_int32 MAIN_STATUS_ABORTING = 2;
    constexpr mp_int32 MAIN_STATUS_FAILED = 6;
    constexpr mp_int32 SUBJOB_MINI_SIZE = 1;
    constexpr int64_t DATA_REPO_TYPE = 1;
    constexpr int64_t META_REPO_TYPE = 0;
    static const mp_string RECORD_FILE_NAME = "RecordFile.txt"; // 保存扫描出来的文件
    static const mp_string RECORD_DIR_NAME = "RecordDir.txt";   // 保存扫描出来的目录
}
namespace AppProtect {
mp_int32 PluginSubPostJob::ExecBackupJob()
{
#ifndef WIN32
    // 后置任务扫描Data仓库仅非Windows下应用需要
    mp_string pluginName;
    mp_int32 iRet =
        ExternalPluginManager::GetInstance().GetParseManager()->GetPluginNameByAppType(m_data.appType, pluginName);
    if (iRet != MP_SUCCESS) {
        WARNLOG("Fail to get plugin name for apptype=%s.", m_data.appType.c_str());
    }
    if (pluginName != "NasPlugin" && pluginName != "FilePlugin" && pluginName != "" && pluginName != "ObsPlugin") {
        StartKeepAliveThread();
        iRet = ScanAndRecordFile();
        if (iRet != MP_SUCCESS) {
            WARNLOG("ScanAndRecordFile failed");
        }
        StopKeepAliveThread();
    }
#endif
    ActionResult ret;
    BackupJob jobParam;
    SubJob subJobParam;
    JsonToStruct(m_data.param, jobParam);
    JsonToStruct(m_data.param, subJobParam);
    INFOLOG("Backup post job, jobId=%s, subJobId=%s, wholeResult=%d.",
        subJobParam.jobId.c_str(), subJobParam.subJobId.c_str(), mp_int32(m_wholeJobResult));
    ProtectServiceCall(&ProtectServiceIf::AsyncBackupPostJob, ret, jobParam, subJobParam, m_wholeJobResult);
    mp_int32 deleteQos = DeleteQosStrategy();
    if (deleteQos != MP_SUCCESS) {
        ERRLOG("Failed to delete the qos policy, ret=%d, jobId=%s, subJobId=%s.", deleteQos, m_data.mainID.c_str(),
            m_data.subID.c_str());
    }
    return ret.code;
}

#ifndef WIN32
mp_int32 PluginSubPostJob::ScanAndRecordFile()
{
    Json::Value jsonData = m_data.param;
    std::vector<mp_string> vecFolderPath;
    std::vector<mp_string> vecFilePath;
    CRootCaller rootCaller;
    for (Json::ArrayIndex index = 0; index < jsonData["repositories"].size(); ++index) {
        mp_string savePath;
        if ((jsonData["repositories"][index]["type"] != META_REPO_TYPE &&
                jsonData["repositories"][index]["type"] != DATA_REPO_TYPE) ||
            jsonData["repositories"][index]["path"].empty()) {
            continue;
        }
        for (Json::ArrayIndex index1 = 0; index1 < jsonData["repositories"][index]["path"].size(); ++index1) {
            mp_string path = jsonData["repositories"][index]["path"][index1].asString();
            INFOLOG("Scan path is:%s.", path.c_str());

            std::vector<mp_string> vecResult;
            mp_int32 iRet = rootCaller.Exec((mp_int32)ROOT_COMMAND_SCAN_DIR_FILE, path, &vecResult);
            if (iRet != MP_SUCCESS) {
                WARNLOG("ScanDirAndFile failed, iRet is:%d.", iRet);
                return MP_FAILED;
            }
            iRet = ExtractScanResult(vecResult, vecFolderPath, vecFilePath);
            if (iRet != MP_SUCCESS) {
                WARNLOG("Extract scan result failed, iRet is:%d.", iRet);
                return MP_FAILED;
            }
            INFOLOG("Scan result: folder size %d, file size %d.", vecFolderPath.size(), vecFilePath.size());
        }
        savePath = jsonData["repositories"][index]["path"][0].asString(); // 保存扫描结果文件的路径
        INFOLOG("Save path is:%s.", savePath.c_str());
        mp_int32 iRet = SaveScanResult(savePath, vecFolderPath, vecFilePath);
        if (iRet != MP_SUCCESS) {
            ERRLOG("Save scan result failed.iRet is:%d.", iRet);
            return MP_FAILED;
        }
    }
    return MP_SUCCESS;
}

mp_int32 PluginSubPostJob::ExtractScanResult(std::vector<mp_string> &vecResult, std::vector<mp_string> &vecFolderPath,
    std::vector<mp_string> &vecFilePath)
{
    // 解析扫描结果，分隔字符串前的放入vecFolderPath，后面的放入vecFilePath
    if (!vecResult.empty()) {
        auto it = std::find(vecResult.begin(), vecResult.end(), INSTANLY_MOUNT_SCAN_RESULT_SPLIT_STR);
        if (it != vecResult.end()) {
            vecFolderPath.insert(vecFolderPath.end(), vecResult.begin(), it);
            vecFilePath.insert(vecFilePath.end(), it + 1, vecResult.end());
        } else {
            WARNLOG("Scan result is invalid.");
            return MP_FAILED;
        }
    }
    return MP_SUCCESS;
}

mp_int32 PluginSubPostJob::SaveScanResult(mp_string &savePath, std::vector<mp_string> &vecFolderPath,
    std::vector<mp_string> &vecFilePath)
{
    CRootCaller rootCaller;
    mp_string RecordFolderPath = savePath + PATH_SEPARATOR + RECORD_DIR_NAME;
    mp_string RecordFilePath = savePath + PATH_SEPARATOR + RECORD_FILE_NAME;
    TruncateScanResult(vecFolderPath);
    TruncateScanResult(vecFilePath);

    vecFolderPath.insert(vecFolderPath.begin(), RecordFolderPath);
    mp_int32 iRet = rootCaller.ExecEx((mp_int32)ROOT_COMMAND_WRITE_SCAN_RESULT, vecFolderPath, nullptr);
    if (iRet != MP_SUCCESS) {
        WARNLOG("Write dir result failed.iRet is:%d.", iRet);
        return MP_FAILED;
    }

    vecFilePath.insert(vecFilePath.begin(), RecordFilePath);
    iRet = rootCaller.ExecEx((mp_int32)ROOT_COMMAND_WRITE_SCAN_RESULT, vecFilePath, nullptr);
    if (iRet != MP_SUCCESS) {
        WARNLOG("Write file result failed.iRet is:%d.", iRet);
        return MP_FAILED;
    }
    INFOLOG("RecordFolderPath is %s, RecordFilePath is %s", RecordFolderPath.c_str(), RecordFilePath.c_str());
    vecFolderPath.clear();
    vecFilePath.clear();
    return MP_SUCCESS;
}


mp_int32 PluginSubPostJob::TruncateScanResult(std::vector<mp_string> &vecfolderPath)
{
    SubJob subJobParam;
    JsonToStruct(m_data.param, subJobParam);
    mp_string truncateStr =
        "/mnt/databackup/" + m_data.appType + PATH_SEPARATOR + subJobParam.jobId.c_str() + PATH_SEPARATOR + "data";
    mp_string truncateStrMeta =
        "/mnt/databackup/" + m_data.appType + PATH_SEPARATOR + subJobParam.jobId.c_str() + PATH_SEPARATOR + "meta";
    for (mp_string &folderPath : vecfolderPath) {
        size_t size = folderPath.size();
        if (folderPath.find(truncateStr) != 0 && folderPath.find(truncateStrMeta) != 0) {
            ERRLOG("folderPath is invalid:%s.", folderPath.c_str());
            return MP_FAILED;
        }
        folderPath = folderPath.substr(truncateStr.length(), size);
    }
    return MP_SUCCESS;
}
#endif

mp_int32 PluginSubPostJob::ExecRestoreJob()
{
    ActionResult ret;
    RestoreJob jobParam;
    SubJob subJobParam;
    JsonToStruct(m_data.param, jobParam);
    JsonToStruct(m_data.param, subJobParam);
    INFOLOG("Restore post job, jobId=%s, subJobId=%s, wholeResult=%d.",
        subJobParam.jobId.c_str(), subJobParam.subJobId.c_str(), mp_int32(m_wholeJobResult));
    ProtectServiceCall(&ProtectServiceIf::AsyncRestorePostJob, ret, jobParam, subJobParam, m_wholeJobResult);
    return ret.code;
}

mp_int32 PluginSubPostJob::ExecInrestoreJob()
{
    ActionResult ret;
    RestoreJob jobParam;
    SubJob subJobParam;
    JsonToStruct(m_data.param, jobParam);
    JsonToStruct(m_data.param, subJobParam);

    INFOLOG("Instance restore post job, jobId=%s, subJobId=%s, wholeResult=%d.",
        subJobParam.jobId.c_str(), subJobParam.subJobId.c_str(),  mp_int32(m_wholeJobResult));
    ProtectServiceCall(&ProtectServiceIf::AsyncInstantRestorePostJob, ret, jobParam, subJobParam, m_wholeJobResult);
    return ret.code;
}

mp_int32 PluginSubPostJob::ReportCompleted()
{
    SubJob subJobParam;
    JsonToStruct(m_data.param, subJobParam);

    SubJobDetails jobDetail;
    jobDetail.__set_jobId(subJobParam.jobId);
    jobDetail.__set_subJobId(subJobParam.subJobId);
    jobDetail.__set_jobStatus(SubJobStatus::type::COMPLETED);
    INFOLOG("MainJob type=%d, jobId=%s.", mp_int32(m_data.mainType), subJobParam.jobId.c_str());

    ActionResult ret;
    AppProtect::AppProtectJobHandler::GetInstance()->ReportJobDetails(ret, jobDetail);
    return ret.code;
}

mp_int32 PluginSubPostJob::GetJobsExecResult()
{
    Json::Value jTaskId;
    jTaskId["task_id"] = m_data.mainID;
    DmeRestClient::HttpReqParam reqParam = {"POST",
        "/v1/dme-unified/tasks/statistic", jTaskId.toStyledString()};
    HttpResponse response;
    reqParam.mainJobId = m_data.mainID;
    mp_int32 iRet = DmeRestClient::GetInstance()->SendRequest(reqParam, response);
    if (iRet != MP_SUCCESS || response.statusCode != SC_OK) {
        RestClientCommon::RspMsg errMsg;
        RestClientCommon::ConvertStrToRspMsg(response.body, errMsg);
        ERRLOG("Post job GetJobsExecResult failed, jobId=%s, errorCode=%s, errorMessage=%s.",
            m_data.mainID.c_str(), errMsg.errorCode.c_str(), errMsg.errorMessage.c_str());
        return MP_FAILED;
    }
    Json::Value rspValue;
    CHECK_FAIL_EX(CJsonUtils::ConvertStringtoJson(response.body, rspValue));
    iRet = GetJobStatus(rspValue);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Get job status failed, ret %d jobId=%s subJobId=%s", iRet,
               m_data.mainID.c_str(), m_data.subID.c_str());
    }
    return iRet;
}
mp_int32 PluginSubPostJob::GetJobStatus(const Json::Value& rspValue)
{
    if (!rspValue.isMember("subTask") || !rspValue.isMember("mainTask")) {
        ERRLOG("DME Job status interface rsp miss key subTask or mainTask");
        return MP_FAILED;
    }
    mp_uint32 subNumber = 0;
    GET_JSON_UINT32(rspValue["subTask"], "total", subNumber);
    // subjob total less than or equal 1 which mean no business job or add post job faild so use mainjob status
    if (subNumber <= SUBJOB_MINI_SIZE) {
        mp_int32 mainStatus = 0;
        GET_JSON_INT32(rspValue, "mainTask", mainStatus);
        DBGLOG("Get maintask status is %d", mainStatus);
        if (mainStatus == MAIN_STATUS_ABORTING ||
            mainStatus == MAIN_STATUS_ABORTED ||
            mainStatus == MAIN_STATUS_ABORT_FAILED) {
            m_wholeJobResult = JobResult::type::ABORTED;
        } else if (mainStatus >= MAIN_STATUS_FAILED) {
            m_wholeJobResult = JobResult::type::FAILED;
        } else {
            m_wholeJobResult = JobResult::type::SUCCESS;
        }
    } else {
        mp_int32 nFailed = 0;
        mp_int32 nAbort = 0;
        GET_JSON_INT32(rspValue["subTask"], "failed", nFailed);
        GET_JSON_INT32(rspValue["subTask"], "aborted", nAbort);
        if (nAbort > 0) {
            m_wholeJobResult = JobResult::type::ABORTED;
        } else if (nFailed > 0) {
            m_wholeJobResult = JobResult::type::FAILED;
        } else {
            m_wholeJobResult = JobResult::type::SUCCESS;
        }
    }
    return MP_SUCCESS;
}
Executor PluginSubPostJob::GetPluginCall()
{
    std::map<MainJobType, Executor> ExcuterMap = {
        { MainJobType::BACKUP_JOB, [this](int32_t)
            {
                return ExecBackupJob();
            }
        },
        { MainJobType::RESTORE_JOB, [this](int32_t)
            {
                return ExecRestoreJob();
            }
        },
        { MainJobType::DELETE_COPY_JOB, [this](int32_t)
            {
                return ReportCompleted();
            }
        },
        { MainJobType::LIVEMOUNT_JOB, [this](int32_t)
            {
                return ReportCompleted();
            }
        },
        { MainJobType::CANCEL_LIVEMOUNT_JOB, [this](int32_t)
            {
                return ReportCompleted();
            }
        },
        { MainJobType::BUILD_INDEX_JOB, [this](int32_t)
            {
                return ReportCompleted();
            }
        },
        { MainJobType::CHECK_COPY_JOB, [this](int32_t)
            {
                return ReportCompleted();
            }
        },
        { MainJobType::INSTANT_RESTORE_JOB, [this](int32_t)
            {
                return ExecInrestoreJob();
            }
        }
    };
    auto it = ExcuterMap.find(m_data.mainType);
    if (it != ExcuterMap.end()) {
        return it->second;
    }
    return GetEmptyExcutor();
}

Executor PluginSubPostJob::ExecPostScript()
{
    return [this](int32_t) {
    mp_int32 iRet = MP_SUCCESS;
    CHECK_FAIL_EX(GetJobsExecResult());
    if ((m_wholeJobResult == JobResult::type::ABORTED) || (m_wholeJobResult == JobResult::type::FAILED)) {
        DBGLOG("Begin to exec job failed post script.");
        iRet = Job::ExecPostScript(KEY_POST_FAIL_SCRIPTS);
    } else if (m_wholeJobResult == JobResult::type::SUCCESS) {
        DBGLOG("Begin to exec job succ post script.");
        iRet = Job::ExecPostScript(KEY_POST_SUCC_SCRIPTS);
    }
    DBGLOG("Exec post script finish.");
    return iRet;
    };
}

bool PluginSubPostJob::NotifyPluginReloadImpl(const mp_string& appType, const mp_string& newPluginPID)
{
    INFOLOG("NotifyPluginReloadImpl jobId=%s suJobId:%s notify plugin reload m_data.status:%d \
        m_pluginPID:%s newPluginPID:%s.",
        m_data.mainID.c_str(), m_data.subID.c_str(), m_data.status, m_pluginPID.c_str(), newPluginPID.c_str());
    if (m_data.status == mp_uint32(SubJobState::UNDEFINE) ||
        m_data.status == mp_uint32(SubJobState::SubJobComplete) ||
        m_data.status == mp_uint32(SubJobState::SubJobFailed)) {
        DBGLOG("No need redo again, jobId=%s, subJobId=%s, status=%d.", m_data.mainID.c_str(), m_data.subID.c_str(),
            m_data.status);
        return true;
    }
    if (!m_pluginPID.empty() && m_pluginPID != newPluginPID) {
        return false;
    }
    return true;
}

mp_int32 PluginSubPostJob::DeleteQosStrategy()
{
    LOGGUARD("");
    if (!m_data.param.isMember("taskParams") || !m_data.param["taskParams"].isObject()) {
        ERRLOG("Json has no taskParams, jobId=%s, subJobId=%s.", m_data.mainID.c_str(),
            m_data.subID.c_str());
        return MP_FAILED;
    }
    auto taskParams = m_data.param["taskParams"];
    if (!taskParams.isMember("qos") || !taskParams["qos"].isObject() || taskParams["qos"].isNull()) {
        return MP_SUCCESS;
    }
    auto dmeClient = DmeRestClient::GetInstance();
    if (dmeClient == nullptr) {
        ERRLOG("Get dme rest client faield, jobId=%s, subJobId=%s.", m_data.mainID.c_str(), m_data.subID.c_str());
        return MP_FAILED;
    }
    mp_string url;
    if (m_data.param["taskId"].isString()) {
        url = "/v1/dme-unified/tasks/qos?task_id=" + m_data.param["taskId"].asString();
    }
    DmeRestClient::HttpReqParam param("DELETE", url, "");
    param.mainJobId = m_data.mainID;
    HttpResponse response;
    mp_int32 iRet = dmeClient->SendRequest(param, response);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Send url %s faield, ret=%d, jobId=%s, subJobId=%s.", url.c_str(), iRet, m_data.mainID.c_str(),
            m_data.subID.c_str());
        return iRet;
    }
    if (response.statusCode != SC_OK) {
        RestClientCommon::RspMsg errMsg;
        RestClientCommon::ConvertStrToRspMsg(response.body, errMsg);
        ERRLOG("Delete qos strategy failed, jobId=%s,  subJobId=%s, statusCode=%d, errorCode=%s, errorMessage=%s.",
            m_data.mainID.c_str(),
            m_data.subID.c_str(),
            response.statusCode,
            errMsg.errorCode.c_str(),
            errMsg.errorMessage.c_str());
        return  CMpString::SafeStoi(errMsg.errorCode);
    }
    return MP_SUCCESS;
}

mp_int32 PluginSubPostJob::NotifyPauseJob()
{
    INFOLOG("After Pause job, set job failed, jobId=%s subJobId=%s", m_data.mainID.c_str(), m_data.subID.c_str());
    SetJobRetry(true);
    ChangeState(SubJobState::SubJobFailed);
    return MP_SUCCESS;
}

mp_int32 PluginSubPostJob::CanbeRunInLocalNode()
{
    LOGGUARD("");
    ActionResult ret;
    if (m_data.mainType == MainJobType::BACKUP_JOB) {
        SetAgentsToExtendInfo(m_data.param);
        BackupJob backupJob;
        JsonToStruct(m_data.param, backupJob);
        SubJob subJob;
        JsonToStruct(m_data.param, subJob);
        ProtectServiceCall(&ProtectServiceIf::AllowBackupSubJobInLocalNode, ret, backupJob, subJob);
        if (ret.code != MP_SUCCESS) {
            ret.code = (ret.bodyErr == 0) ? ERR_PLUGIN_AUTHENTICATION_FAILED : ret.bodyErr;
            ERRLOG("Check jobId=%s can be allow backup in local node failed, subJobId=%s, error=%d",
                m_data.mainID.c_str(), m_data.subID.c_str(), ret.code);
        }
    } else if (m_data.mainType == MainJobType::RESTORE_JOB) {
        RestoreJob restoreJob;
        JsonToStruct(m_data.param, restoreJob);
        SubJob subJob;
        JsonToStruct(m_data.param, subJob);
        ProtectServiceCall(&ProtectServiceIf::AllowRestoreSubJobInLocalNode, ret, restoreJob, subJob);
        if (ret.code != MP_SUCCESS) {
            ret.code = (ret.bodyErr == 0) ? ERR_PLUGIN_AUTHENTICATION_FAILED : ret.bodyErr;
            ERRLOG("Check jobId=%s can be allow restore in local node failed, subJobId=%s, error=%d",
                m_data.mainID.c_str(), m_data.subID.c_str(), ret.code);
        }
    } else if (m_data.mainType == MainJobType::CHECK_COPY_JOB) {
        INFOLOG("Check copy post_job can run,jobId=%s, subJobId=%s.", m_data.mainID.c_str(), m_data.subID.c_str());
        CheckCopyJob checkCopyJob;
        SubJob subJob;
        JsonToStruct(m_data.param, checkCopyJob);
        JsonToStruct(m_data.param, subJob);
        ProtectServiceCall(&ProtectServiceIf::AllowCheckCopySubJobInLocalNode, ret, checkCopyJob, subJob);
        if (ret.code != MP_SUCCESS) {
            ret.code = (ret.bodyErr == 0) ? ERR_PLUGIN_AUTHENTICATION_FAILED : ret.bodyErr;
            ERRLOG("Check jobId=%s can be allow check copy in local node failed, subJobId=%s, error=%d",
                m_data.mainID.c_str(),
                m_data.subID.c_str(),
                ret.code);
        }
    }
    return ret.code;
}
}  // namespace AppProtect
